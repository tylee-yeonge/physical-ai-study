# Format Spec — openvla 커스텀 데이터셋 등록 요건 확정

> 목적: 코드를 쓰기 전에 설치된 openvla 코드(`~/openvla`)에서 등록 3파일의 위치·형식과 action 인코딩을 사실로 확정한다.
> 작성일: 2026-08-10
> 확인 대상: `~/openvla/prismatic/vla/datasets/rlds/` (로컬 설치본을 grep으로 직접 확인)

## 확정 요약

| 항목 | 확정 내용 | 출처 (파일:줄) |
|---|---|---|
| 데이터 형식 | RLDS (TFDS DatasetBuilder로 빌드된 데이터셋). 로더가 `tfds.builder(name, data_dir)`로 열고 `dl.DLataset.from_rlds()`로 읽는다 | `rlds/dataset.py:202, 239` |
| 등록 파일 3개와 각 역할 | `configs.py:54` `OXE_DATASET_CONFIGS` (데이터셋별 설정), `transforms.py:845` `OXE_STANDARDIZATION_TRANSFORMS` (데이터셋명 -> 변환 함수), `mixtures.py:11` `OXE_NAMED_MIXTURES` (mixture명 -> (데이터셋명, 가중치) 리스트) | `oxe/configs.py:54`, `oxe/transforms.py:845`, `oxe/mixtures.py:11` |
| 채택할 action 인코딩 + 벡터 구성 | **EEF_POS** (=1): EEF Delta XYZ (3) + Roll-Pitch-Yaw (3) + Gripper Open/Close (1) = 7차원 | `oxe/configs.py:44-49` |
| 설정 항목 키 목록 | `image_obs_keys`, `depth_obs_keys`, `state_obs_keys`, `state_encoding`, `action_encoding` 5개 | `oxe/configs.py:72-78` (`bridge_oxe` 항목) |
| 변환 함수가 읽는 키 / 쓰는 키 | 읽기: `trajectory["action"]` 하위 키들, `trajectory["observation"]["natural_language_instruction"]`. 쓰기: `trajectory["action"]` (T, 7) float32 텐서, `trajectory["language_instruction"]` | `oxe/transforms.py:283-296` (`toto_dataset_transform`) |
| 정규화 마스크의 출처 | **자동 파생** — `action_encoding`이 EEF_POS면 `action_normalization_mask = [True]*6 + [False]`, `absolute_action_mask = [False]*6 + [True]`이 자동 생성. 수동 기록 불필요 | `oxe/materialize.py:37-40` |

## 회전 표현 게이트 판정: 통과

`ActionEncoding` 중 로더가 실제 지원하는 것은 EEF_POS와 EEF_R6 둘뿐이다 — 그 외는 `materialize.py:32-33`에서 `ValueError`로 거부된다.

- EEF_POS: 회전을 **euler 각(Roll-Pitch-Yaw) delta** 3차원으로 표현
- EEF_R6: 회전을 **R6 표현** 6차원으로 표현 (총 10차원)

week1 라벨은 delta position (3) + delta rotation euler (3, 라디안) + gripper (1)의 7차원이다 (`week1/action_transform.py:71-74`). 이는 EEF_POS의 벡터 구성과 일치하므로 **라벨 재생성 없이 진행한다.**

## 1. 등록 3파일의 역할

세 파일 모두 `~/openvla/prismatic/vla/datasets/rlds/oxe/` 아래에 있다.

| 파일 | 레지스트리 | 역할 |
|---|---|---|
| `configs.py:54` | `OXE_DATASET_CONFIGS` | 데이터셋 이름 -> 설정 dict. 어떤 관측 키를 어떤 카메라 슬롯에 매핑할지, state·action 인코딩이 무엇인지 선언 |
| `transforms.py:845` | `OXE_STANDARDIZATION_TRANSFORMS` | 데이터셋 이름 -> 표준화 변환 함수. 원본 RLDS trajectory를 공통 형식(7차원 action, `language_instruction`)으로 바꾼다 |
| `mixtures.py:11` | `OXE_NAMED_MIXTURES` | mixture 이름 -> `[(데이터셋명, 샘플링 가중치), ...]`. 학습 시 `--data_mix`로 지정하는 이름이 여기 키다 |

내 데이터셋을 쓰려면 세 곳 모두에 한 항목씩 추가한다: 설정 1개, 변환 함수 1개 + 레지스트리 등록 1줄, mixture 1개.

## 2. 설정 항목의 본보기 (`bridge_oxe`, configs.py:72-78)

```python
"bridge_oxe": {
    "image_obs_keys": {"primary": "image", "secondary": "image_1", "wrist": None},
    "depth_obs_keys": {"primary": None, "secondary": None, "wrist": None},
    "state_obs_keys": ["EEF_state", None, "gripper_state"],
    "state_encoding": StateEncoding.POS_EULER,
    "action_encoding": ActionEncoding.EEF_POS,
},
```

- `image_obs_keys`: 카메라 슬롯(primary/secondary/wrist) -> RLDS observation 안의 이미지 키 이름. 없는 슬롯은 `None`. `materialize.py`가 `load_camera_views`에 있는 슬롯이 이 dict에 존재하는지 검사하므로 세 슬롯 키는 모두 적는다
- `depth_obs_keys`: 같은 구조의 depth 매핑. depth가 없으면 전부 `None`
- `state_obs_keys`: proprio state로 이어 붙일 observation 키 리스트. `None`은 해당 자리를 0 패딩
- `state_encoding`: `StateEncoding` enum (`configs.py:33-39`). NONE(-1), POS_EULER(1), POS_QUAT(2), JOINT(3), JOINT_BIMANUAL(4)
- `action_encoding`: `ActionEncoding` enum (`configs.py:44-49`). 아래 표 참조

### ActionEncoding 전체 목록 (configs.py:44-49)

| 값 | 이름 | 벡터 구성 | 로더 지원 |
|---|---|---|---|
| 1 | EEF_POS | EEF Delta XYZ (3) + Roll-Pitch-Yaw (3) + Gripper (1) = 7 | 지원 |
| 2 | JOINT_POS | Joint Delta Position (7) + Gripper (1) = 8 | 거부 (`materialize.py:32-33`) |
| 3 | JOINT_POS_BIMANUAL | 2 x [Joint Delta (6) + Gripper (1)] = 14 | 거부 |
| 4 | EEF_R6 | EEF Delta XYZ (3) + R6 (6) + Gripper (1) = 10 | 지원 |

## 3. 변환 함수의 입출력 규약 (`toto_dataset_transform`, transforms.py:283-296)

```python
def toto_dataset_transform(trajectory: Dict[str, Any]) -> Dict[str, Any]:
    trajectory["action"] = tf.concat(
        (
            trajectory["action"]["world_vector"],
            trajectory["action"]["rotation_delta"],
            tf.cast(trajectory["action"]["open_gripper"][:, None], tf.float32),
        ),
        axis=-1,
    )
    trajectory["language_instruction"] = trajectory["observation"]["natural_language_instruction"]
    return trajectory
```

- 입력: RLDS trajectory dict 하나. 키 이름은 **내 RLDS 빌더가 정의한 그대로** 들어온다 (trajectory 단위라 각 텐서의 앞 차원은 시간 T)
- 출력 계약 두 가지: `trajectory["action"]`을 (T, 7) float32로 만들 것, `trajectory["language_instruction"]`을 채울 것
- tf 그래프 모드에서 실행되므로 numpy가 아니라 `tf.concat`, `tf.cast` 등 tf 연산만 사용
- 내 데이터셋의 action이 이미 7차원 단일 텐서로 저장돼 있으면 concat 없이 dtype 캐스팅과 `language_instruction` 지정만 하면 된다

작성한 함수는 `transforms.py:845`의 `OXE_STANDARDIZATION_TRANSFORMS`에 `"내_데이터셋명": 내_변환_함수` 한 줄로 등록한다.

## 4. 정규화 마스크: 수동 기록 불필요 (materialize.py:37-40)

```python
if dataset_kwargs["action_encoding"] is ActionEncoding.EEF_POS:
    dataset_kwargs["absolute_action_mask"] = [False] * 6 + [True]
    dataset_kwargs["action_normalization_mask"] = [True] * 6 + [False]
```

- `action_encoding`만 올바르게 선언하면 두 마스크가 자동으로 붙는다. 설정 dict에 마스크를 직접 적을 필요가 없다
- 의미: 앞 6차원(위치·회전 delta)은 정규화 대상, 마지막 gripper 차원은 절대값이라 정규화에서 제외
- 이 마스크는 `dataset.py:225-231`에서 통계의 `dataset_statistics["action"]["mask"]`로 저장되어 정규화 단계에 쓰인다

## 5. mixture 항목의 본보기 (mixtures.py:11-)

```python
OXE_NAMED_MIXTURES: Dict[str, List[Tuple[str, float]]] = {
    "bridge": [
        ("bridge_orig", 1.0),
    ],
    ...
}
```

단일 데이터셋 학습이면 `"내_믹스명": [("내_데이터셋명", 1.0)]` 한 항목이면 된다. 가중치는 mixture 내 샘플링 비율이다.
