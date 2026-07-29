# Week 2 실습: RLDS 변환 -> 등록 -> 로드 검증


> **실습 목표**: week1 데이터를 RLDS 로 바꾸고 OpenVLA 데이터로더에 등록해, 배치 1개를 실제로 꺼내 검사한다.
> **예상 시간**: 8-10시간
> **원칙**: 이번 주의 실패는 예외를 내지 않는다. 실습 4-5 의 검사를 통과하기 전에는 "변환이 끝났다" 고 보지 않는다.


---


## 환경 설정


변환 도구는 TensorFlow 계열이라 sim venv 와 섞지 않는다.


```bash
cd "/workspace/study/physical-ai-study/Studies/Phase 4.5"
python3 -m venv .venv-rlds                                     # 변환·등록·로드 검증용
source .venv-rlds/bin/activate
pip install --upgrade pip
pip install -r week2/requirements.txt
mkdir -p week2/outputs
git clone https://github.com/openvla/openvla ~/openvla          # 등록 대상
git clone https://github.com/kpertsch/rlds_dataset_builder ~/rlds_dataset_builder
cd week2
```


> 설치 중 겪은 문제를 `outputs/env_rlds.md` 에 남긴다. 이 환경이 Section 0 후반 Docker 이미지의 학습 측 명세가 된다.


---


## 실습 1: 포맷 요건 확정


**산출물**: `outputs/format_spec.md`


코드를 쓰기 전에 요건을 upstream 코드에서 사실로 확정한다. 문서가 아니라 코드를 본다.


```bash
cd ~/openvla/prismatic/vla/datasets/rlds/oxe


# 1-1. 등록 대상 3개의 위치와 형태 확인
grep -n "OXE_DATASET_CONFIGS" configs.py | head -3          # 설정 딕셔너리 시작 지점
grep -n "OXE_STANDARDIZATION_TRANSFORMS" transforms.py | tail -3   # 변환 함수 레지스트리
grep -n "OXE_NAMED_MIXTURES" mixtures.py | head -3          # mixture 딕셔너리


# 1-2. action 인코딩 종류와 각 벡터 구성 확인 (회전 표현이 여기서 정해진다)
grep -n -A 12 "class ActionEncoding" ../../../../../prismatic/vla/datasets/rlds/oxe/configs.py \
  2>/dev/null || grep -rn -A 12 "class ActionEncoding" ~/openvla/prismatic


# 1-3. 이미 등록된 데이터셋 하나를 골라 설정 항목을 그대로 읽는다 (내 것의 본보기)
grep -n -A 8 '"bridge_oxe"' configs.py


# 1-4. 가장 단순한 변환 함수 하나를 읽는다 (내 변환 함수의 본보기)
grep -n -B 2 -A 14 "def toto_dataset_transform" transforms.py


# 1-5. 정규화 마스크가 어디서 만들어지는지 확인 (수동인가 자동인가)
grep -rn "action_normalization_mask" ~/openvla/prismatic | head
```


**기록할 것** (`outputs/format_spec.md`)

| 항목 | 확정 내용 | 출처 (파일:줄) |
|---|---|---|
| 데이터 형식 | | upstream README |
| 등록 파일 3개와 각 역할 | | 1-1 |
| 채택할 action 인코딩 + 벡터 구성 | | 1-2 |
| 설정 항목 키 목록 (관측·state·action) | | 1-3 |
| 변환 함수가 읽는 키 / 쓰는 키 | | 1-4 |
| 정규화 마스크의 출처 (자동 파생인지) | | 1-5 |


> 1-2 의 결과로 **회전 표현이 확정된다.** week1 라벨이 그 표현이 아니면 여기서 멈추고 week1 실습 2 로 돌아가 라벨을 재생성한다 — 뒤로 갈수록 되돌리는 비용이 커진다.


---


## 실습 2: RLDS 빌더 작성 + 변환


**파일명**: `practice_build_rlds.py` (빌더 정의) + `tfds build` 실행


템플릿을 복사해 필요한 곳만 고친다. 처음부터 쓰지 않는다.


```bash
cp -r ~/rlds_dataset_builder/example_dataset ~/rlds_dataset_builder/maniskill_pickcube
cd ~/rlds_dataset_builder/maniskill_pickcube
ls                                     # 빌더 .py 와 설정 파일 구성을 먼저 본다
```


빌더 파일에서 고칠 곳은 두 군데다 — **feature 명세**와 **샘플 생성기**.


```python
"""
실습 2: week1 npz 를 RLDS 로 변환하는 빌더

템플릿의 example_dataset 빌더를 복사해 아래 두 메서드만 바꾼다.
클래스 이름과 파일 이름은 템플릿 규칙(디렉터리명과 일치)을 따른다.
"""
import glob                                    # week1 npz 파일 목록
import numpy as np
import tensorflow_datasets as tfds


WEEK1_DATASET = "/workspace/study/physical-ai-study/Studies/Phase 4.5/week1/outputs/dataset"
INSTRUCTION = "pick up the cube"               # week0-1 과 같은 문구 (변하면 변인이 늘어난다)


class ManiskillPickcube(tfds.core.GeneratorBasedBuilder):
    """week1 수집 데이터를 RLDS 로 변환하는 빌더."""

    VERSION = tfds.core.Version("1.0.0")       # 데이터를 바꾸면 올린다 (캐시 혼동 방지)
    RELEASE_NOTES = {"1.0.0": "initial"}

    def _info(self) -> tfds.core.DatasetInfo:
        """관측·action·instruction 의 형상과 dtype 을 선언한다."""
        # 아래 형상은 week0 sim_facts.md 의 카메라 해상도와 맞춰야 한다.
        return self.dataset_info_from_configs(
            features=tfds.features.FeaturesDict({
                "steps": tfds.features.Dataset({
                    "observation": tfds.features.FeaturesDict({
                        "image": tfds.features.Image(       # 관측 이미지
                            shape=(224, 224, 3),            # <- week0 확정 해상도로 교체
                            dtype=np.uint8,
                            encoding_format="png",
                        ),
                    }),
                    "action": tfds.features.Tensor(         # 7차원 라벨 (원시 물리 단위)
                        shape=(7,), dtype=np.float32,
                    ),
                    "discount": tfds.features.Scalar(dtype=np.float32),
                    "reward": tfds.features.Scalar(dtype=np.float32),
                    "is_first": tfds.features.Scalar(dtype=np.bool_),   # episode 첫 스텝
                    "is_last": tfds.features.Scalar(dtype=np.bool_),    # episode 마지막 스텝
                    "is_terminal": tfds.features.Scalar(dtype=np.bool_),
                    "language_instruction": tfds.features.Text(),
                }),
                "episode_metadata": tfds.features.FeaturesDict({
                    "file_path": tfds.features.Text(),      # 어느 npz 에서 왔는지
                    "seed": tfds.features.Scalar(dtype=np.int32),   # week1 seed 를 보존
                }),
            })
        )
        # 템플릿에 language_embedding 항목이 있으면 필요 여부를 확인한다.
        # OpenVLA 는 instruction 문자열을 직접 토크나이즈하므로 없어도 되는지 1-4 에서 판단.

    def _split_generators(self, dl_manager):
        """학습 split 하나만 만든다 (eval 은 sim 에서 직접 돌린다)."""
        return {"train": self._generate_examples(sorted(glob.glob(f"{WEEK1_DATASET}/ep*.npz")))}

    def _generate_examples(self, paths):
        """npz 하나를 episode 하나로 변환해 내놓는다."""
        for path in paths:
            data = np.load(path)                            # week1 산출물
            frames = data["frames"]                         # (T, H, W, 3)
            actions = data["openvla_actions"]               # (T-1, 7) — week1 실습 2 결과
            seed = int(data["seed"])                        # 재현·오염 검사용

            steps = []
            # action 은 t -> t+1 변화이므로 관측도 마지막 프레임을 뺀 길이에 맞춘다.
            for index in range(len(actions)):
                steps.append({
                    "observation": {"image": frames[index]},
                    "action": actions[index].astype(np.float32),   # 정규화하지 않은 원값
                    "discount": 1.0,
                    "reward": float(index == len(actions) - 1),     # 마지막 스텝에만 1
                    "is_first": index == 0,
                    "is_last": index == len(actions) - 1,
                    "is_terminal": index == len(actions) - 1,
                    "language_instruction": INSTRUCTION,
                })

            yield path, {"steps": steps,
                         "episode_metadata": {"file_path": path, "seed": seed}}
```


변환 실행:


```bash
cd ~/rlds_dataset_builder/maniskill_pickcube
tfds build                                     # RLDS 데이터셋 생성 (기본 출력은 ~/tensorflow_datasets)
ls ~/tensorflow_datasets/maniskill_pickcube/   # 생성 결과 + 버전 디렉터리 확인
```


**확인 포인트**

- episode 수가 week1 `collect_meta.json` 의 `episodes_saved` 와 같은가
- action 배열의 값이 원시 물리 단위 대역인가 (미리 정규화하지 않았는지 눈으로 재확인)
- 관측 수와 action 수가 1 차이 규칙대로 맞는가


---


## 실습 3: 데이터로더 등록 (3파일)


**산출물**: `outputs/registration.md`


upstream 리포를 수정하는 작업이다. **무엇을 어떻게 바꿨는지 정확히 남긴다** — week3 컨테이너화에서 이 변경을 재현해야 한다.


### 3-1. `configs.py` — 관측·action 공간 명세


실습 1 의 1-3 에서 읽은 기존 항목을 본보기로 내 항목을 추가한다.


```python
# ~/openvla/prismatic/vla/datasets/rlds/oxe/configs.py 의 OXE_DATASET_CONFIGS 안에 추가
"maniskill_pickcube": {
    "image_obs_keys": {"primary": "image", "secondary": None, "wrist": None},   # 빌더의 키 이름
    "depth_obs_keys": {"primary": None, "secondary": None, "wrist": None},      # 깊이 없음
    "state_obs_keys": [None, None, None],        # proprio 를 안 넣었으면 비운다 (형식은 1-3 확인)
    "state_encoding": StateEncoding.NONE,        # <- 1-2 에서 본 실제 이름으로
    "action_encoding": ActionEncoding.EEF_POS,   # <- 7차원 = XYZ delta + RPY + gripper
},
```


> `action_encoding` 이 정규화 마스크까지 결정한다 (README §5). 여기가 틀리면 gripper 가 정규화된다.


### 3-2. `transforms.py` — 표준화 변환 함수


빌더가 이미 표준 형태로 저장했다면 변환 함수는 거의 통과다. 그래도 **함수는 반드시 있어야 한다** (레지스트리에 이름이 없으면 로더가 데이터셋을 못 찾는다).


```python
# ~/openvla/prismatic/vla/datasets/rlds/oxe/transforms.py 하단에 추가
def maniskill_pickcube_transform(trajectory: Dict[str, Any]) -> Dict[str, Any]:
    """RLDS 원시 궤적을 표준 형태로 옮긴다 (7차원 action + instruction)."""
    # action 이 이미 7차원이면 재조립 없이 그대로 둔다.
    # 3개 조각으로 나눠 저장했다면 여기서 tf.concat 으로 합친다 (1-4 의 예시 형태).
    trajectory["language_instruction"] = trajectory["observation"]["language_instruction"] \
        if "language_instruction" in trajectory["observation"] else trajectory["language_instruction"]
    return trajectory


# 같은 파일 하단의 레지스트리에 등록
OXE_STANDARDIZATION_TRANSFORMS["maniskill_pickcube"] = maniskill_pickcube_transform
```


### 3-3. `mixtures.py` — 학습에 쓸 조합


단일 데이터셋이어도 mixture 로 지정해야 학습 스크립트가 인자로 받을 수 있다.


```python
# ~/openvla/prismatic/vla/datasets/rlds/oxe/mixtures.py 의 OXE_NAMED_MIXTURES 안에 추가
"maniskill_pickcube_only": [
    ("maniskill_pickcube", 1.0),     # (데이터셋 이름, 샘플링 가중치)
],
```


### 3-4. 변경 기록


```bash
cd ~/openvla
git diff > "/workspace/study/physical-ai-study/Studies/Phase 4.5/week2/outputs/openvla_registration.patch"
git log -1 --format="%H" > "/workspace/study/physical-ai-study/Studies/Phase 4.5/week2/outputs/openvla_base_commit.txt"
```


`outputs/registration.md` 에 적을 것: 3파일에 각각 무엇을 넣었는지, 기준 커밋 해시, 그리고 **week3 에서 이 변경을 어떻게 재현할지의 잠정 방안** (패치 적용 / fork 사용).


---


## 실습 4: 로드 검증


**파일명**: `practice_load_check.py`


학습을 돌리지 않고 배치 하나를 꺼내 검사한다. 이번 주의 실질적 게이트다.


```python
"""
실습 4: OpenVLA 데이터로더로 배치 1개를 꺼내 스키마·범위를 검사
"""
import numpy as np


print("=" * 60)
print("실습 4: 로드 검증")
print("=" * 60)


# -- 4-1. 데이터셋 생성 --
# openvla 의 RLDS 데이터셋 클래스를 사용한다. 정확한 클래스명·인자는 finetune.py 에서
# 데이터셋을 만드는 부분을 그대로 베껴 온다 (아래 셸 명령으로 위치를 찾는다):
#   grep -n "RLDSDataset\|make_dataset" ~/openvla/vla-scripts/finetune.py
#
# dataset = <finetune.py 와 같은 방식으로 데이터셋 생성>
#   data_root_dir = ~/tensorflow_datasets
#   data_mix      = "maniskill_pickcube_only"
dataset = None                                  # <- 위 한 줄을 구현해 교체


# -- 4-2. 배치 1개 꺼내기 --
batch = next(iter(dataset))                     # 첫 샘플
print("\n[4-2] 배치 키:", list(batch.keys()))    # 어떤 필드가 들어오는지 먼저 본다


# -- 4-3. 항목별 검사 (README §7 의 표) --
print("\n[4-3] 검사")


# (a) 이미지: 형상과 dtype
image = np.asarray(batch["pixel_values"])       # <- 4-2 출력의 실제 키로 교체
print(f"   image shape={image.shape} dtype={image.dtype}")


# (b) action: 차원 수
action = np.asarray(batch["actions"])           # <- 실제 키로 교체
print(f"   action shape={action.shape}")
assert action.shape[-1] == 7, "action 이 7차원이 아니다"


# (c) 정규화 후 범위: 앞 6차원은 대략 [-1, 1] 대역이어야 한다
flat = action.reshape(-1, 7)
for dim in range(6):
    column = flat[:, dim]
    print(f"   dim{dim}: min={column.min():+.3f} max={column.max():+.3f}")
# 자릿수가 크게 벗어나면 이중 정규화(week1 에서 미리 정규화) 또는 통계 오류다


# (d) gripper 차원: 정규화를 거치지 않아 원값 두 갈래로 남아야 한다
gripper = flat[:, 6]
print(f"   dim6(gripper): unique 근사값 {np.unique(np.round(gripper, 2))[:6]}")


# (e) instruction 문자열: week0-1 과 같은 문구인가
print("   instruction:", batch.get("language_instruction", "<키 확인 필요>"))


# (f) episode 경계: 한 배치에 여러 episode 가 섞이는 구조인지 확인
# finetune.py 가 스텝 단위로 샘플링하는지 궤적 단위인지에 따라 다르다 -- 4-2 출력으로 판단해 기록
```


**통과 판정**

| 검사 | 실패 시 |
|---|---|
| action 7차원 | `configs.py` 의 action 인코딩 확인 (실습 3-1) |
| 앞 6차원이 `[-1, 1]` 대역 | week1 에서 미리 정규화했는지 확인 -> 라벨 재생성 |
| gripper 가 두 갈래 원값 | action 인코딩 오등록 확인 |
| instruction 일치 | 빌더의 `INSTRUCTION` 상수 확인 |


검사 출력을 `outputs/load_check.log` 로 저장한다. 이 로그가 "학습 전에 데이터 연결을 확인했다" 의 증거다.


---


## 실습 5: 정규화 계약 확인


**파일명**: `practice_norm_check.py`


통계가 실제로 무엇으로 계산됐는지, 그리고 그것이 낡지 않았는지 본다.


```python
"""
실습 5: 데이터셋 통계 캐시와 정규화 마스크를 확인
"""
import glob
import json


print("=" * 60)
print("실습 5: 정규화 계약")
print("=" * 60)


# -- 5-1. 통계 캐시 파일 찾기 (빌더 데이터 디렉터리에 저장된다) --
candidates = glob.glob("/root/tensorflow_datasets/maniskill_pickcube/**/*.json", recursive=True)
for path in candidates:                         # 후보를 모두 출력해 어느 것이 통계인지 확인
    print("  ", path)


STATS_PATH = candidates[0]                      # <- 실제 통계 파일 경로로 교체
with open(STATS_PATH) as f:
    stats = json.load(f)


# -- 5-2. 통계 내용 확인 --
print("\n[5-2] 통계 키:", list(stats.keys()))
action_stats = stats["action"]                  # <- 실제 구조에 맞게 교체
for name, value in action_stats.items():
    print(f"   {name}: {value}")


# -- 5-3. 마스크 확인: 앞 6개는 정규화 대상, 마지막(gripper)은 제외여야 한다 --
mask = action_stats.get("mask")                 # 키 이름은 5-2 출력으로 확인
print("\n[5-3] 정규화 마스크:", mask)
# 기대: 앞 6개가 True 계열, 마지막 1개가 False 계열


# -- 5-4. 캐시 신선도 확인 --
# 데이터를 재생성했다면 통계도 다시 계산돼야 한다. 파일 수정 시각을 데이터와 비교한다.
import os                                       # (이 실습에서만 쓰는 확인용)
print("\n[5-4] 시각 비교")
print("   통계 파일:", os.path.getmtime(STATS_PATH))
data_files = glob.glob("/root/tensorflow_datasets/maniskill_pickcube/**/*.tfrecord*", recursive=True)
if data_files:
    print("   데이터 파일:", max(os.path.getmtime(p) for p in data_files))
# 데이터가 통계보다 새로우면 캐시를 지우고 다시 만든다
```


**기록할 것** (`outputs/norm_check.md`)

- 통계 파일 경로와 계산 기준 (어떤 통계량인가)
- 마스크 값과 그것이 gripper 를 제외하는지
- 캐시 신선도 판정
- **week1 gripper 부호 규약이 그대로 학습된다는 확인** — 정규화가 보정하지 않으므로, 부호가 맞는지 여기서 마지막으로 점검한다


---


## 마무리: 다음 주로 넘기는 것


| 산출물 | week3 에서의 용도 |
|---|---|
| RLDS 데이터셋 (`~/tensorflow_datasets/...`) | 학습 입력 |
| `outputs/openvla_registration.patch` + `openvla_base_commit.txt` | 컨테이너에서 등록 재현 |
| `outputs/load_check.log` | 학습 전 검증 통과 증거 |
| `outputs/norm_check.md` | 학습 통계 = 추론 `unnorm_key` 의 근거 |
| `outputs/env_rlds.md` | Docker 이미지 학습 측 명세의 원본 |


> RLDS 데이터셋 본체는 커밋하지 않는다 (용량). 대신 재생성 절차 (빌더 파일 + week1 데이터 + `tfds build`) 가 기록돼 있으면 복구 가능하다. 반면 등록 패치와 기준 커밋은 **기록되지 않으면 복구 불가**이므로 반드시 남긴다.
