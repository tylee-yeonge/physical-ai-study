# week3 RunPod 기동·이관 기록

> 확인일: 2026-08-12
> 용도: "클라우드에서 같은 환경이 재현된다" 의 증거. Section 0 의 "RunPod 에서 기동 + 재현 확인" 항목을 닫는 근거이며, pod 에서 문제가 났을 때 계정 설정·이미지·전송 중 어디를 의심할지 가르는 재료
> 대상: RunPod 계정 설정 + `openvla-train:v2` 이미지 + pod 1대
> 결과: **환경 재현 확인 통과** (§5). 남은 빈 칸은 시간 측정치이며 판정에는 쓰이지 않는다

## 1. 계정 측 설정

### 1.1 반드시 해야 하는 것

| 항목 | 위치 | 상태 | 없으면 |
|---|---|---|---|
| 크레딧 충전 | Billing | 완료 | pod 생성 자체가 안 된다 |
| SSH 공개키 등록 | Credentials > SSH Public Keys | 완료 (호스트 키) | pod 에 접속할 수 없다. 등록한 키가 컨테이너에 `PUBLIC_KEY` 로 주입되고, `runpod_start.sh` 가 그것을 `authorized_keys` 에 넣는다 |
| Container Registry Auth | Credentials > Container Registry Auth | 완료 (`docker-study`) | 커스텀 이미지 pull 이 익명 요청이 되어 Docker Hub 시간당 한도에 걸릴 수 있다 |
| network volume 생성 | Storage | 완료 (`openvla-train` 50GB) | 체크포인트가 pod 와 함께 사라진다 (README §5) |
| 커스텀 Pod 템플릿 | Templates | 완료 (`openvla-train`) | Deploy 화면이 공식 PyTorch 이미지를 기본값으로 두므로, 매번 손으로 채우다 TCP 포트나 환경변수를 빠뜨린다 |

**등록한 키는 호스트(`/home/thira`)의 것 하나다.** 데이터셋은 VS Code 컨테이너 안에만 있는데 그 컨테이너의 키는 등록하지 않았고, **실행 중인 pod 에는 새 키를 주입할 수 없다**(`PUBLIC_KEY` 는 생성 시점에 전달된다). 그래서 전송은 데이터를 호스트가 보는 경로로 꺼낸 뒤 호스트에서 수행한다 (§4).

SSH 공개키는 **전송을 실행할 기계의 키**를 등록해야 한다. RLDS 데이터셋은 VS Code 컨테이너 안(`/root/tensorflow_datasets`, 호스트에는 없는 경로)에 있고 이미지 빌드·push 는 호스트 셸에서 하므로, 두 곳의 키가 다르면 **양쪽 다 등록**한다. 여러 줄로 나열하면 모두 등록된다.

```bash
cat ~/.ssh/id_ed25519.pub    # 각 기계에서 실행해 그 출력을 붙여넣는다
```

Container Registry Auth 에 넣는 값:

| 필드 | 값 |
|---|---|
| Credential name | RunPod 안에서 구분할 라벨 |
| Username | Docker Hub ID (이메일 아님) |
| Password | Docker Hub Access Token. **권한은 Read only** — RunPod 은 받아 가기만 한다 |

등록만으로는 적용되지 않는다. pod 를 만들 때 드롭다운에서 이 credential 을 선택해야 인증된 상태로 pull 한다.

### 1.2 지금은 쓰지 않는 것

| 항목 | 언제 필요한가 |
|---|---|
| API Keys | `runpodctl` 이나 API 로 pod 를 제어할 때. 웹 콘솔로만 다루면 불필요 |
| S3 API Keys | pod 없이 network volume 에 직접 파일을 넣고 뺄 때. 전송이 pod 경유 rsync 로 되면 불필요 |
| Secrets | 토큰 값을 콘솔에 저장해 두고 template 에서 참조할 때. `HF_TOKEN` 을 pod 환경변수에 직접 넣으면 없어도 된다 |

## 2. 이미지 배포

| 항목 | 값 | 확인 방법 |
|---|---|---|
| 태그 | `openvla-train:v2` | - |
| 이미지 ID | `4b2530b5d326` | `docker images openvla-train:v2` |
| 크기 | 23.3GB (압축 전) | 같은 명령의 DISK USAGE |
| 베이스 | `openvla-train:v1` (`image_build.md`) | `Dockerfile.pod:2` |
| 레지스트리 / repository | Docker Hub `tyleeyeonge/openvla-train` | pod 이 이 태그로 기동 |
| pull 인증 | RunPod credential `docker-study` | Container Registry Auth 등록분을 템플릿에서 선택 |
| push 소요 시간 | 미기록 | `time docker push` |
| pod 에서 pull 소요 시간 | 미기록 | |

push 는 기존 Docker Hub 로그인을 건드리지 않기 위해 설정 디렉터리를 분리해 수행했다 (`docker --config ~/.docker-study`). `~/.docker/config.json` 은 registry 별로 항목이 하나뿐이라 `docker login` 을 다시 하면 기존 계정이 덮어써진다.

크기가 v1 과 같은 23.3GB 로 보이는 것은 표시 단위 때문이다. v2 가 더한 것은 openssh-server 와 코드 사본으로, 23GB 옆에서 소수점 아래에 묻힌다. **flash-attn 을 다시 컴파일하지 않았다는 것이 이 수치의 의미**이기도 하다 — v1 레이어를 그대로 재사용했다.

v1 대비 v2 에서 바꾼 것 — 사유는 pod 환경의 두 제약이다.

| 변경 | 사유 |
|---|---|
| 코드를 `/workspace/openvla` 에서 `/opt/openvla` 로 옮기고 editable install 재지정 | network volume 이 pod 의 `/workspace` 에 마운트되어 이미지 안의 같은 경로를 가린다. 그대로 두면 `import prismatic` 이 빈 경로를 가리켜 깨진다 |
| `openssh-server` 설치 + 시작 스크립트 추가 | RunPod 은 `PUBLIC_KEY` 를 주입해 줄 뿐이고 sshd 는 이미지의 몫이다. 베이스 `pytorch/pytorch` 에 없다. 프록시 접속(`ssh.runpod.io`) 은 SCP/SFTP 를 지원하지 않아 rsync 도 이 sshd 를 요구한다 |
| 빌드 중 생성된 `/etc/ssh/ssh_host_*` 삭제 | 패키지 설치 과정이 빌드 시점에 호스트 키를 만들어 이미지에 굽는다. public repository 에 올리면 개인키가 공개돼 중간자 공격을 탐지할 수 없다. 시작 스크립트의 `ssh-keygen -A` 가 컨테이너마다 새로 만든다 |
| 시작 스크립트에서 환경변수를 `/etc/environment` 로 내보냄 | SSH 로 들어온 셸은 Docker 환경변수를 물려받지 않는다. `PATH` 가 없으면 conda 의 python 을 못 찾고, `HF_HOME` 이 무시되면 base 가중치 15GB 가 volume 이 아니라 pod 기본 디스크로 떨어진다 |
| 시작 스크립트 끝에 `sleep infinity` | 시작 명령이 끝나면 pod 가 함께 죽는다 |

`--no-deps` 로 editable 재설치한 이유: `pyproject.toml` 의 `tensorflow==2.15.0` 핀이 다시 해석되면 week2 가 검증한 조합(`image_build.md` §1)이 되돌아간다. 이 레이어가 3.5초에 끝난 것이 의존성 재해석을 건너뛴 증거다.

### 배포 전 로컬 검증

pod 에 올리기 전에 호스트에서 확인한 것. 아래 문제 셋이 여기서 잡혔고, pod 에서 발견됐다면 매번 이미지 수정 -> 23GB push -> 재배포 왕복이었다.

| 검사 | 방법 | 결과 |
|---|---|---|
| 볼륨이 `/workspace` 를 덮어도 코드가 살아 있나 | `--mount type=tmpfs,destination=/workspace` 로 v2 실행 | `ok` |
| 같은 조건에서 v1 은 실패하나 (대조) | 같은 명령을 v1 으로 | `ModuleNotFoundError: No module named 'prismatic'` |
| sshd 가 뜨고 키 인증이 되나 | `-p 2222:22` + `PUBLIC_KEY` 주입 | 접속 성공 |
| SSH 세션에서 python 과 패키지가 보이나 | ssh 로 `python -c "import prismatic"` 실행 | `ok` |

두 번째 줄이 첫 줄의 근거다. 통과만 보면 원래부터 되던 것과 구분되지 않는다.

**검사 방법 자체의 함정**: 처음에는 도커 named volume(`-v vol_test:/workspace`) 으로 덮었는데 v1 까지 통과해 대조가 성립하지 않았다. 비어 있는 named volume 은 **첫 마운트 때 이미지의 해당 경로 내용을 볼륨 안으로 복사해 넣기** 때문이다. pod 의 network volume 은 복사 없이 덮으므로 tmpfs 가 맞는 재현이다.

### 로컬 검증에서 잡은 문제

**1. 컨테이너가 시작 직후 죽는다**

시작 스크립트가 `set -e` 아래에서 `env | grep -E '^(HF_HOME|HF_TOKEN|HUGGINGFACE)'` 를 실행했다. grep 은 매치가 하나도 없으면 실패를 반환하므로, 해당 환경변수를 주지 않은 실행에서는 스크립트가 그 줄에서 끝나고 sshd 에 도달하지 못했다.

증상은 `kex_exchange_identification: Connection reset by peer` (포트 매핑이 아직 살아 있을 때) 또는 `Connection refused` (이미 사라졌을 때) 였고, **`docker logs` 가 CUDA 배너에서 오류 메시지 없이 끊긴 것**이 단서였다. grep 은 조용히 실패하고 `set -e` 도 조용히 끝내므로 로그에 아무것도 남지 않는다. `|| true` 로 막았다.

pod 에서는 `HF_HOME` 을 주므로 우연히 통과했을 것이다. 환경변수 이름 하나가 어긋나면 pod 이 뜨지 않는 구조였다.

**2. SSH 세션에서 `python: command not found`**

베이스 이미지는 python 을 `/opt/conda/bin` 에 두고 그 경로를 Docker `ENV PATH` 로 넣는데, SSH 로 들어온 셸은 Docker 환경변수를 물려받지 않는다. `/etc/environment` 로 내보내는 목록에 `PATH` 가 빠져 있었다. `LD_LIBRARY_PATH` / `CUDA_HOME` 도 같은 이유로 함께 넣었다 — 지금은 GPU 없이 돌려 드러나지 않지만 학습에서 CUDA 라이브러리를 못 찾는 형태로 재발한다.

**3. 호스트 개인키가 이미지에 구워진다**

`openssh-server` 설치 과정이 빌드 시점에 `/etc/ssh/ssh_host_*` 를 만든다. 단서는 시작 로그의 `ssh-keygen: generating new host keys: DSA` 였다 — DSA 만 만든다는 것은 나머지가 이미 있었다는 뜻이다. public repository 에 올리면 그 개인키가 공개돼 중간자 공격을 탐지할 수 없다.

빌드 마지막에 삭제하고 시작 스크립트의 `ssh-keygen -A` 가 컨테이너마다 새로 만들게 했다. 대가로 컨테이너가 새로 뜰 때마다 호스트 키가 바뀌므로 재접속 시 `ssh-keygen -R "[주소]:[포트]"` 가 필요하다. **pod 을 stop/start 하는 실습 5 에서 같은 일이 일어나고, 그때 rsync 가 실패하는 것처럼 보인다** — 실제로는 ssh 가 접속을 거부한 것이다.

## 3. pod 사양

| 항목 | 값 |
|---|---|
| GPU 종류 / VRAM | RTX 4090 1x, 24564MiB (`nvidia-smi`) |
| 호스트 사양 | 60GB RAM, 12 vCPU |
| 드라이버 / CUDA | 570.195.03 / 12.8 (이미지는 12.1 — 드라이버가 상위여서 그대로 동작) |
| 시간당 요금 | GPU $0.74 + container disk $0.007 = **$0.75/hr** (On-Demand, 2026-08-12 확인) |
| 데이터센터 / 티어 | EU-RO-1 / Secure Cloud |
| container disk 크기 | 50GB |
| network volume | `openvla-train` 50GB, `/workspace` (`mfs#euro.runpod.net:9421`, MooseFS) |
| 노출 포트 | 템플릿에 TCP 22 |
| SSH 접속 방식 | **프록시(`ssh.runpod.io`) 로 접속.** 직접 TCP 는 미확인 — §6 |

Secure Cloud 를 고른 이유는 Community Cloud 가 재시작·마이그레이션 시 public IP 를 바꿀 수 있어서다. 직접 TCP 로 파일을 옮기는 절차가 IP 변동에 취약하다.

**EU-RO-1 의 4090 재고는 `2 max` 였다.** stop 후 start 가 재고 때문에 실패할 수 있고, 볼륨이 이 데이터센터에 묶여 있어 다른 지역으로 피할 수도 없다. 학습을 중간에 멈추는 실습 5 에서 이것이 실제 위험이 된다.

`df -h /workspace` 가 총량을 2.3P 로 보여주는 것은 MooseFS 공유 파일시스템이라서다. 50GB 는 쿼터로 걸리며 df 에는 나타나지 않는다.

데이터센터를 먼저 정하는 이유: network volume 은 만든 데이터센터에 종속되고 pod 는 그 데이터센터의 재고 안에서만 뜬다. 볼륨부터 만들면 "볼륨은 있는데 24GB GPU 가 없는" 상태가 된다.

volume 크기 산정 근거: base 가중치 15GB + 데이터셋 0.25GB + 체크포인트. 체크포인트는 저장 시 어댑터가 base 에 머지되어(README §7) 회당 15GB 급이고, 시점별 보관을 고르면 회수만큼 곱해진다. 보관 방식 선택은 `recovery_check.md` 에 기록한다.

## 4. 데이터 이관

| 항목 | 값 |
|---|---|
| 전송 대상 / 크기 | `~/tensorflow_datasets/maniskill_pickcube` — 250MB, 파일 5개 (+ `practice_load_check.py`) |
| 전송 방식 | `runpodctl send` / `receive` (tar.gz 로 묶어 전송) |
| 전송 시간 | 미기록 (250MB, 체감 수 초) |
| 파일 수 대조 | 5 / 5 |
| 파일 바이트 합 대조 | 261,606,336 / 261,606,336 |

### rsync 가 아니라 runpodctl 을 쓴 이유

이 pod 의 Connect 에는 프록시 접속(`ssh.runpod.io`) 만 있었고, **프록시는 원격 명령 실행을 거부한다.**

```
$ ssh <id>@ssh.runpod.io 'mkdir -p /workspace/data'
Error: Your SSH client doesn't support PTY

$ rsync -avP -e "ssh -i ~/.ssh/id_ed25519" maniskill_pickcube <id>@ssh.runpod.io:/workspace/data/
protocol version mismatch -- is your shell clean?
```

rsync 는 원격에서 rsync 를 실행해 파이프로 통신하는 구조라 exec 가 막히면 성립하지 않는다. `tar cz ... | ssh ... 'tar -x'` 우회도 같은 이유로 실패했다. RunPod 문서가 "프록시는 SCP/SFTP 를 지원하지 않는다" 고 적은 것의 실제 내용이 이것이다.

`runpodctl` 은 일회용 코드로 P2P 전송을 하므로 SSH 를 거치지 않는다. **다만 재개 기능이 없다** — 실습 4 에서 체크포인트 15GB 를 내릴 때는 이 경로로 감당하기 어렵다 (§6).

전송 실행 위치도 제약이 있었다. 데이터셋은 VS Code 컨테이너의 overlay 에 있어 호스트에 존재하지 않고, 컨테이너에는 rsync 도 등록된 SSH 키도 없다. `/workspace` 가 호스트 마운트라는 점을 이용해 데이터를 그쪽으로 복사한 뒤 호스트에서 전송했다.

base 가중치 15GB 는 **올리지 않는다.** pod 에서 HuggingFace 로부터 직접 받는 편이 빠르다. `HF_HOME=/workspace/hf` 로 volume 에 받아 두면 pod 를 다시 만들어도 재다운로드가 없다.

데이터셋 디렉터리의 `dataset_statistics_*.json` 은 week2 가 만든 것이다. 함께 보내야 pod 에서 통계가 재계산되지 않아 로컬과 같은 값이 쓰인다.

## 5. 검증 결과

### 5.1 환경 확인 (접속 직후)

| 검사 | 출력 | 판정 |
|---|---|---|
| `import prismatic` | `ok` | 통과 |
| `import flash_attn` | `2.5.5` | 통과 — **로컬 4070(sm_89) 에서 컴파일된 바이너리가 4090 에서 그대로 import 됐다** |
| `torch.cuda.is_available()` | `2.2.0+cu121 True NVIDIA GeForce RTX 4090` | 통과 |
| `df -h /workspace` | `mfs#euro.runpod.net:9421` | network volume 마운트 확인 |
| `echo $HF_HOME` | `/workspace/hf` | **`/etc/environment` 처리가 SSH 세션까지 전달됐다** |

TensorFlow 가 낸 `Failed to determine cuDNN version` 과 `Skipping registering GPU devices` 는 무시한다. TF 는 데이터 로딩(CPU) 만 담당하고, TF 가 GPU 를 잡으면 오히려 VRAM 을 선점해 학습을 방해한다. 학습을 수행하는 torch 는 위 세 번째 줄에서 `True` 다.

### 5.2 로드 검증 (week2 실습 4 와 같은 스크립트)

경로만 `ln -s /workspace/data /root/tensorflow_datasets` 로 맞추고 스크립트는 고치지 않았다 — 그래야 "같은 코드가 통과했다" 가 성립한다.

| 검사 | 로컬 (week2 `registration.md` §2.1) | pod | 일치 |
|---|---|---|---|
| 인식한 데이터셋 / 가중치 | `maniskill_pickcube` 1.000000 | 동일 | 예 |
| `observation.image_primary` | `(1, 224, 224, 3)` uint8 | 동일 | 예 |
| `action` | `(1, 7)` float32 | 동일 | 예 |
| `task.language_instruction` | `b'pick up the cube'` | 동일 | 예 |
| 통계 파일 | `dataset_statistics_76ad416b...cbe.json` 생성 | **같은 파일을 `Loading existing` 으로 재사용** | 예 |

마지막 줄이 판정의 핵심이다. 통계 해시가 같고 재계산되지 않았다는 것은 **정규화 기준이 로컬과 동일**하다는 뜻이다. 통계 파일이 전송에서 빠졌다면 pod 이 새로 계산해 다른 해시가 생기고, 그러면 추론 단계의 `unnorm_key` 가 어긋난다 (README §6).

**dim 별 min/max 값은 대조 항목이 아니다.** 로더에 `shuffle_buffer_size` 가 걸려 있어 실행마다 다른 샘플이 나오므로 값이 로컬과 달라도 정상이다. 배치에 샘플이 하나뿐이라 `min` 과 `max` 가 같게 보이는 것도 정상이다. 값에 대해 확인할 것은 정규화된 차원이 -1 - 1 안에 있다는 것뿐이다.

> 이 절이 Section 0 의 "RunPod 에서 컨테이너 기동 + 재현 확인" 항목을 닫는다.

## 6. 이 기록이 보장하지 않는 것

- **학습을 돌려 본 적이 없다.** 여기까지 확인한 것은 환경이 뜨고 데이터가 읽힌다는 것까지다. 스텝 수행과 VRAM 은 실습 3 의 probe 가 처음 확인한다
- **flash-attn 은 import 만 확인했다.** 4090 에서 `2.5.5` 가 로드됐지만 어텐션 커널이 실제로 실행되는지는 학습이 돌아야 드러난다. GPU 종류를 바꾸면 이 확인부터 다시 한다
- **회수 경로는 S3 호환 API 로 닫혔다.** 프록시 SSH 는 rsync 를 받지 못하고 `runpodctl` 은 pod -> 로컬 방향에서 막힌다. volume 의 S3 API 가 유일하게 동작한 경로이며 절차와 함정은 `PRACTICE.md` 4-4 에 있다. 단 **`aws s3 cp` 는 HeadObject 403 으로 실패하고 `aws s3api get-object` 를 써야 한다**
- **volume 잔존 확인(실습 5)은 수행하지 못했다.** pod 과 volume 을 함께 정리해 재기동 검증의 대상이 사라졌다. 머지 가중치 15GB 도 그때 소실됐다 (`train_log.md` §3.4)
