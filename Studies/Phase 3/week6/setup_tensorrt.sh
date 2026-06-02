#!/usr/bin/env bash
#
# Week 6 실습용 TensorRT 환경 설치/검증 스크립트
#
# 두 플랫폼을 자동 구분한다:
#   - x86_64 데스크톱 dGPU : apt repo에서 TensorRT를 직접 설치한다.
#   - Jetson (aarch64/Tegra) : TensorRT는 JetPack에 포함되므로 설치하지 않고
#                              존재 여부만 확인/안내한다.
#
# 공통으로 하는 일:
#   1. 플랫폼(아키텍처/Jetson 여부)과 CUDA Toolkit major를 자동 감지
#   2. (x86만) "TensorRT 10.x + 감지된 CUDA major" 변형 중 최신을 자동 선택해 설치
#   3. OpenCV(C++) 확인
#   4. 아키텍처에 맞는 경로로 설치 결과 검증
#
# 왜 이렇게 짰나:
#   - 실습 C++ 코드는 TRT 10 API 기준이라 TRT major를 10으로 고정한다.
#     (apt 기본 후보는 최신 11.x를 끌어오므로 명시적으로 막는다.)
#   - x86에서 apt 기본 후보는 최신 CUDA 변형(cuda13 등)을 끌어오지만, 그 PC의
#     CUDA Toolkit과 major가 어긋나면 안 되므로 감지된 major에 맞는 변형만 고른다.
#   - Jetson은 JetPack이 TensorRT를 /usr/src/tensorrt 등에 미리 깔아두므로 apt 설치를
#     하지 않는다. 잘못 apt 설치하면 JetPack 구성과 충돌할 수 있다.
#
# 사용법:
#   bash setup_tensorrt.sh

set -euo pipefail

# 고정 상수: 실습 자료가 TRT 10 API 기준이므로 major를 10으로 박는다.
readonly TRT_MAJOR=10

# root가 아니면 apt 명령에 sudo를 붙인다.
if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
else
    SUDO="sudo"
fi

# 아키텍처별 multiarch 경로 조각 (x86_64-linux-gnu / aarch64-linux-gnu)
ARCH=$(uname -m)
MULTIARCH="${ARCH}-linux-gnu"

# Jetson(Tegra) 여부 판별: tegra 릴리스 파일 또는 device-tree 모델명으로 확인한다.
is_jetson() {
    [ -f /etc/nv_tegra_release ] && return 0
    if [ -f /proc/device-tree/model ] \
       && tr -d '\0' < /proc/device-tree/model | grep -qiE 'jetson|orin|tegra'; then
        return 0
    fi
    return 1
}

echo "=============================================="
echo " Week 6 TensorRT 환경 설치/검증"
echo "=============================================="
echo "[INFO] 아키텍처: $ARCH (multiarch: $MULTIARCH)"

# --- 공통 전제 확인: nvcc(CUDA Toolkit) ---
# C++ 빌드는 CUDA Toolkit이 있어야 가능하다. x86/Jetson 모두 필요하다.
if ! command -v nvcc >/dev/null 2>&1; then
    echo "[ERROR] nvcc를 찾을 수 없습니다. CUDA Toolkit이 설치돼 있어야 합니다."
    echo "        PATH에 /usr/local/cuda/bin 이 포함됐는지 확인하세요."
    exit 1
fi
CUDA_FULL=$(nvcc --version | grep -oP 'release \K[0-9]+\.[0-9]+')
CUDA_MAJOR=${CUDA_FULL%%.*}
echo "[INFO] 감지된 CUDA Toolkit: $CUDA_FULL (major=$CUDA_MAJOR)"

# --- 플랫폼 분기 ---
if is_jetson; then
    # ===== Jetson 경로: 설치하지 않고 JetPack 포함 여부만 확인/안내 =====
    echo "[INFO] 플랫폼: Jetson (Tegra) 감지됨"
    if [ -f "/usr/include/${MULTIARCH}/NvInfer.h" ]; then
        echo "[INFO] TensorRT가 이미 설치돼 있습니다 (JetPack 포함). 설치를 건너뜁니다."
    else
        echo "[ERROR] Jetson에 TensorRT가 보이지 않습니다."
        echo "        JetPack SDK로 설치해야 합니다. 예:"
        echo "          sudo apt-get update && sudo apt-get install nvidia-jetpack"
        echo "        또는 NVIDIA SDK Manager로 플래싱하세요."
        exit 1
    fi
else
    # ===== x86_64 데스크톱 dGPU 경로: apt repo에서 직접 설치 =====
    echo "[INFO] 플랫폼: x86_64 데스크톱 dGPU"

    # NVIDIA 드라이버/GPU 확인 (없어도 빌드는 되지만 엔진 빌드/실행에는 필요)
    if command -v nvidia-smi >/dev/null 2>&1; then
        GPU_NAME=$(nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null | head -1)
        echo "[INFO] 감지된 GPU: ${GPU_NAME:-(확인 실패)}"
    else
        echo "[WARN] nvidia-smi가 없습니다. 엔진 빌드/실행 시 GPU가 필요합니다."
    fi

    # apt repo 확인 및 갱신
    $SUDO apt-get update -qq
    if [ -z "$(apt-cache madison libnvinfer10 2>/dev/null)" ]; then
        echo "[ERROR] apt repo에서 libnvinfer10을 찾을 수 없습니다."
        echo "        NVIDIA CUDA apt repo가 등록돼 있는지 확인하세요."
        echo "        (예: /etc/apt/sources.list.d/cuda*.list)"
        exit 1
    fi

    # 설치할 버전 자동 선택: "TRT major == 10" 이고 "cuda major == 감지값" 인 변형 중 최신.
    # madison 출력 형식: "libnvinfer10 | 10.16.1.11-1+cuda12.9 | <repo url>"
    VER=$(apt-cache madison libnvinfer10 2>/dev/null \
            | awk -F'|' '{gsub(/ /,"",$2); print $2}' \
            | grep -E "cuda${CUDA_MAJOR}\." \
            | grep -E "^${TRT_MAJOR}\." \
            | sort -V | tail -1)
    if [ -z "$VER" ]; then
        echo "[ERROR] TensorRT ${TRT_MAJOR}.x + cuda${CUDA_MAJOR} 변형을 repo에서 찾지 못했습니다."
        echo "        사용 가능한 libnvinfer10 버전 목록:"
        apt-cache madison libnvinfer10 2>/dev/null | awk -F'|' '{print "          " $2}'
        exit 1
    fi
    echo "[INFO] 설치할 TensorRT 버전: $VER"

    # C++ 빌드(헤더/so)와 trtexec, ONNX 파서에 필요한 closure만 같은 버전으로 핀한다.
    # 전부 핀하지 않으면 핀 안 한 의존 패키지가 최신(cuda13 등) 변형으로 튀어 충돌이 난다.
    PKGS=(
        "libnvinfer10=$VER"                  # 런타임 라이브러리
        "libnvinfer-lean10=$VER"             # lean 런타임 (bin 의존)
        "libnvinfer-plugin10=$VER"           # 플러그인 런타임
        "libnvinfer-vc-plugin10=$VER"        # vc-plugin 런타임 (bin 의존)
        "libnvinfer-dispatch10=$VER"         # dispatch 런타임 (bin 의존)
        "libnvonnxparsers10=$VER"            # ONNX 파서 런타임
        "libnvinfer-bin=$VER"                # trtexec 등 CLI 바이너리
        "libnvinfer-dev=$VER"                # C++ 개발 라이브러리 (libnvinfer.so 링크)
        "libnvinfer-headers-dev=$VER"        # C++ 헤더 (NvInfer.h)
        "libnvinfer-safe-headers-dev=$VER"   # safe 헤더 (dev 의존)
        "libnvinfer-plugin-dev=$VER"         # 플러그인 개발 라이브러리
        "libnvinfer-headers-plugin-dev=$VER" # 플러그인 헤더
        "libnvonnxparsers-dev=$VER"          # ONNX 파서 개발 라이브러리
    )
    echo "[INFO] TensorRT 패키지 설치 중..."
    $SUDO apt-get install -y "${PKGS[@]}"
fi

# --- 공통: OpenCV(C++) 확인 ---
# Step 4-5의 전처리/후처리/시각화에 OpenCV C++가 필요하다.
if pkg-config --modversion opencv4 >/dev/null 2>&1; then
    echo "[INFO] OpenCV(C++) 이미 설치됨: $(pkg-config --modversion opencv4)"
elif is_jetson; then
    # Jetson은 JetPack OpenCV가 있을 수 있어 apt 설치로 덮지 않고 안내만 한다.
    echo "[WARN] OpenCV(C++)가 보이지 않습니다. JetPack 구성 또는 다음으로 설치하세요:"
    echo "         sudo apt-get install libopencv-dev"
else
    echo "[INFO] OpenCV(C++) 설치 중..."
    $SUDO apt-get install -y libopencv-dev
fi

# --- 공통: 검증 (아키텍처에 맞는 경로 사용) ---
echo "=============================================="
echo " 설치 검증 ($MULTIARCH)"
echo "=============================================="

# trtexec: PATH 우선, 없으면 Jetson 기본 경로(/usr/src/tensorrt/bin) 확인
TRTEXEC=""
if command -v trtexec >/dev/null 2>&1; then
    TRTEXEC=$(command -v trtexec)
elif [ -x /usr/src/tensorrt/bin/trtexec ]; then
    TRTEXEC=/usr/src/tensorrt/bin/trtexec
fi
if [ -n "$TRTEXEC" ]; then
    echo "[OK] trtexec: $("$TRTEXEC" --version 2>&1 | grep -oE 'TensorRT v[0-9]+' | head -1) ($TRTEXEC)"
    [ "$TRTEXEC" != "$(command -v trtexec 2>/dev/null || true)" ] \
        && echo "     (PATH에 없습니다. export PATH=/usr/src/tensorrt/bin:\$PATH 를 고려하세요.)"
else
    echo "[FAIL] trtexec를 찾을 수 없습니다."
fi

# C++ 헤더
HDR="/usr/include/${MULTIARCH}/NvInfer.h"
[ -f "$HDR" ] && echo "[OK] C++ 헤더: $HDR" || echo "[FAIL] $HDR 없음"

# 링크용 so
SO="/usr/lib/${MULTIARCH}/libnvinfer.so"
[ -e "$SO" ] && echo "[OK] 링크 라이브러리: $SO" || echo "[FAIL] $SO 없음"

# OpenCV
if pkg-config --modversion opencv4 >/dev/null 2>&1; then
    echo "[OK] OpenCV(C++): $(pkg-config --modversion opencv4)"
else
    echo "[FAIL] OpenCV(C++)를 찾을 수 없습니다."
fi

echo "=============================================="
if is_jetson; then
    # Jetson은 엔진을 이 장비에서 새로 빌드해야 하므로 그 절차를 안내한다.
    echo " 완료. [Jetson 다음 단계: TRT 엔진을 이 장비에서 새로 빌드]"
    echo "   .trt 엔진은 빌드한 GPU에서만 동작합니다. 다른 PC에서 만든 .trt는"
    echo "   Jetson에서 재사용할 수 없으므로 반드시 Jetson에서 다시 빌드하세요."
    echo "   1) ONNX 준비 (ONNX는 플랫폼 독립):"
    echo "        - 우분투에서 만든 yolo11n.onnx를 복사해 오거나,"
    echo "        - 없으면 week5 PRACTICE.md를 실행해 ONNX를 먼저 생성하세요."
    echo "   2) TRT 변환 (week6 PRACTICE.md Step 1의 trtexec 사용):"
    echo "        trtexec --onnx=yolo11n.onnx --saveEngine=yolo11n_fp16.trt --fp16"
else
    # x86 데스크톱: ONNX -> TRT 변환이 다음 단계임을 안내한다.
    echo " 완료. [다음 단계: ONNX -> TRT 변환]"
    echo "   week6 PRACTICE.md Step 1의 trtexec로 엔진을 빌드하세요:"
    echo "     trtexec --onnx=yolo11n.onnx --saveEngine=yolo11n_fp16.trt --fp16"
fi
echo "=============================================="
