# week2 RLDS 변환·등록·로드 검증 환경 구축 기록

> 확인일: 2026-08-10
> 용도: Section 0 후반 Docker 이미지의 학습 측 명세 근거. 로드 검증 결과가 흔들릴 때 "환경 조합" 을 용의자 목록에서 지우거나 남기는 판단 재료
> 대상 venv: `Studies/Phase 4.5/.venv-rlds` (week0-1 의 `.venv-sim` 과 분리)

## 확정된 스택

| 항목 | 값 | 확인 방법 |
|---|---|---|
| venv | `Studies/Phase 4.5/.venv-rlds` | - |
| python | 3.12.3 | `python -V`. 컨테이너에 `/usr/bin/python3.12` 만 존재, conda 없음 |
| tensorflow | 2.21.0 | `pip list` |
| tensorflow-datasets | 4.9.10 | `pip list`. `~/tensorflow_datasets/maniskill_pickcube/1.0.0/` 을 이 버전으로 구웠다 |
| numpy | 2.4.6 | `pip list` |
| Pillow | 12.3.0 | `pip list` |
| torch | 2.13.0 | `pip list`. 핀 미적용 (아래 결정 절) |
| torchvision | 0.28.0 | `pip list`. 핀 미적용 |
| timm | 1.0.28 | `pip list`. 핀 미적용 |
| transformers | 4.40.1 | `pip list`. 핀 유지 |
| tokenizers | 0.19.1 | `pip list`. 핀 유지 |
| dlimp | 0.0.1, git commit `040105d` | `pip freeze`. 데이터 파이프라인이 실제로 호출하는 패키지 |
| tensorflow-graphics | 2021.12.3 | `pip freeze`. 핀 유지 |
| openvla | 0.0.3, editable (`~/openvla`) | `pip install -e ~/openvla --no-deps` |
| openvla 기준 커밋 | `c8f03f48af692657d3060c19588038c7220e9af9` | `outputs/openvla_base_commit.txt` |

전체 목록은 `outputs/pip_freeze_rlds.txt` (297개 패키지).

venv 를 sim 과 분리한 이유는 변환 도구가 TensorFlow 계열이기 때문이다. 한 venv 에 섞으면 의존성 해결 과정에서 torch 버전이 갈리고, 그러면 week0-1 의 측정 환경이 깨진다.

## openvla 설치: 리포 지침을 그대로 따를 수 없다

`~/openvla/pyproject.toml:52` 가 `tensorflow==2.15.0` 을 핀으로 박아 두는데, **TF 2.15 에는 Python 3.12 용 휠이 없다.**

```
$ pip install --dry-run --no-deps "tensorflow==2.15.0"
ERROR: Could not find a version that satisfies the requirement tensorflow==2.15.0
       (from versions: 2.16.0rc0, 2.16.1, ... 2.21.0)
ERROR: No matching distribution found for tensorflow==2.15.0
```

openvla README 의 설치 절차는 `conda create -n openvla python=3.10` 을 전제로 쓰여 있다. 이 컨테이너에는 python 3.12 하나뿐이고 conda 도 없다.

**같은 벽이 openvla 한 곳이 아니다.** 데이터 파이프라인이 쓰는 패키지들이 줄줄이 같은 시절에 멈춰 있다.

| 패키지 | 요구하는 것 | py3.12 에서 |
|---|---|---|
| `openvla` (pyproject) | `tensorflow==2.15.0` | 휠 없음 |
| `dlimp` (setup.py) | `tensorflow==2.15.0` | 휠 없음 |
| `tensorflow_graphics==2021.12.3` | `tensorflow-addons>=0.10.0` | **배포 자체가 없음** (`from versions: none`) |

`tensorflow-addons` 는 프로젝트가 종료돼 py3.12 용 배포가 아예 만들어지지 않았다. 즉 이 계열은 `--no-deps` 로만 넘어갈 수 있고, python 3.10 환경이라야 핀이 원래대로 맞는다. **Section 0 후반 Docker 이미지를 3.10 으로 만들 근거가 여기서 하나 더 늘었다.**

openvla pyproject 에서 막힌 것은 tensorflow 핀 하나뿐이다. 나머지 핀은 3.12 에서 설치된다 (`pip install --dry-run --no-deps` 로 각각 확인).

| 패키지 | py3.12 설치 |
|---|---|
| `tensorflow==2.15.0` | 불가 |
| `torch==2.2.0` | 가능 |
| `tokenizers==0.19.1` | 가능 |
| `tensorflow_datasets==4.9.3` | 가능 |
| `tensorflow_graphics==2021.12.3` | 가능 |
| `sentencepiece==0.1.99` | 소스 빌드로 가능 |

## 설치 방식 결정: `--no-deps` + 기존 TF 스택 유지

```bash
pip install -e ~/openvla --no-deps
```

pyproject 의 의존성 해석을 건너뛰어 tensorflow 핀에서 막히지 않게 하고, 부족한 패키지는 import 에러가 지목하는 대로 하나씩 채운다.

근거:

- 이번 주에 필요한 것은 **데이터를 읽는 것**뿐이다. 모델을 올리지 않으므로 `flash-attn`, `peft`, `accelerate`, `wandb` 는 이 단계에 불필요하다. 학습은 RunPod 에서 돈다
- `~/tensorflow_datasets/maniskill_pickcube/` 를 TFDS 4.9.10 으로 구웠다. 여기에 4.9.3 을 덮으면 **빌드에 쓴 버전과 읽는 버전이 갈린다**
- python 3.10 을 따로 설치하는 경로는 이번 주의 목표(로드 검증)에 도달하기까지 시간을 더 쓰고, 그 대가로 얻는 것이 없다

**대가**: upstream 이 테스트한 적 없는 조합이다. 아래 세 항목이 openvla 검증 조합과 다르다.

| 항목 | 이 환경 | openvla pyproject |
|---|---|---|
| tensorflow | 2.21.0 | 2.15.0 |
| tensorflow-datasets | 4.9.10 | 4.9.3 |
| python | 3.12.3 | 3.10 (README 기준) |
| torch | 핀 미적용 (아래 결정 절) | 2.2.0 |

로드 검증 결과가 이상하면 이 표가 용의자 목록의 첫 줄이다.

python 3.10 환경에서는 핀을 그대로 지킬 수 있다. Section 0 후반 Docker 이미지는 3.10 기반으로 만들어 이 차이를 없앤다.

## 의존성 채우기 진행 상태

`python -c "from prismatic.vla.datasets import RLDSDataset; print('ok')"` 가 통과할 때까지 반복한다. 에러가 지목한 모듈만 하나씩 설치해, 데이터 경로와 모델 경로를 구분해 기록한다.

경로 분류 기준은 트레이스백에서 **그 모듈을 요구한 파일**이다. `prismatic/models/...` 면 모델 경로, `prismatic/vla/datasets/rlds/...` 면 데이터 경로.

| 요구 모듈 | 요구한 파일 | 경로 | 설치 버전 |
|---|---|---|---|
| `huggingface_hub` | `prismatic/models/load.py:13` | 모델 | 0.36.2 (아래 상한 주의) |
| `draccus` | `prismatic/conf/datasets.py:17` | 모델 | 0.8.0 (핀 유지) |
| `transformers` | `prismatic/models/materialize.py:10` | 모델 | 4.40.1 (핀 유지) |
| `timm` / `torch` / `torchvision` | `prismatic/models/backbones/vision/base_vision.py:16-19` | 모델 | 1.0.28 / 2.13.0 / 0.28.0 (핀 해제) |
| `dlimp` | `prismatic/vla/datasets/rlds/dataset.py:13` | **데이터** | 0.0.1, git commit `040105d` (`--no-deps`) |
| `tensorflow_graphics` | `prismatic/vla/datasets/rlds/oxe/utils/droid_utils.py:6` | **데이터** | 2021.12.3 (`--no-deps`) |

이 표가 최종본이다. 다섯 라운드로 끝났고, `from prismatic.vla.datasets import RLDSDataset` 이 통과한다.

`prismatic/__init__.py:1` 이 `from .models import ...` 로 시작하므로 `import prismatic` 만으로 모델 스택 전체가 끌려온다. 데이터로더만 쓰더라도 모델 쪽 import 의존은 모두 채워야 한다. 위 분류는 그 안에서 **실행에 실제로 관여하는 것**과 **import 통과용**을 나누기 위한 것이다.

### 경로 칸이 가르는 것

`dlimp` 앞의 네 줄은 전부 "모델" 이다. 이것들은 **import 를 통과시키려고** 깐 것이고, 로드 검증 경로에서 실제로 호출되지 않는다. `dlimp` 는 다르다 — `dataset.py:13` 에서 `dl` 이라는 별명으로 받아 데이터를 읽는 코드 한복판에서 호출된다.

이 구분이 Section 0 후반 Docker 이미지를 얇게 만들 때 **"빼도 되는 것"과 "빼면 안 되는 것"을 가르는 근거**다. 모델 경로 항목은 학습 이미지에서는 어차피 필요하지만, 데이터 변환·검증만 하는 이미지를 따로 만든다면 대부분 뺄 수 있다. 데이터 경로 항목은 어느 이미지에서도 못 뺀다.

같은 이유로 데이터 경로 항목은 버전을 대체하거나 건너뛸 수 없다. 문제가 생기면 openvla README 421-434 줄의 알려진 이슈 항목을 먼저 본다.

### `dlimp` 를 `--no-deps` 로 넣어도 되는 근거

리포의 `setup.py` 가 요구하는 것은 둘뿐이다.

```python
install_requires=[
    "tensorflow==2.15.0",
    "tensorflow_datasets>=4.9.2",
]
```

| 요구 | 현재 | 판정 |
|---|---|---|
| `tensorflow==2.15.0` | 2.21.0 | 핀 불일치. 이미 감수한 차이 |
| `tensorflow_datasets>=4.9.2` | 4.9.10 | 충족 |

즉 `--no-deps` 로 건너뛴 것은 이미 깔린 두 패키지의 재확인뿐이고, 새로 빠지는 것이 없다.

### `tensorflow_graphics` 는 `--no-deps` 외에 길이 없다

이쪽은 사정이 다르다. 의존성 중 `tensorflow-addons>=0.10.0` 이 py3.12 용 배포를 아예 갖고 있지 않다.

```
$ pip install --dry-run "tensorflow_graphics==2021.12.3"
ERROR: Could not find a version that satisfies the requirement tensorflow-addons>=0.10.0
       (from tensorflow-graphics) (from versions: none)
```

`from versions: none` 은 "그 버전이 없다" 가 아니라 **"이 python 에 설치 가능한 배포가 하나도 없다"** 는 뜻이다. `tensorflow-addons` 는 프로젝트가 종료돼 3.12 휠이 만들어지지 않았다.

`--no-deps` 로 넣으면 `scipy`, `trimesh`, `networkx`, `OpenEXR` 등도 함께 빠진다. 그래서 우리가 실제로 쓰는 하위 모듈만 따로 확인했다.

```bash
python -c "import tensorflow_graphics.geometry.transformation; print('tfg ok')"
```

통과한다. `droid_utils.py:6` 이 쓰는 것이 이 모듈뿐이므로 빠진 패키지들은 이 경로에 관여하지 않는다.

### pip 의 dependency conflict 경고는 정상이다

패키지를 설치할 때마다 pip 이 아래 형태의 블록을 낸다.

```
ERROR: pip's dependency resolver does not currently take into account all the packages
       that are installed. ...
openvla 0.0.3 requires torch==2.2.0, which is not installed.
openvla 0.0.3 requires tensorflow==2.15.0, but you have tensorflow 2.21.0 which is incompatible.
```

`--no-deps` 로 openvla 를 얹었으므로 pyproject 의 요구가 채워지지 않은 상태이고, pip 은 그것을 매번 보고할 뿐이다. **`Successfully installed ...` 줄이 함께 나왔다면 그 설치는 성공한 것이다.** 이 블록의 내용은 위 "이 환경 vs openvla pyproject" 표와 아래 진행 표가 이미 추적하고 있다.

### 버전 핀 처리 기준

- pyproject 에 핀이 없는 것(`huggingface_hub`, `einops`, `matplotlib`, `rich` 등) — 최신
- 핀이 있고 모델 경로인 것(`transformers==4.40.1`, `tokenizers==0.19.1`, `draccus==0.8.0`) — 핀을 지킨다. py3.12 에서 설치된다
- 핀이 있고 numpy 를 끌어내리는 것(`torch`, `torchvision`, `timm`) — 핀을 버린다. 아래 결정 절
- 데이터 경로(`dlimp`, `tensorflow_graphics`) — 파이프라인이 직접 호출하므로 대체 불가. 핀과 git 커밋을 그대로 지킨다

"핀 없으면 최신" 은 무조건이 아니다. **핀이 걸린 패키지가 상한을 함께 요구하면 그쪽이 이긴다.** 지금 걸려 있는 사례가 하나 있다.

| 패키지 | 버전 | 상한을 요구하는 쪽 |
|---|---|---|
| `huggingface_hub` | 0.36.2 | `transformers==4.40.1` 이 `huggingface-hub<1.0,>=0.19.3` 요구 |

`huggingface_hub` 은 pyproject 에 핀이 없어 최신(1.x)으로 받았지만, 핀이 걸린 `transformers` 가 `<1.0` 상한을 요구해 0.x 로 내려앉았다. pip 이 알아서 처리하는 정상 동작이다. **"핀 없으면 최신" 은 나중에 설치되는 핀 패키지가 뒤집을 수 있다.**

## torch 버전 결정: 핀을 지키지 않는다

**`torch==2.2.0` 핀을 버리고 numpy 2.4.6 과 함께 사는 최신 torch 를 쓴다.** `timm` 과 `torchvision` 도 같은 기준으로 따라간다.

이 결정이 걸리는 지점은 `prismatic/models/backbones/vision/base_vision.py:16-19` 다. 이 파일이 `timm` / `torch` / `torchvision` 을 한꺼번에 요구하고, `timm==0.9.10` 은 torch 와 torchvision 을 함께 끌어온다.

선택지는 둘이었다.

| 선택지 | 결과 |
|---|---|
| 핀 유지 (`torch==2.2.0`) | numpy 를 1.26.x 로 내려야 한다. torch 2.2.0 은 numpy 1.x 시절 빌드라 numpy 2 위에서 ABI 문제를 낸다. TF 2.21 은 `numpy>=1.26.0` 만 요구하므로 1.26.x 에서 둘 다 살기는 한다 |
| **핀 해제 (채택)** | numpy 2.4.6 을 그대로 둔다 |

근거:

1. **이 환경에서 torch 의 역할은 사실상 import 뿐이다.** 데이터로더가 torch 에서 쓰는 것은 `IterableDataset` 상속과 `torch.tensor` 정도이고(`prismatic/vla/datasets/datasets.py:12-15`), `batch_transform` 을 원본 통과로 두면 `torch.tensor` 경로조차 타지 않는다
2. **numpy 는 이 검증에서 torch 보다 중요한 부품이다.** TF 2.21 이 데이터를 읽고, `practice_load_check.py` 가 값을 판정하는 도구가 numpy 다. 정작 쓰지 않을 torch 를 위해 실제로 쓰는 부품을 흔드는 것은 손해 보는 거래다
3. **torch 버전 재현이 필요한 지점은 여기가 아니다.** 학습은 이 컨테이너가 아니라 RunPod 에서 돈다. 재현 대상은 Section 0 후반 Docker 이미지다

대가: upstream 검증 조합에서 한 발 더 멀어지고, 이상 동작 시 용의자가 하나 는다. 다만 python 3.12 와 TF 2.21 에서 이미 갈라져 있으므로 **한 발 더 가는 비용이 첫 발보다 작다.** 위 "이 환경 vs openvla pyproject" 표에 torch 행을 함께 추적한다.

## 환경 검증: 조합이 실제로 동작한다

import 통과만으로는 부족하다. `--no-deps` 로 조립한 조합이라 **실행 시점에 깨질 가능성**이 남아 있었다. 그래서 로더를 실제로 돌려 배치를 하나 꺼내 봤다.

검증 방법은 `RLDSDataset` 을 생성하고 `next(iter(dataset))` 로 배치 하나를 꺼내는 것이다. 인자는 `data_root_dir=~/tensorflow_datasets`, `data_mix="maniskill_pickcube_only"`, `batch_transform` 은 원본 통과, `resize_resolution=(224, 224)`.

결과:

| 항목 | 값 |
|---|---|
| 로더가 인식한 데이터셋 | `maniskill_pickcube` (가중치 1.000000) |
| 읽은 스플릿 | `train[:95%]` |
| `len(dataset)` | 3760 |
| 배치 최상위 키 | `action`, `observation`, `task`, `dataset_name`, `absolute_action_mask` |
| `action` shape | `(1, 7)` |

통계 캐시도 예상 경로에 생성됐다.

```
/root/tensorflow_datasets/maniskill_pickcube/1.0.0/dataset_statistics_76ad416b...cbe.json
```

| 항목 | 값 |
|---|---|
| 최상위 키 | `action`, `proprio`, `num_transitions`, `num_trajectories` |
| action 통계량 | `mean`, `std`, `max`, `min`, `q01`, `q99` |
| `num_transitions` | 3760 |
| `num_trajectories` | 100 |
| 폴백 경로 `~/.cache/orca/` | 생성되지 않음 (기본 경로 쓰기 성공) |

**판정: 이 환경 조합에서 이상 징후 없음.** python 3.12 / TF 2.21 / 핀 없는 torch 조합이 데이터 읽기 경로에서 문제를 일으키지 않는다. 로드 검증 결과가 흔들릴 때 "환경 조합" 은 용의자 목록에서 뺄 수 있다.

`action` 앞에 붙은 크기 1짜리 축은 `datasets.py:103-104` 의 `window_size=1`, `future_action_window_size=0` 에서 온다. 값 자체의 항목별 검사는 실습 4 의 몫이다.

## 환경 구축 완료

- [x] `from prismatic.vla.datasets import RLDSDataset` 통과
- [x] torch / timm / torchvision 설치. numpy 가 2.4.6 에서 움직이지 않았다 — 핀을 버린 목적이 달성됐다
- [x] `dlimp`, `tensorflow_graphics` 설치 (`--no-deps`)
- [x] `pip freeze > outputs/pip_freeze_rlds.txt`
- [x] 로더 실행 검증 — 배치 1개 획득, 통계 캐시 생성 확인. 이상 징후 없음

이 환경으로 실습 4 를 진행한다. `outputs/load_check.log` 가 이상하면 원인은 환경이 아니라 실습 2-3 의 빌드·등록 내용이나 검사 코드 쪽이다 — 환경은 위 검증으로 배제된다.

## Docker 이미지로 넘길 때

Section 0 후반에 학습 이미지를 만들 때 이 기록에서 가져갈 것.

- **python 3.10 을 쓴다.** 그러면 `tensorflow==2.15.0`, `tensorflow-addons` 핀이 원래대로 맞아 `--no-deps` 우회가 전부 불필요해진다
- **데이터 경로 패키지는 못 뺀다** — `dlimp`(git 커밋 고정), `tensorflow_graphics`, `tensorflow`, `tensorflow-datasets`
- **모델 경로 패키지는 학습 이미지에만 필요하다.** 데이터 변환·검증 전용 이미지를 따로 만든다면 `transformers`, `timm`, `torch`, `torchvision`, `huggingface_hub` 은 뺄 수 있다. 단 `prismatic/__init__.py:1` 이 모델 스택을 끌어오므로, 빼려면 import 경로를 우회하는 방법이 함께 필요하다
