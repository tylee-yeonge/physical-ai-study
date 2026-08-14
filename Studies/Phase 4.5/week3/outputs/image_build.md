# week3 학습 측 Docker 이미지 빌드 기록

> 확인일: 2026-08-12
> 용도: week4 의 버전 호환성 대조에서 인용할 학습 측 버전 목록. 그리고 pod 에서 문제가 났을 때 "이미지 조합" 을 용의자 목록에서 지우거나 남기는 판단 재료
> 대상: `openvla-train:v1` (`Studies/Phase 4.5/week3/Dockerfile`)

## 이미지 식별

| 항목 | 값 | 확인 방법 |
|---|---|---|
| 태그 | `openvla-train:v1` | - |
| 이미지 ID | `43782dc016f6` | `docker images openvla-train:v1` |
| 크기 | 23.3GB (압축 전) | 같은 명령의 DISK USAGE |
| 베이스 | `pytorch/pytorch:2.4.0-cuda12.1-cudnn9-devel` | `Dockerfile:4` |
| 베이스 digest | `sha256:a55ff10111eb11f998884327d37361592e632899edd24fce99886b69289e33e6` | 빌드 로그의 FROM 해석 결과 |
| CUDA | 12.1.1 | 컨테이너 시작 배너 |
| python | 3.11 | site-packages 경로가 `/opt/conda/lib/python3.11/` |
| openvla 기준 커밋 | `c8f03f48af692657d3060c19588038c7220e9af9` | week2 `outputs/openvla_base_commit.txt` |
| 적용 패치 | `openvla_registration.patch` (md5 `9a0d1651a230c2f94b7c1e466e297123`) | `md5sum` |

베이스 선택의 제약은 **python 버전**이다. openvla 가 `tensorflow==2.15.0` 을 핀으로 박는데 이 버전은 python 3.12 용 휠이 없다. 3.11 이면 핀이 그대로 선다 (week2 `outputs/env_rlds.md` 의 py3.12 실패 기록이 근거).

## 설치된 버전

`docker run --rm openvla-train:v1 pip list` 기준.

| 패키지 | 버전 | 출처 |
|---|---|---|
| torch | 2.2.0+cu121 | openvla 핀 |
| torchvision | 0.17.0 | openvla 핀 |
| torchaudio | 2.2.0 | openvla 핀 |
| timm | 0.9.10 | openvla 핀 |
| transformers | 4.40.1 | openvla 핀 |
| peft | 0.11.1 | openvla 핀 |
| tensorflow | 2.21.0 | week2 값 |
| tensorflow-datasets | 4.9.10 | week2 값 |
| tensorflow-metadata | 1.21.0 | week2 값 |
| protobuf | 6.33.6 | week2 값 |
| tensorflow-graphics | 2021.12.3 | 양쪽 동일 |
| tensorflow-io-gcs-filesystem | 0.37.1 | 전이 의존성 |
| flash-attn | 2.5.5 | 별도 설치 |

이미지에 **없는** 것: `tensorflow-addons`, `tensorflow-estimator`, `bitsandbytes`. 사유는 아래 표.

## 리포 핀과 다른 항목 (week4 대조용)

week4 는 이 표를 학습 측 버전으로 인용한다. openvla `pyproject.toml` 과 다른 값이 들어간 항목 전부다.

| 패키지 | pyproject | 이미지 | 사유 |
|---|---|---|---|
| tensorflow | `==2.15.0` | 2.21.0 | 핀을 깼다. 2.15 가 protobuf 를 5 미만으로 묶어 아래 §1 이 터진다 |
| tensorflow-datasets | `==4.9.3` | 4.9.10 | 위와 한 묶음. TF 2.21 과 짝이 맞는 버전 |
| protobuf | 핀 없음 | 6.33.6 | 핀이 없어 명시했다. 값은 week2 freeze |
| tensorflow-metadata | 명시 없음 (전이) | 1.21.0 | 핀이 없어 명시했다. 값은 week2 freeze |
| tensorflow-addons | 명시 없음 (`tensorflow-graphics` 가 끌고 옴) | 제거 | TF 2.16 이상과 맞지 않는다. week2 환경에도 없고, 없는 상태로 `tensorflow_graphics` import 가 통과했다 |
| tensorflow-estimator | 전이 (TF 2.15) | 제거 | TF 2.21 이 쓰지 않는 잔재 |
| flash-attn | 45행에 주석 처리 | 2.5.5 설치 | 주석에 "editable install 뒤 별도 설치" 라고 적혀 있다. §2 |
| bitsandbytes | 명시 없음 | 미설치 | `finetune.py:102` 의 `use_quantization` 기본값이 `False` 이고 4-bit 학습 경로를 쓰지 않는다 |

torch / timm 계열은 **핀을 그대로 지켰다.** week2 freeze 에는 torch 2.13.0 / timm 1.0.28 이 적혀 있지만 따라가지 않았다. week2 가 통과시킨 검사는 RLDS 로드 검증 하나이고 그것이 지나는 길은 데이터 경로다. torch 와 timm 은 그 검사가 건드리지 않았으므로 week2 값에 검증 이력이 없고, 반대로 `torch==2.2.0` 은 upstream 이 학습을 돌려 본 조합이다.

## 겪은 문제

### 1. protobuf 와 tensorflow-metadata 의 어긋남

증상: 빌드는 성공하는데 import 에서 죽는다.

```
tensorflow_metadata/proto/v0/anomalies_pb2.py, line 9
    from google.protobuf import runtime_version as _runtime_version
ImportError: cannot import name 'runtime_version'
```

`google.protobuf.runtime_version` 은 protobuf 5.27 부터 생긴 모듈이고, `tensorflow-metadata` 의 `_pb2.py` 는 그 이후 protoc 로 생성돼 이 모듈을 요구한다. 그런데 `tensorflow==2.15.0` 은 protobuf 를 5 미만으로 캡한다. 실제로 깔린 것은 protobuf 4.25.9 였다.

이 조합이 pip 해석 단계에서 안 걸러진 이유는 `tensorflow-metadata` 가 선언한 protobuf 하한이 실제 생성 코드가 요구하는 값보다 느슨하기 때문이다. 그래서 설치는 조용히 성공하고 import 시점에 터진다.

해결: 데이터 경로 4개를 week2 freeze 값으로 덮어썼다. 커밋을 고정해도 **핀이 없는 전이 의존성은 빌드 시점의 최신이 들어온다** — 리포가 2024년 것이라 2026년 지금은 그 격차가 벌어졌고, 하필 protobuf 에서 드러났다.

### 2. flash-attn 은 자동으로 들어오지 않는다

`pyproject.toml:45` 가 `flash_attn==2.5.5` 를 주석 처리해 두고 "editable install 뒤에 따로 깔라" 고 지시한다. `pip install -e .` 만으로는 절대 안 들어온다. 한편 `prismatic/models/backbones/llm/llama2.py:62` 는 `use_flash_attention_2` 기본값을 `True` 로 둔다.

소스 빌드라 컴파일에 시간이 든다. pod 위에서 이걸 발견하면 그 시간을 요금 내며 기다리므로 로컬 이미지에 미리 구웠다. 베이스가 `-devel` 이라 nvcc 가 있어 빌드가 가능했다.

### 3. 빌드는 호스트 셸에서 해야 한다

이 레포를 편집하는 VS Code 세션 자체가 컨테이너 안이다. 그 안에는 docker CLI 도 데몬 소켓도 없어 `docker build` 가 되지 않는다. `apt install docker` 는 무의미하다 — 우분투 저장소의 `docker` 는 Docker 가 아니라 window manager 용 dock 앱이고, 올바른 이름(`docker.io`)으로 깔아도 명령을 받아 줄 데몬이 컨테이너 밖에 있다.

그리고 `Dockerfile` 과 패치가 같은 디렉터리에 있어야 한다. `outputs/` 에서 빌드하면 컨텍스트에 패치가 없어 `COPY` 단계에서 멈춘다. 두 파일 모두 `week3/` 에 둔다.

### 4. 이미지 크기

도커 레이어는 덧쌓기라 뒤 레이어에서 지워도 앞 레이어의 용량은 남는다. 중복이 생기는 자리가 셋이다.

| 중복 | 처리 |
|---|---|
| pip 캐시 | 모든 pip 단계에 `--no-cache-dir` |
| TF 두 벌 (`-e .` 의 2.15 + 덮어쓴 2.21) | 한 `RUN` 으로 병합해 최종 상태만 남긴다 |
| torch 두 벌 (베이스 2.4.0 + 핀 2.2.0) | 미처리. 3-5GB 로 추정 |

23.3GB 는 앞의 둘을 처리한 상태의 크기다. 세 번째는 베이스 태그를 openvla 핀과 같은 torch 버전으로 고르면 사라지지만, 베이스를 바꾸면 python 버전도 함께 바뀌어 TF 설치 가능 여부부터 다시 확인해야 하고 flash-attn 도 다시 컴파일해야 한다. 검증이 끝난 이미지를 깨뜨릴 위험 대비 얻는 것이 작아 두었다.

레지스트리 push 시간은 이 크기에 비례한다.

### 5. `/workspace` 에 클론한 것이 pod 에서 걸린다

이 이미지는 코드를 `/workspace/openvla` 에 두고 editable install 을 걸었다. RunPod 의 network volume 은 pod 의 `/workspace` 에 마운트되어 이 경로를 덮으므로, 볼륨을 붙인 상태에서는 `import prismatic` 이 `ModuleNotFoundError` 로 실패한다. 빈 tmpfs 로 `/workspace` 를 덮어 재현했다.

```
docker run --rm --mount type=tmpfs,destination=/workspace openvla-train:v1 \
    python -c "import prismatic"
ModuleNotFoundError: No module named 'prismatic'
```

**이 이미지는 고치지 않았다.** 대신 pod 배포용 한 겹(`Dockerfile.pod` -> `openvla-train:v2`)에서 코드를 `/opt/openvla` 로 복사하고 editable install 을 재지정했다. 상세는 `runpod_setup.md` §2. 여기를 고치면 뒤 레이어가 전부 무효화되어 flash-attn 부터 다시 컴파일해야 하는데, 얹는 방식은 그 비용이 없다.

이 이미지의 용도가 "로컬에서 검증된 학습 환경" 이라는 것은 그대로다. 잘못 만든 것이 아니라 로컬 실행만 전제한 것이며, 위 검사가 그 전제의 경계를 드러낸다.

## 검증 결과

네 검사 모두 통과. 명령은 `PRACTICE.md` 실습 1 참조.

| 검사 | 출력 |
|---|---|
| 1 mixture 등록 | `['maniskill_pickcube_only']` |
| 2 세 등록 교차 일치 | `maniskill_pickcube 1.0 True True` |
| 3 GPU 가시성 (`--gpus all`) | `2.2.0+cu121 True NVIDIA GeForce RTX 4070` |
| 4 버전 분리 | 위 "설치된 버전" 표와 일치 |

검사 2 가 검사 1 과 별개로 필요한 이유: 패치는 `mixtures.py` / `configs.py` / `transforms.py` 셋을 건드리는데 검사 1 은 첫 번째만 본다. `git apply` 가 all-or-nothing 이라 셋 다 적용된 것은 맞지만, mixture 가 가리키는 데이터셋 이름과 나머지 두 곳의 키가 일치하는지는 별개 문제다. 어긋나면 패치는 깔끔히 적용되고 학습 시작 시 `KeyError` 로 죽는다.

`--gpus all` 없이 실행하면 드라이버 미검출 경고와 `cuInit` 실패가 뜬다. 검사 1-2 는 GPU 를 쓰지 않으므로 무시해도 되는 출력이다.

## 이 기록이 보장하지 않는 것

- **학습을 돌려 본 적이 없다.** 확인한 것은 import 가 통과하고 GPU 가 보인다는 것까지다. 실제 데이터 로드와 스텝 수행은 실습 3 의 probe 가 처음 확인한다
- **flash-attn 이 LoRA 경로에서 실제로 쓰이는지 확인하지 않았다.** `vla-scripts/finetune.py` 는 flash-attn 을 직접 import 하지 않아 코드만으로는 단정할 수 없다. upstream 이 설치를 지시하고 백본 기본값이 `True` 라는 사실에 근거해 넣었다
- **pod 에서의 재현은 확인 전이다.** 볼륨 덮기와 SSH 접속은 배포용 v2 로 로컬에서 확인했으나(`runpod_setup.md` 의 "배포 전 로컬 검증"), 실제 pod 기동은 실습 2 가 닫는다
