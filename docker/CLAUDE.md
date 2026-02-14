# DS224+ Docker 환경 설정

## 환경 정보
- **하드웨어**: Synology DS224+ (Intel Celeron J4125, x86_64, AVX 미지원)
- **RAM**: 6GB (컨테이너 4.5GB 할당)
- **GPU**: 없음
- **컨테이너**: Ubuntu 22.04
- **프로젝트 경로**: /workspace (볼륨 마운트)

## 설치된 라이브러리
- C++: OpenCV 4.5, Eigen 3.4, Ceres 2.0, g2o, Google Test
- Python: numpy, matplotlib, opencv-python-headless (PyTorch 미설치 — AVX 미지원)
- 도구: CMake, Node.js 20.x, VS Code CLI

## 언어
- 응답과 설명은 한국어로 작성
- 코드 주석은 한국어로 작성
- 변수명, 함수명, 클래스명은 영어
- 커밋 메시지는 한국어로 작성

## 빌드 명령어
- DS224+ 메모리 제한으로 `make -j2` 권장 (`-j4`는 메모리 부족 가능)
```bash
cd /workspace/Studies/Phase\ 2/week1
mkdir -p build && cd build
cmake ..
make -j2
ctest
```

## 작업 원칙
- 큰 변경 전에 항상 확인 질문하기
- 기존 코드 패턴을 따르고, 불필요한 리팩토링 하지 않기
- 파일 수정 전에 반드시 먼저 읽기
- 한 번에 하나의 작업에 집중
- 과도한 추상화 금지 — 필요한 만큼만

## 실행 범위
- Phase 1-4 (C++) 예제: 빌드/실행 가능
- Phase 6 (Python 기초): numpy/matplotlib 예제만 실행 가능
- Phase 5-6 (PyTorch/CUDA): 이 환경에서 실행 불가 → Jetson/Ubuntu/MacBook에서 수행

## C++ 스타일
- C++17, Allman 중괄호, 4칸 들여쓰기
- 헤더에 Doxygen 주석 (@brief, @param, @return)
- Google C++ Style 기반 (네이밍, include 순서 등)

## Python 스타일
- Google style docstring (Args, Returns, Raises)
- Black formatter (line-length: 88)
- type hints 적극 사용

## CMake
- cmake_minimum_required(VERSION 3.10)
- 컴파일 경고: -Wall -Wextra
- 테스트: Google Test + ctest
