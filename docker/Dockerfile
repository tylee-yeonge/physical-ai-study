# =============================================================================
# Visual SLAM & Perception Learning - DS224+ 개발 환경
# =============================================================================
# 대상: Synology DS224+ (Intel Celeron J4125, AVX 미지원, 6GB RAM)
# 용도: Phase 1-4 C++ 예제 빌드/실행 + Phase 6 경량 Python 예제
# =============================================================================

FROM ubuntu:22.04

# 대화형 프롬프트 방지
ENV DEBIAN_FRONTEND=noninteractive

# -----------------------------------------------------------------------------
# 1단계: 기본 빌드 도구
# -----------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    pkg-config \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# 2단계: C++ 라이브러리 (apt)
# -----------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    # OpenCV 4.5.4 (Phase 2-3: 영상처리, 특징점, 카메라 캘리브레이션)
    libopencv-dev \
    # Eigen 3.4 (Phase 2-4: 선형대수, 자세 표현)
    libeigen3-dev \
    # Ceres Solver 2.0 (Phase 1, 3: 비선형 최적화, BA)
    libceres-dev \
    # Ceres 의존성
    libgoogle-glog-dev \
    libgflags-dev \
    libatlas-base-dev \
    libsuitesparse-dev \
    # Google Test (소스만 설치됨, 아래에서 빌드)
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# 3단계: Google Test 빌드 (apt는 소스만 설치하므로 직접 빌드)
# -----------------------------------------------------------------------------
RUN cd /usr/src/googletest \
    && cmake . \
    && make -j$(nproc) \
    && make install

# -----------------------------------------------------------------------------
# 4단계: g2o 소스 빌드 (Ubuntu 22.04 apt에 없음)
# Phase 3 week7, week9: 그래프 최적화
# -----------------------------------------------------------------------------
RUN git clone --depth 1 https://github.com/RainerKuemmerle/g2o.git /tmp/g2o \
    && cd /tmp/g2o \
    && mkdir build && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_APPS=OFF \
        -DG2O_BUILD_EXAMPLES=OFF \
        -DG2O_BUILD_APPS=OFF \
    && make -j$(nproc) \
    && make install \
    && ldconfig \
    && rm -rf /tmp/g2o

# -----------------------------------------------------------------------------
# 5단계: Python 경량 환경 (Phase 6 기초 예제용)
# PyTorch 미설치: DS224+ CPU가 AVX 미지원으로 pip PyTorch 불가
# -----------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir \
    numpy \
    matplotlib \
    opencv-python-headless

# -----------------------------------------------------------------------------
# 6단계: Node.js 20.x LTS (Claude Code CLI용)
# -----------------------------------------------------------------------------
RUN curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
    && apt-get install -y --no-install-recommends nodejs \
    && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# 7단계: Claude Code CLI
# -----------------------------------------------------------------------------
RUN npm install -g @anthropic-ai/claude-code

# -----------------------------------------------------------------------------
# 8단계: VS Code CLI (Remote Tunnel용)
# -----------------------------------------------------------------------------
RUN curl -fSL 'https://update.code.visualstudio.com/latest/cli-linux-x64/stable' \
    --output /tmp/vscode_cli.tar.gz \
    && tar -xzf /tmp/vscode_cli.tar.gz -C /usr/local/bin \
    && rm /tmp/vscode_cli.tar.gz

# -----------------------------------------------------------------------------
# 9단계: Claude Code 설정 (사용자 레벨)
# /workspace는 볼륨 마운트로 덮어쓰이므로 /root/.claude/에 복사
# -----------------------------------------------------------------------------
COPY CLAUDE.md /root/.claude/CLAUDE.md
COPY settings.json /root/.claude/settings.json
COPY rules/ /root/.claude/rules/

# -----------------------------------------------------------------------------
# 작업 디렉토리 (볼륨 마운트 지점)
# -----------------------------------------------------------------------------
WORKDIR /workspace

CMD ["bash"]
