"""Dataset & DataLoader 실습용 간이 분류 데이터셋 생성기.

PIL 만으로 3-class 도형 분류 데이터셋 (원/사각형/삼각형, 64x64 RGB) 을 만든다.
외부 다운로드 없이 즉시 생성되며, README 의 CustomDataset 예제가 그대로 동작한다.

파일명 규칙:
    {class_id}_{image_id:03d}.png
    - class_id: 0 = circle, 1 = square, 2 = triangle
    - 예: 0_000.png, 1_007.png, 2_019.png

사용:
    python prepare_demo_dataset.py
    -> ./data/demo_dataset/images/ 에 60장 (클래스당 20장) 생성
"""

import argparse
import random
from pathlib import Path

from PIL import Image
from PIL import ImageDraw


# 클래스 id -> 이름 매핑 (README/실습에서 그대로 참조 가능)
CLASS_NAMES = ["circle", "square", "triangle"]


def random_color(rng: random.Random) -> tuple:
    """채도가 어느 정도 보장된 무작위 RGB 색을 만든다.

    Args:
        rng: 재현성 확보용 random 인스턴스.

    Returns:
        (R, G, B) 0-255 정수 튜플.
    """
    # 너무 어두운 색은 배경(흰색)과 대비가 약하니 80 이상으로 컷
    return (rng.randint(80, 240), rng.randint(80, 240), rng.randint(80, 240))


def draw_circle(draw: ImageDraw.ImageDraw, size: int, rng: random.Random) -> None:
    """이미지 중앙 부근에 살짝 흔든 원을 그린다."""
    # 반지름과 중심을 약간 흔들어서 이미지마다 다른 모양이 나오게 함
    radius = rng.randint(size // 4, size // 3)
    cx = size // 2 + rng.randint(-5, 5)
    cy = size // 2 + rng.randint(-5, 5)
    bbox = [cx - radius, cy - radius, cx + radius, cy + radius]
    draw.ellipse(bbox, fill=random_color(rng))


def draw_square(draw: ImageDraw.ImageDraw, size: int, rng: random.Random) -> None:
    """축 정렬 사각형 (위치/크기 흔들림 포함) 을 그린다."""
    half = rng.randint(size // 4, size // 3)
    cx = size // 2 + rng.randint(-5, 5)
    cy = size // 2 + rng.randint(-5, 5)
    bbox = [cx - half, cy - half, cx + half, cy + half]
    draw.rectangle(bbox, fill=random_color(rng))


def draw_triangle(draw: ImageDraw.ImageDraw, size: int, rng: random.Random) -> None:
    """위쪽 꼭짓점이 있는 이등변 삼각형 (위치/크기 흔들림 포함) 을 그린다."""
    half = rng.randint(size // 4, size // 3)
    cx = size // 2 + rng.randint(-5, 5)
    cy = size // 2 + rng.randint(-5, 5)
    points = [
        (cx, cy - half),  # 위 꼭짓점
        (cx - half, cy + half),  # 왼쪽 아래
        (cx + half, cy + half),  # 오른쪽 아래
    ]
    draw.polygon(points, fill=random_color(rng))


# class_id 별 도형 그리기 함수 매핑
DRAW_FUNCS = [draw_circle, draw_square, draw_triangle]


def make_one_image(class_id: int, size: int, rng: random.Random) -> Image.Image:
    """주어진 class_id 에 해당하는 64x64 RGB 도형 이미지를 만든다.

    Args:
        class_id: 0/1/2 (circle/square/triangle).
        size: 이미지 한 변 픽셀 수.
        rng: 재현성 확보용 random 인스턴스.

    Returns:
        PIL Image (RGB).
    """
    img = Image.new("RGB", (size, size), color=(245, 245, 245))  # 옅은 회색 배경
    draw = ImageDraw.Draw(img)
    DRAW_FUNCS[class_id](draw, size, rng)
    return img


def main():
    """간이 데이터셋을 만들어 ./data/demo_dataset/images/ 에 저장한다."""
    parser = argparse.ArgumentParser(description="Demo image classification dataset generator")
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path(__file__).parent / "data" / "demo_dataset" / "images",
        help="출력 디렉토리 (기본: ./data/demo_dataset/images)",
    )
    parser.add_argument(
        "--per-class",
        type=int,
        default=20,
        help="클래스당 이미지 수 (기본: 20)",
    )
    parser.add_argument("--size", type=int, default=64, help="이미지 한 변 픽셀 수 (기본: 64)")
    parser.add_argument("--seed", type=int, default=42, help="재현성을 위한 시드")
    args = parser.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    rng = random.Random(args.seed)

    total = 0
    for class_id, class_name in enumerate(CLASS_NAMES):
        for image_id in range(args.per_class):
            img = make_one_image(class_id, args.size, rng)
            # 파일명 규칙: {class_id}_{image_id:03d}.png  -> 라벨 추출이 split 한 번으로 끝남
            filename = f"{class_id}_{image_id:03d}.png"
            img.save(args.out_dir / filename)
            total += 1
        print(f"  class {class_id} ({class_name}): {args.per_class} 장 저장")

    print(f"\n총 {total} 장 생성 완료 -> {args.out_dir}")
    print("파일명 규칙: {class_id}_{image_id}.png  (예: 0_000.png = circle)")


if __name__ == "__main__":
    main()
