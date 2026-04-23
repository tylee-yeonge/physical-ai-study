"""Classical SGBM vs Learning-based Monocular Depth 비교 실습.

기하학 기반 stereo (StereoSGBM) 와 딥러닝 기반 monocular depth
(Depth Anything v2) 의 결과를 같은 장면에 대해 나란히 시각화한다.

목적:
    - SGBM 이 실패하는 영역 (textureless / repetitive / occlusion) 을
      학습 기반 방법이 어떻게 메우는지 확인
    - (선택) SGBM 의 유효 픽셀로 Depth Anything 의 scale/shift 를 맞춰
      "학습 prior + 기하 anchor" hybrid 구조 체험

사용 예:
    python practice_dl_compare.py \\
        --left build/output/01_left.png \\
        --right build/output/01_right.png \\
        --output-dir build/output/dl_compare
"""

import argparse
from pathlib import Path
from typing import Tuple

import cv2
import matplotlib.pyplot as plt
import numpy as np
import torch
from PIL import Image
from transformers import pipeline


# SGBM 파라미터 (rectified 합성 스테레오에 적당한 기본값)
SGBM_MIN_DISP = 0
SGBM_NUM_DISP = 128
SGBM_BLOCK_SIZE = 5
SGBM_UNIQUENESS = 10
SGBM_SPECKLE_WINDOW = 100
SGBM_SPECKLE_RANGE = 32


def compute_sgbm_disparity(
    left_bgr: np.ndarray, right_bgr: np.ndarray
) -> Tuple[np.ndarray, np.ndarray]:
    """StereoSGBM 으로 dense disparity 와 invalid mask 를 계산한다.

    OpenCV SGBM 은 disparity 를 16 배 스케일 int16 으로 반환하므로
    float32 로 나눠서 실제 픽셀 단위로 변환한다. minDisparity - 1 은
    매칭 실패 sentinel 값으로, left-right consistency check 또는
    uniqueness ratio 에서 탈락한 픽셀이 여기로 들어간다.

    Args:
        left_bgr: Rectified 좌측 이미지 (BGR, uint8)
        right_bgr: Rectified 우측 이미지 (BGR, uint8)

    Returns:
        (disparity, invalid_mask) 튜플.
        disparity: float32 픽셀 단위 disparity (H, W)
        invalid_mask: bool mask (True = 유효하지 않음)
    """
    left_gray = cv2.cvtColor(left_bgr, cv2.COLOR_BGR2GRAY)
    right_gray = cv2.cvtColor(right_bgr, cv2.COLOR_BGR2GRAY)

    sgbm = cv2.StereoSGBM_create(
        minDisparity=SGBM_MIN_DISP,
        numDisparities=SGBM_NUM_DISP,
        blockSize=SGBM_BLOCK_SIZE,
        P1=8 * 3 * SGBM_BLOCK_SIZE**2,
        P2=32 * 3 * SGBM_BLOCK_SIZE**2,
        disp12MaxDiff=1,
        uniquenessRatio=SGBM_UNIQUENESS,
        speckleWindowSize=SGBM_SPECKLE_WINDOW,
        speckleRange=SGBM_SPECKLE_RANGE,
        mode=cv2.STEREO_SGBM_MODE_SGBM_3WAY,
    )

    # SGBM 은 16 배 스케일로 반환하므로 실제 픽셀 단위로 변환
    disparity_raw = sgbm.compute(left_gray, right_gray)
    disparity = disparity_raw.astype(np.float32) / 16.0

    # minDisparity - 1 값 (= -1 기본값) 은 매칭 실패 sentinel
    invalid_mask = disparity < (SGBM_MIN_DISP + 0.5)
    return disparity, invalid_mask


def predict_mono_depth(
    left_bgr: np.ndarray, model_name: str, device: str
) -> np.ndarray:
    """Depth Anything v2 로 monocular relative depth 를 추정한다.

    HuggingFace transformers 의 depth-estimation pipeline 을 사용한다.
    첫 실행 시 모델 가중치를 자동 다운로드한다. 출력은 가까울수록 큰
    값을 갖는 inverse-depth 유사 출력이다 (disparity 와 같은 방향).

    Args:
        left_bgr: 좌측 이미지 (BGR, uint8)
        model_name: HuggingFace 모델 ID
        device: 'cuda' 또는 'cpu'

    Returns:
        Relative depth (H, W) float32 numpy array.
        SGBM disparity 와 부호/방향을 맞추기 위해 가까울수록 큰 값.
    """
    # BGR -> RGB 변환 후 PIL 로 전달 (pipeline 이 PIL 을 요구)
    left_rgb = cv2.cvtColor(left_bgr, cv2.COLOR_BGR2RGB)
    pil_image = Image.fromarray(left_rgb)

    device_index = 0 if device == "cuda" else -1
    pipe = pipeline(
        task="depth-estimation", model=model_name, device=device_index
    )
    result = pipe(pil_image)

    # pipeline 출력은 PIL Image (uint8) 의 predicted_depth. 원본 해상도로
    # 이미 resize 되어 있으므로 그대로 numpy 로 변환한다.
    depth_pil = result["depth"]
    depth = np.array(depth_pil).astype(np.float32)

    # 해상도 안전 장치: 드물게 pipeline 이 다른 크기를 줄 경우 복원
    h, w = left_bgr.shape[:2]
    if depth.shape != (h, w):
        depth = cv2.resize(depth, (w, h), interpolation=cv2.INTER_LINEAR)
    return depth


def align_scale_shift(
    da_depth: np.ndarray, sgbm_disp: np.ndarray, valid_mask: np.ndarray
) -> np.ndarray:
    """SGBM 유효 픽셀로 Depth Anything 출력의 scale/shift 를 추정한다.

    da_aligned = s * da_depth + t 형태로 least-squares 회귀한다.
    둘 다 "가까울수록 큰 값" 방향이므로 sign flip 없이 바로 회귀 가능.

    Args:
        da_depth: Depth Anything 출력 (H, W)
        sgbm_disp: SGBM disparity (H, W, float)
        valid_mask: SGBM 유효 영역 bool mask (H, W)

    Returns:
        Scale/shift 적용된 da_depth (H, W).
        유효 픽셀 수가 부족하면 원본을 그대로 반환.
    """
    if valid_mask.sum() < 100:
        print("  [경고] SGBM 유효 픽셀이 너무 적어 scale alignment 생략")
        return da_depth

    x = da_depth[valid_mask].astype(np.float64)
    y = sgbm_disp[valid_mask].astype(np.float64)

    # y = s*x + t 의 closed-form least squares
    a_matrix = np.stack([x, np.ones_like(x)], axis=1)
    (s, t), *_ = np.linalg.lstsq(a_matrix, y, rcond=None)
    aligned = (s * da_depth + t).astype(np.float32)
    print(f"  [align] scale={s:.4f}, shift={t:.4f}")
    return aligned


def colorize(depth_like: np.ndarray, invalid_mask: np.ndarray = None) -> np.ndarray:
    """Depth/Disparity 를 JET 컬러맵으로 시각화. invalid 는 검정."""
    valid = depth_like if invalid_mask is None else depth_like[~invalid_mask]
    if valid.size == 0:
        return np.zeros((*depth_like.shape, 3), dtype=np.uint8)

    vmin, vmax = np.percentile(valid, [1, 99])
    normalized = np.clip((depth_like - vmin) / max(vmax - vmin, 1e-6), 0, 1)
    colored = cv2.applyColorMap(
        (normalized * 255).astype(np.uint8), cv2.COLORMAP_JET
    )
    if invalid_mask is not None:
        colored[invalid_mask] = 0
    return cv2.cvtColor(colored, cv2.COLOR_BGR2RGB)


def save_comparison(
    left_bgr: np.ndarray,
    disparity: np.ndarray,
    invalid_mask: np.ndarray,
    da_depth: np.ndarray,
    out_path: Path,
) -> None:
    """4-panel 비교 이미지를 저장한다.

    패널 구성:
        (0,0) 좌측 원본
        (0,1) SGBM disparity (invalid 는 검정)
        (1,0) Depth Anything relative depth (dense)
        (1,1) DA depth + SGBM invalid 영역을 빨간 외곽선으로 오버레이
    """
    left_rgb = cv2.cvtColor(left_bgr, cv2.COLOR_BGR2RGB)
    disp_color = colorize(disparity, invalid_mask)
    da_color = colorize(da_depth)

    # Invalid 영역 경계를 빨간 외곽선으로 표시 (오버레이용)
    overlay = da_color.copy()
    invalid_u8 = invalid_mask.astype(np.uint8) * 255
    contours, _ = cv2.findContours(
        invalid_u8, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )
    cv2.drawContours(overlay, contours, -1, (255, 0, 0), 2)

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    axes[0, 0].imshow(left_rgb)
    axes[0, 0].set_title("Left (Input)")
    axes[0, 1].imshow(disp_color)
    axes[0, 1].set_title(
        f"SGBM Disparity (invalid: {invalid_mask.mean() * 100:.1f}%)"
    )
    axes[1, 0].imshow(da_color)
    axes[1, 0].set_title("Depth Anything v2 (Relative)")
    axes[1, 1].imshow(overlay)
    axes[1, 1].set_title("DA + SGBM Invalid Region (red outline)")
    for ax in axes.flat:
        ax.axis("off")
    plt.tight_layout()
    plt.savefig(out_path, dpi=120, bbox_inches="tight")
    plt.close(fig)


def save_single(image_rgb: np.ndarray, path: Path) -> None:
    """RGB numpy array 를 PNG 로 저장."""
    cv2.imwrite(str(path), cv2.cvtColor(image_rgb, cv2.COLOR_RGB2BGR))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--left", type=Path, required=True, help="Rectified 좌측 이미지 경로"
    )
    parser.add_argument(
        "--right", type=Path, required=True, help="Rectified 우측 이미지 경로"
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("build/output/dl_compare"),
        help="결과 저장 디렉토리",
    )
    parser.add_argument(
        "--model",
        type=str,
        default="depth-anything/Depth-Anything-V2-Small-hf",
        help="HuggingFace 모델 ID",
    )
    parser.add_argument(
        "--align-scale",
        action="store_true",
        help="SGBM 유효 픽셀로 DA 의 scale/shift 추정",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    left_bgr = cv2.imread(str(args.left))
    right_bgr = cv2.imread(str(args.right))
    if left_bgr is None or right_bgr is None:
        raise FileNotFoundError(
            f"이미지 로드 실패: {args.left} / {args.right}"
        )
    if left_bgr.shape != right_bgr.shape:
        raise ValueError(
            f"좌우 이미지 크기 불일치: {left_bgr.shape} vs {right_bgr.shape}"
        )

    device = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"[1/3] SGBM disparity 계산 중...")
    disparity, invalid_mask = compute_sgbm_disparity(left_bgr, right_bgr)
    print(
        f"  invalid 비율: {invalid_mask.mean() * 100:.1f}%, "
        f"유효 disparity 범위: [{disparity[~invalid_mask].min():.1f}, "
        f"{disparity[~invalid_mask].max():.1f}]"
    )

    print(f"[2/3] Depth Anything v2 추론 중 (device={device})...")
    da_depth = predict_mono_depth(left_bgr, args.model, device)

    if args.align_scale:
        print(f"[2.5/3] Scale alignment 수행 중...")
        da_depth = align_scale_shift(da_depth, disparity, ~invalid_mask)

    print(f"[3/3] 결과 저장 중 -> {args.output_dir}")
    save_single(colorize(disparity, invalid_mask), args.output_dir / "01_sgbm_disparity.png")
    save_single(
        np.stack([invalid_mask.astype(np.uint8) * 255] * 3, axis=-1),
        args.output_dir / "02_sgbm_invalid_mask.png",
    )
    save_single(colorize(da_depth), args.output_dir / "03_da_depth.png")
    save_comparison(
        left_bgr,
        disparity,
        invalid_mask,
        da_depth,
        args.output_dir / "04_comparison.png",
    )
    print("완료. 04_comparison.png 를 먼저 열어보세요.")


if __name__ == "__main__":
    main()
