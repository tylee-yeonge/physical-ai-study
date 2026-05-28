#!/usr/bin/env bash
# requirements.txt 의존성을 venv에 설치하는 스크립트
# PEP 668(externally-managed-environment) 차단을 우회하기 위해 시스템 Python이 아닌 격리된 venv 사용

set -euo pipefail

# 스크립트가 위치한 디렉터리로 이동 (requirements.txt와 같은 위치)
cd "$(dirname "$0")"

# venv 생성에 필요한 시스템 패키지 설치 (apt는 root 권한 필요)
# 이미 설치돼 있으면 apt가 즉시 통과하므로 매번 실행해도 무방
apt-get update
apt-get install -y python3-venv python3-full

# venv 생성 (이미 있으면 건너뜀)
python3 -m venv .venv-week5

# venv 활성화
source .venv-week5/bin/activate

# pip 최신화
pip install --upgrade pip

# 의존성 설치
pip install -r requirements.txt
