# Registration — openvla 데이터로더 등록 기록

> 목적: upstream 리포(`~/openvla`)에 얹은 로컬 변경을 week3 컨테이너에서 재현할 수 있도록 남긴다. 무엇을 넣었는가, 어느 버전 위에 얹었는가, 어떻게 다시 얹을 것인가.
> 작성일: 2026-08-10
> 짝 산출물: [`openvla_registration.patch`](openvla_registration.patch) (변경 내용), [`openvla_base_commit.txt`](openvla_base_commit.txt) (기준 커밋)
> 관련: [`format_spec.md`](format_spec.md) (등록 요건), [`norm_check.md`](norm_check.md) (마스크가 여기서 파생된다)

## 기준 커밋

| 항목 | 값 |
|---|---|
| 리포 | `https://github.com/openvla/openvla` (origin) |
| 브랜치 | `main` |
| 기준 커밋 | `c8f03f48af692657d3060c19588038c7220e9af9` (`c8f03f4`) |
| 커밋 일자 / 제목 | 2025-03-23, `Update README: Remove code block typo` |
| 로컬 변경 범위 | `oxe/configs.py`, `oxe/mixtures.py`, `oxe/transforms.py` 3파일. 그 외 수정 없음 (`git status` 확인) |
| 패치 크기 | 53줄 (3 파일 hunk 4개) |

패치와 기준 커밋을 한 쌍으로 남기는 이유: `git diff` 는 무엇을 바꿨는지만 담고 어느 버전 위에 얹혔는지는 담지 않는다. week3 컨테이너에서 `main` 최신을 받아 패치를 적용하면 상류가 그 사이 같은 자리를 건드렸을 때 충돌한다. 기준 커밋으로 체크아웃한 뒤 적용해야 결과가 확정된다.

## 1. 세 파일에 무엇을 넣었는가

| 파일 | 삽입 위치 | 넣은 것 |
|---|---|---|
| `prismatic/vla/datasets/rlds/oxe/configs.py` | `OXE_DATASET_CONFIGS` 끝 (673행) | `"maniskill_pickcube"` 설정 항목 1개 |
| `prismatic/vla/datasets/rlds/oxe/transforms.py` | 함수 정의 844행, 레지스트리 927행 | 변환 함수 1개 + 레지스트리 1줄 |
| `prismatic/vla/datasets/rlds/oxe/mixtures.py` | `OXE_NAMED_MIXTURES` 끝 (211행) | mixture `"maniskill_pickcube_only"` 1개 |

### 1.1 configs.py — 관측·action 공간 명세

```python
"maniskill_pickcube": {
    "image_obs_keys": {"primary": "image", "secondary": None, "wrist": None},
    "depth_obs_keys": {"primary": None, "secondary": None, "wrist": None},
    "state_obs_keys": [None, None, None],
    "state_encoding": StateEncoding.NONE,
    "action_encoding": ActionEncoding.EEF_POS,
},
```

| 키 | 값의 근거 | 로더에서 실제로 쓰이는 방식 |
|---|---|---|
| `image_obs_keys.primary` | 빌더가 `observation["image"]` 로 저장 | `restructure` 가 `image_primary` 로 옮긴다 (`dataset.py:145-149`). `load_camera_views=("primary",)` 라 나머지 두 슬롯은 materialize 단계에서 걸러진다 |
| `depth_obs_keys` | RGB 만 수집 | `load_depth=False` 면 키 자체가 제거된다 (`materialize.py:60-61`) |
| `state_obs_keys` | proprio 미수집 | 파인튜닝 경로는 `load_proprio=False` 라 키가 통째로 제거된다 (`materialize.py:62-63`). 따라서 `[None, None, None]` 은 이 경로에서 쓰이지 않는다 — 통계 파일의 proprio 가 길이 3 이 아니라 **길이 7 의 0 벡터**로 나온 것이 그 증거다 ([`norm_check.md`](norm_check.md) §5.3) |
| `state_encoding` | proprio 없음 | 마스크 파생 후 제거된다. 정보용 |
| `action_encoding` | 7차원 = XYZ delta 3 + RPY 3 + gripper 1 | **정규화 마스크가 여기서 파생된다** — `[True]*6 + [False]` (`materialize.py:35-39`). 값이 틀리면 gripper 가 정규화되고, 그 실패는 조용하다 |

`state_obs_keys` 를 남겨 둔 채로 `load_proprio=True` 인 경로를 쓰면 3차원 zero proprio 가 만들어진다(`dataset.py:157-168`). 파인튜닝은 그 경로가 아니므로 지금은 무해하지만, 다른 스크립트를 쓸 때 걸릴 수 있는 자리다.

### 1.2 transforms.py — 표준화 변환 함수

```python
def maniskill_pickcube_transform(trajectory: Dict[str, Any]) -> Dict[str, Any]:
    trajectory["language_instruction"] = trajectory["observation"]["language_instruction"] \
        if "language_instruction" in trajectory["observation"] else trajectory["language_instruction"]
    return trajectory
```

레지스트리 등록 (927행):

```python
"maniskill_pickcube": maniskill_pickcube_transform,
```

- 빌더가 action 을 이미 (T, 7) float32 단일 텐서로 저장하므로 재조립(`tf.concat`)이 없다. 변환 함수는 사실상 통과다
- 빌더는 `language_instruction` 을 step 레벨에 저장했다. 따라서 삼항식의 조건은 항상 거짓이고 `trajectory["language_instruction"]` 을 자기 자신에 대입한다 — 동작에는 영향이 없다
- 함수가 하는 일이 없어도 **레지스트리에 이름이 있어야 한다.** `materialize.py:70` 이 `OXE_STANDARDIZATION_TRANSFORMS[dataset_name]` 을 직접 인덱싱하므로, 없으면 `KeyError` 로 막힌다
- 이 함수의 **소스 코드 문자열이 통계 캐시 해시에 들어간다**(`dataset.py:215-219`). 나중에 이 함수를 손대면 통계가 자동으로 재계산된다 ([`norm_check.md`](norm_check.md) §3.1)

### 1.3 mixtures.py — 학습에 쓸 조합

```python
"maniskill_pickcube_only": [
    ("maniskill_pickcube", 1.0),
],
```

학습 스크립트가 받는 `--data_mix` 값이 이 키다. 단일 데이터셋이므로 가중치 `1.0` 은 비율 조절 의미가 없다.

## 2. 등록이 실제로 해석되는지 실행으로 확인

| 확인 항목 | 상태 |
|---|---|
| 3파일에 항목이 들어갔다 | 확인 (`grep -rn maniskill_pickcube prismatic/` 로 5곳) |
| 패치와 기준 커밋 파일이 남았다 | 확인 (`outputs/` 2개 파일) |
| 등록이 로더에서 해석되는가 (kwargs 생성, 마스크 파생) | 확인 — §2.1 |
| 배치가 실제로 꺼내지는가 | 확인 — §2.1 |

### 2.1 실측 증거

`RLDSDataset(data_root_dir=~/tensorflow_datasets, data_mix="maniskill_pickcube_only", ...)` 로 로더를 띄워 배치 하나를 꺼냈다.

| 확인 대상 | 실측 결과 | 어느 등록이 검증되나 |
|---|---|---|
| 로더가 인식한 데이터셋 | `maniskill_pickcube` (가중치 1.000000) | `mixtures.py` 의 mixture 항목. `datasets.py:85-86` 의 mixture 분기를 탔다 |
| `len(dataset)` | 3760 | `configs.py` 항목으로 kwargs 가 생성되어 tfrecord 를 열었다 |
| `observation.image_primary` | `(1, 224, 224, 3)` uint8 | `image_obs_keys.primary = "image"` 매핑. `restructure` 가 슬롯 이름으로 옮겼다 |
| `action` | `(1, 7)` float32 | 변환 함수가 7차원 벡터를 그대로 내놓았다 |
| `absolute_action_mask` | `[F, F, F, F, F, F, T]` | **`action_encoding = EEF_POS` 에서 마스크가 파생됐다** |
| `task.language_instruction` | `b'pick up the cube'` | 변환 함수의 `language_instruction` 처리가 통과했다 |
| 통계 캐시 | `dataset_statistics_76ad416b...cbe.json` 생성 | 변환 함수 소스가 해시에 들어갔다 |

가장 중요한 줄은 `absolute_action_mask` 다. `configs.py` 에 `ActionEncoding.EEF_POS` 한 줄을 적은 것이 `materialize.py:37-38` 을 거쳐 이 값을 만들었다. 이 값이 틀리면 gripper 가 정규화되고 **그 실패는 예외를 내지 않는다.** 마스크 계약의 전체 판정은 [`norm_check.md`](norm_check.md) §5.4 에 있다.

### 2.2 `configs.py` 가 끌고 오는 무관한 의존성

`configs.py:29` 는 자기 자신만으로 import 되지 않는다. `oxe/utils/droid_utils.py:6` 을 거쳐 **`tensorflow_graphics`** 를 요구한다. 내 데이터셋과 무관한 상류 데이터셋(DROID) 때문에 생기는 의존성이므로, week3 컨테이너 명세에도 이 패키지가 들어가야 한다.

그리고 `prismatic/__init__.py:1` 이 `from .models import ...` 로 시작하므로, 데이터로더만 쓰더라도 모델 스택의 import 의존(`huggingface_hub`, `draccus`, `transformers`, `timm`, `torch`, `torchvision`)을 전부 채워야 한다. 어느 패키지가 데이터 경로이고 어느 것이 import 통과용인지는 [`env_rlds.md`](env_rlds.md) 의 진행 표가 구분해 둔다 — week3 이미지를 얇게 만들 때 그 표가 근거다.

## 3. week3 재현 방안 (잠정)

컨테이너에서 같은 코드를 띄우려면 이 3파일 변경을 다시 얹어야 한다. 세 가지 방법을 비교한다.

| 방안 | 절차 | 장점 | 단점 |
|---|---|---|---|
| A. 기준 커밋 pin + 패치 적용 | `git clone` -> `git checkout c8f03f4` -> `git apply openvla_registration.patch` | 산출물 2개로 자기완결. 상류 변동과 무관하게 결과가 확정 | 패치가 리포 밖 파일이라 함께 옮겨야 한다. 상류 최신 수정을 못 받는다 |
| B. fork 후 브랜치 | GitHub fork -> 커밋 -> 컨테이너에서 fork 를 clone | 컨테이너 빌드가 `git clone` 한 줄. 이력이 git 에 남는다 | 원격 저장소 관리 부담. 공개 여부 판단 필요. 상류 재베이스는 별도 작업 |
| C. 파일 통째 복사 (`COPY`) | 수정된 3파일을 이미지에 덮어쓰기 | 가장 단순 | 상류 버전이 바뀌면 조용히 어긋난다. 3파일 전체를 레포에 들고 있어야 한다 |

**잠정 채택: A.** 이번 Phase 의 원칙이 "변인을 늘리지 않는다" 이고, 기준 커밋을 고정하면 상류 변동이라는 변인이 사라진다. B 의 이점(clone 한 줄)은 지금 규모에서 크지 않다. C 는 조용히 어긋나는 실패 방식이라 이번 주의 주제와 정면으로 어긋난다.

Dockerfile 에서의 형태는 이렇게 된다.

```dockerfile
RUN git clone https://github.com/openvla/openvla /opt/openvla \
 && cd /opt/openvla \
 && git checkout c8f03f48af692657d3060c19588038c7220e9af9
COPY openvla_registration.patch /tmp/
RUN cd /opt/openvla && git apply /tmp/openvla_registration.patch
```

확정은 week3 컨테이너화 작업에서 한다. 그때 함께 결정할 것:

- 등록 3파일 외에 추가로 고쳐야 할 upstream 코드가 생기면 패치를 갱신할지, 그 시점에 B 로 갈아탈지
- `tensorflow_graphics` 를 포함한 openvla 의존성을 이미지에 어떻게 넣을지 (§2.2)
- RLDS 데이터셋 디렉터리(`~/tensorflow_datasets/maniskill_pickcube/1.0.0/`)를 이미지에 굽을지 볼륨으로 붙일지

## 4. 정리해 둘 흠

기능에는 영향이 없지만 패치를 다시 적용할 때 눈에 걸리는 것들이다.

| 위치 | 내용 | 영향 |
|---|---|---|
| `configs.py:673` | 항목의 첫 줄 들여쓰기가 8칸 (주변은 4칸) | 없음 (dict 리터럴 내부라 문법상 무관). 패치 재적용 시 diff 가 지저분해진다 |
| `mixtures.py:212` | 줄 끝 공백 | 없음 |
| `transforms.py:844` | 함수 정의와 `# === Registry ===` 사이 빈 줄 1개 (PEP8 은 2개) | 없음 |
| `transforms.py:845-846` | 삼항식의 조건이 항상 거짓 (§1.2) | 없음. 단 이 함수 소스가 통계 해시에 들어가므로, 나중에 정리하면 통계가 재계산된다 |
