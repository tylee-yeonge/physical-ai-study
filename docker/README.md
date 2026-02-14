# DS224+ Docker 개발 환경 설정 가이드

DS224+ 순정 상태에서 Visual SLAM & Perception Learning 프로젝트를 실행하기 위한 Docker 환경 구성 가이드.

## 환경 정보

| 항목 | 내용 |
|------|------|
| NAS | Synology DS224+ (Intel Celeron J4125, x86_64) |
| RAM | 6GB (컨테이너 4.5GB 할당) |
| OS | DSM 7.2+ |
| 접속 방식 | VS Code Remote Tunnel |
| 실행 범위 | Phase 1-4 C++ 예제 + Phase 6 경량 Python 예제 |

> **참고**: DS224+ CPU는 AVX 명령어를 지원하지 않아 PyTorch 설치가 불가합니다. Phase 5-6의 딥러닝 실습은 Jetson/Ubuntu/MacBook에서 수행합니다.

---

## 1단계: DSM SSH 활성화

1. 웹 브라우저에서 DSM에 로그인
2. **제어판** → **터미널 및 SNMP** → **터미널** 탭
3. **SSH 서비스 활성화** 체크 → 포트 확인 (기본: 22) → **적용**

## 2단계: Container Manager 설치

1. DSM에서 **패키지 센터** 열기
2. **Container Manager** 검색 → **설치**
3. 설치 완료 확인

## 3단계: NAS IP 확인

NAS의 IP 주소를 먼저 확인합니다. 아래 방법 중 택일:

- **DSM 웹 UI**: **제어판** → **네트워크** → **네트워크 인터페이스** → LAN의 IPv4 주소 확인
- **Mac 터미널**: `ping ds224plus.local` (Bonjour 지원 시)
- **Synology Assistant**: Synology 공식 앱 설치 후 자동 검색

## 4단계: SSH 접속 및 프로젝트 클론

터미널(Mac/Linux) 또는 PowerShell(Windows)에서 NAS에 SSH 접속:

```bash
# <사용자명> = DSM 로그인 ID, <NAS_IP> = 위에서 확인한 IP (예: 192.168.1.100)
ssh <사용자명>@<NAS_IP> -p 22

# 예시:
# ssh yeonge@192.168.1.100 -p 22
```

> 비밀번호는 DSM 웹 로그인 비밀번호와 동일합니다.

### Git 설치 확인

DSM에는 git이 기본 설치되어 있지 않을 수 있습니다. 먼저 확인합니다:

```bash
git --version
```

`command not found` 가 출력되면, 아래 방법 중 택일하여 설치합니다:

**방법 1: DSM 패키지 센터 (권장)**

1. DSM 웹 UI → **패키지 센터** → **Git Server** 검색 → **설치**
2. 설치 후 SSH에서 `git --version` 으로 확인

**방법 2: opkg (Entware) 사용**

```bash
# Entware가 설치되어 있는 경우
sudo opkg install git
```

> Git Server 패키지를 설치하면 Git 서버 기능도 함께 설치되지만, `git` CLI만 사용해도 무방합니다.

### GitHub SSH 키 등록

NAS에서 GitHub 저장소를 클론하려면 SSH 키가 필요합니다.

SSH 키 생성:

```bash
ssh-keygen -t ed25519 -C "your_email@example.com"
# 엔터 3번 (기본 경로, 패스프레이즈 없이)
```

> **DSM 권한 주의**: Synology DSM은 홈 디렉토리 파일이 `0777` 권한으로 생성될 수 있습니다.
> SSH는 개인 키 권한이 너무 열려 있으면 거부하므로, 반드시 아래 명령으로 권한을 수정합니다:
> ```bash
> chmod 700 ~/.ssh
> chmod 600 ~/.ssh/id_ed25519
> chmod 644 ~/.ssh/id_ed25519.pub
> ```

공개 키 확인 및 복사:

```bash
cat ~/.ssh/id_ed25519.pub
```

GitHub에 공개 키 등록:

1. [GitHub SSH Keys 설정](https://github.com/settings/keys) 페이지 열기
2. **New SSH key** 클릭
3. Title: `DS224+` (식별용)
4. Key: 위에서 복사한 공개 키 붙여넣기
5. **Add SSH key** 클릭

연결 테스트:

```bash
ssh -T git@github.com
# "Hi <사용자명>! You've successfully authenticated" 출력 확인
```

### 프로젝트 클론

```bash
sudo mkdir -p /volume1/docker/Learning
cd /volume1/docker/Learning
sudo git clone git@github.com:<사용자명>/visual-slam-and-perception-learning.git
```

> 디렉토리 소유권 설정 (필요 시):
> ```bash
> sudo chown -R $(whoami) /volume1/docker/Learning
> ```

## 5단계: Docker 이미지 빌드

```bash
cd /volume1/docker/Learning/visual-slam-and-perception-learning/docker
sudo docker compose build
```

> 첫 빌드 시 g2o 소스 컴파일 등으로 **20-30분** 정도 소요될 수 있습니다.

## 6단계: 컨테이너 실행

```bash
sudo docker compose up -d
```

## 7단계: VS Code Tunnel 인증

컨테이너 로그를 확인하여 GitHub 인증 URL을 찾습니다:

```bash
sudo docker logs vsp-dev
```

출력에서 아래와 같은 URL이 표시됩니다:

```
To grant access to the server, please log into https://github.com/login/device
and use code XXXX-XXXX
```

1. 웹 브라우저에서 해당 URL 열기
2. 표시된 코드 입력
3. GitHub 계정으로 인증 승인

## 8단계: VS Code에서 접속

1. VS Code 열기 (로컬 PC/MacBook)
2. 확장 프로그램 **Remote - Tunnels** 설치
3. 왼쪽 사이드바 **Remote Explorer** → **Tunnels** 섹션
4. 인증된 NAS 컨테이너가 표시됨 → 클릭하여 접속

---

## 예제 빌드 및 실행

VS Code 터미널(또는 SSH)에서 컨테이너 내부로 접속 후:

### C++ 예제 (Phase 2 Week 1)

```bash
cd /workspace/Studies/Phase\ 2/week1
mkdir -p build && cd build
cmake ..
make -j2
./basic
ctest    # Google Test 실행
```

### Ceres 예제 (Phase 1 Week 8)

```bash
cd /workspace/Studies/Phase\ 1/week8/ceres_example
mkdir -p build && cd build
cmake ..
make -j2
./curve_fitting
```

### g2o 예제 (Phase 3 Week 7)

```bash
cd /workspace/Studies/Phase\ 3/week7
mkdir -p build && cd build
cmake ..
make -j2
./ba_demo
```

### Python 예제 (Phase 6 Week 1)

```bash
cd /workspace/Studies/Phase\ 6/week1
python3 quiz_easy.py
```

> **빌드 팁**: DS224+는 메모리가 제한적이므로 `make -j2`를 권장합니다. `-j4`는 메모리 부족으로 빌드 실패할 수 있습니다.

---

## 컨테이너 관리

```bash
# 컨테이너 상태 확인
sudo docker ps

# 컨테이너 중지
cd /volume1/docker/Learning/visual-slam-and-perception-learning/docker
sudo docker compose stop

# 컨테이너 재시작
sudo docker compose start

# 컨테이너 삭제 후 재생성
sudo docker compose down
sudo docker compose up -d

# 컨테이너 셸 직접 접속
sudo docker exec -it vsp-dev bash
```

---

## 트러블슈팅

### 빌드 시 메모리 부족 (Killed 에러)

```bash
# make 병렬 수를 줄여서 재시도
make -j1
```

### VS Code Tunnel 재인증 필요

컨테이너 재시작 후 tunnel 인증이 풀린 경우:

```bash
sudo docker exec -it vsp-dev code tunnel --accept-server-license-terms
```

로그에서 새 인증 URL을 확인하여 다시 인증합니다.

### g2o CMake 에러 (FindG2O.cmake 관련)

g2o가 `/usr/local/lib/cmake/g2o`에 설치되어 있는지 확인:

```bash
sudo docker exec -it vsp-dev ls /usr/local/lib/cmake/g2o/
```

CMake에서 찾지 못하면 `CMAKE_PREFIX_PATH`를 설정:

```bash
cmake .. -DCMAKE_PREFIX_PATH=/usr/local
```

### Docker 빌드 실패 (네트워크 에러)

NAS의 DNS 설정 확인:

```bash
# DSM 제어판 → 네트워크 → 일반 → DNS 서버
# Google DNS (8.8.8.8, 8.8.4.4) 권장
```

---

## 검증 체크리스트

- [ ] `docker compose build` 성공
- [ ] `docker compose up -d` 후 `docker logs vsp-dev`에서 tunnel URL 출력
- [ ] VS Code Remote Tunnel 접속 성공
- [ ] Phase 2 week1: cmake → make → ./basic → ctest 성공
- [ ] Phase 1 week8: ceres_example 빌드/실행 성공
- [ ] Phase 3 week7: g2o BA demo 빌드/실행 성공
- [ ] Python: `python3 -c "import numpy; import matplotlib; import cv2; print('OK')"` 성공
