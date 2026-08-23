# action 변환 계약 (실습 4)

> 작성일: 2026-07-29
> 대상: OpenVLA `openvla/openvla-7b` 출력 -> ManiSkill `PickCube-v1` / `pd_ee_delta_pose` 입력
> unnorm_key: `bridge_orig`
> sim 쪽 확정값: `outputs/sim_facts.md`

## 계약 표

| # | 항목 | OpenVLA 쪽 | ManiSkill 쪽 | 변환 규칙 | 출처 |
|---|---|---|---|---|---|
| 1 | 위치 델타의 단위·스케일 | 미터. 출력 범위 = `[q01, q99]`, 최대 ±0.042 m/step (5 Hz 기준) | 미터. ±0.1 m/step (20 Hz) | `a = d / 0.1` | `modeling_prismatic.py:528-537`, `panda.py:115-116`, 실측 |
| 2 | 값의 범위 규약 (정규화 여부) | 출력은 **역정규화된 물리량**. `[-1, 1]` 정규화는 토크나이저 내부에서만 쓰이고 반환값에는 남지 않는다 (mask=False 차원 제외) | 입력은 **`[-1, 1]` 정규화값**. `normalize_action=True` | 물리량을 각 축 한계로 나눠 정규화 | 3-1 출력 `Box(-1.0, 1.0, (7,), float32)`, `pd_ee_pose.py:187`, `modeling_prismatic.py:521-537` |
| 3 | 기준 프레임 | bridge 규약의 EEF 델타. 병진은 베이스 기준 | `root_translation:root_aligned_body_rotation` — 병진은 root(베이스) 기준, 회전은 root 정렬 body 회전. `use_delta=True`, `use_target=False` | 프레임 재사상 불필요 | `pd_ee_pose.py:279-285`, `pd_ee_pose.py:179-190` |
| 4 | 회전 표현 | axis-angle 3벡터, 라디안. 최대 ±0.207 rad/step | XYZ 오일러 3벡터, ±0.1 rad/step. **부호가 반전되고** 클리핑이 축별이 아니라 3벡터 노름 기준 | `a = r / (-0.1)`, 노름 > 1 이면 방향 유지한 채 단위 노름으로 축소 | `pd_ee_pose.py:232`, `pd_ee_pose.py:240-243`, 실측 |
| 5 | gripper 부호 규약 | mask=False -> 역정규화 우회, raw 값 통과. 범위 `[0, 1]`, **0 = 닫힘 / 1 = 열림** | 범위 `[-1, 1]`, **-1 = 닫힘 / +1 = 열림** (관절 목표 -0.01 - 0.04 m) | `g = 2 * raw - 1`. raw 가 이산값이므로 0.5 임계 이진화도 가능 | `mask` / `q01` / `q99` = `[..., False]` / `0.0` / `1.0`, `panda.py:179-180`, 실측 |

경로 기준:

```bash
# OpenVLA 원격 코드
/root/.cache/huggingface/hub/models--openvla--openvla-7b/snapshots/47a0ec7fc4ec123775a391911046cf33cf9ed83f/modeling_prismatic.py
# ManiSkill (.venv-sim 기준 상대 경로)
.venv-sim/lib/python3.12/site-packages/mani_skill/agents/robots/panda/panda.py
.venv-sim/lib/python3.12/site-packages/mani_skill/agents/controllers/pd_ee_pose.py
```

## 변환 코드

```python
def to_maniskill_action(raw_action):
    """OpenVLA 역정규화 출력(7,) 을 ManiSkill pd_ee_delta_pose action(7,) 으로 변환한다.

    Args:
        raw_action: vla.predict_action() 출력. [dx,dy,dz](m), [drx,dry,drz](rad), gripper[0,1]

    Returns:
        ManiSkill action (7,) float32. 전 차원 [-1,1] 정규화값
    """
    pos = raw_action[:3] / 0.1                    # 미터 -> 정규화 (±0.1 m 가 ±1)
    rot = raw_action[3:6] / -0.1                  # 라디안 -> 정규화, rot_lower 곱셈 때문에 부호 반전
    rot_norm = np.linalg.norm(rot)                # 회전은 축별이 아니라 노름으로 제한된다
    if rot_norm > 1.0:                            # 노름이 1 을 넘으면 방향을 유지한 채 축소
        rot = rot / rot_norm
    grip = 2.0 * raw_action[6] - 1.0              # [0,1](0=닫힘) -> [-1,1](-1=닫힘)
    action = np.concatenate([np.clip(pos, -1, 1), rot, [np.clip(grip, -1, 1)]])
    return action.astype(np.float32)
```

이 함수는 실행 보드 `#5` 의 `RobotPolicy` adapter + action schema validation 의 첫 sim 구현체다. 별도 유틸로 두지 말고 그 인터페이스 위에 얹는다.

## 근거 1 — OpenVLA 의 역정규화 규약

`predict_action` 이 쓰는 통계량은 **`q01` 과 `q99` 두 개뿐**이다. `min`, `max`, `mean`, `std` 는 참조되지 않는다.

```python
mask = stats.get("mask", 전부 True)
action_high, action_low = q99, q01
actions = np.where(mask,
                   0.5 * (normalized + 1) * (q99 - q01) + q01,   # mask=True
                   normalized)                                    # mask=False
```

`normalized` 는 `bin_centers[...]` (256 bin) 에서 나오고 범위는 `[-1, 1]` 이다. 대입하면 사상은 다음과 같고, 결과적으로 **출력 범위가 정확히 `[q01, q99]`** 다.

| 입력 정규화값 | 출력 |
|---|---|
| -1 | `q01` |
| 0 | `(q01 + q99) / 2` |
| +1 | `q99` |

`min`/`max` 를 쓰지 않는 이유는 `bridge_orig` 통계에서 직접 보인다. 회전 z축의 `max` 가 6.280 = 2π 인데 같은 축의 `q99` 는 0.204 다. 6.28 은 각도 차분이 -π 와 +π 경계를 넘으며 생긴 wrap-around 아티팩트이고, `min`/`max` 로 스케일을 잡으면 이 이상치 하나가 사상 범위를 30배 늘린다. `q01`/`q99` 는 상하위 1%를 잘라 이 오염을 받지 않는다.

## 근거 2 — 단위 판정 (미터·라디안)

`bridge_orig` 의 출력 범위와 초당 환산 (step = 0.2초):

| 차원 | 출력 범위 | step당 최대 크기 | 5 Hz 환산 |
|---|---|---|---|
| 0 (x) | -0.0287 - 0.0283 | 2.87 cm | 14.4 cm/s |
| 1 (y) | -0.0417 - 0.0409 | 4.17 cm | 20.9 cm/s |
| 2 (z) | -0.0261 - 0.0402 | 4.02 cm | 20.1 cm/s |
| 3 (roll) | -0.0809 - 0.0819 | 0.082 rad = 4.7도 | 23도/s |
| 4 (pitch) | -0.0929 - 0.0779 | 0.093 rad = 5.3도 | 27도/s |
| 5 (yaw) | -0.2072 - 0.2038 | 0.207 rad = 11.9도 | 59도/s |

판정 근거는 반대 단위를 가정하면 물리적으로 불가능해지는 것이다.

- 미터로 읽으면 최대 21 cm/s. WidowX 가 테이블 위 물체를 조작하는 속도로 타당하다
- cm 로 읽으면 0.0417 cm/step = 2.1 mm/s. 2초에 4 mm 움직이는 팔은 없다. 기각
- 회전을 도로 읽으면 0.207도/step = 1도/s. 같은 이유로 기각. `max` 가 정확히 2π 인 것이 라디안의 직접 증거다 — 도 단위면 wrap 아티팩트가 360 근처에 찍힌다

## 근거 3 — 마스크

`bridge_orig` 의 `mask` 는 `[True, True, True, True, True, True, False]` 다. **7번째 차원(0-based index 6, gripper)이 역정규화에서 제외**되고 `[-1, 1]` 정규화값이 그대로 통과한다.

그 차원의 `q01` = 0.0, `q99` = 1.0 이라는 값이 규약의 단서다. 원본 bridge 데이터의 gripper action 이 `{0, 1}` 이산값(0 = 닫힘, 1 = 열림)이고, mask 가 False 라서 학습 시에도 정규화 없이 raw 값이 토큰화되었다. 따라서 추론 출력도 raw 규약을 따라 대략 `[0, 1]` 범위이며 0 이 닫힘, 1 이 열림이다.

mask 는 데이터셋 공통 규약이다. `config.json` 의 `norm_stats` 를 직접 열어 확인한 4개 키 전부 `[T,T,T,T,T,T,False]` 이고 dim6 의 `q01`/`q99` 가 전부 `0.0`/`1.0` 이다.

## 근거 4 — ManiSkill 쪽 실측

컨트롤러의 `_clip_and_scale_action` 을 직접 호출해 정규화 입력이 물리량으로 바뀌는 값을 측정했다.

| 설정 | 값 |
|---|---|
| frame | `root_translation:root_aligned_body_rotation` |
| use_delta / use_target | True / False |
| normalize_action | True |
| 위치 한계 (`pos_lower`/`pos_upper`) | -0.1 / 0.1 m |
| 회전 한계 (`rot_lower`/`rot_upper`) | -0.1 / 0.1 rad |
| gripper 관절 범위 | -0.01 - 0.04 m (mimic 컨트롤러) |
| 제어 주기 | 20 Hz (step = 0.05초), 50 step = 2.5초 |

| 입력 action | 실제 물리량 |
|---|---|
| pos x = +1 | +0.1 m |
| pos x = -1 | -0.1 m |
| pos y = +0.5 | +0.05 m |
| rot z = +1 | **-0.1 rad** |
| rot z = -1 | **+0.1 rad** |
| rot x = +0.5 | **-0.05 rad** |
| gripper = -1 | 관절 목표 -0.01 (닫힘) |
| gripper = +1 | 관절 목표 +0.04 (열림) |

**회전 부호 반전의 원인**: `pd_ee_pose.py:232` 가 회전 벡터를 `rot_action * self.config.rot_lower` 로 스케일하고 Panda 의 `rot_lower` 가 `-0.1` 이다. 위치는 `clip_and_scale_action` 으로 `[low, high]` 에 정상 사상되는데 회전만 이 경로를 탄다. 코드를 읽지 않으면 잡히지 않고, 모르고 넣으면 회전이 전부 반대로 간다.

회전 클리핑도 축별이 아니다. 3벡터 노름이 1 을 넘을 때만 방향을 유지한 채 단위 노름으로 축소한다 (`pd_ee_pose.py:240-243`).

## unnorm_key 선택 근거

`bridge_orig` 를 쓴다. 다른 키를 쓰면 계약 표 1번 행이 통째로 바뀐다 — `config.json` 의 `norm_stats` 비교:

| unnorm_key | 위치 q01 / q99 (x축) | 해석 | transitions |
|---|---|---|---|
| `bridge_orig` | -0.029 / 0.028 | 미터, 5 Hz | 2,135,463 |
| `fractal20220817_data` | -0.225 / 0.178 | 미터, 대역 8배 (RT-1, 3 Hz) | 3,786,400 |
| `taco_play` | -0.711 / 0.648 | 미터로 보기 어려운 대역 | 237,798 |
| `austin_buds_...` | -1.000 / 1.000 | 원본이 이미 정규화됨 -> 미터 해석 불가 | 34,112 |

`austin_buds` 의 `q01`/`q99` 가 정확히 ±1 인 것이 결정적이다. **역정규화 출력의 단위는 unnorm_key 마다 다르며 "미터"가 보편 규약이 아니다.**

`bridge_orig` 채택 사유: (a) `[q01, q99]` 대역이 미터로 물리적으로 타당하고 테이블 위 근거리 조작 규모와 맞다, (b) 대역이 ManiSkill Panda 의 step 한계 ±0.1 m 안에 들어 클리핑 없이 사상된다 — `fractal20220817_data` 는 ±0.22 m 로 한계를 2배 넘어 위치 명령이 상시 포화한다, (c) transition 수가 두 번째로 많아 통계가 안정적이다.

## 제어 주기 불일치

`bridge_orig` 의 델타는 0.2초(5 Hz)에 적용될 값이고 ManiSkill step 은 0.05초(20 Hz)다. 그대로 1 step 에 넣으면 같은 변위를 1/4 시간에 실행해 팔이 학습 시보다 4배 빠르게 움직인다.

**action repeat 4** 를 쓴다. OpenVLA 를 한 번 호출하고 같은 action 을 ManiSkill 에 4 step 연속 넣어 실효 주기를 5 Hz 로 맞춘다.

이때 정책 결정 횟수가 step cap 의 1/4 이 되므로 `gym.make(..., max_episode_steps=200)` 으로 늘려 50번의 정책 결정(= 10초 sim 시간)을 확보한다. **이 두 값은 실습 5 의 상한 대조와 실습 6 이 동일하게 써야 한다** — 다르면 상한 대조가 성립하지 않는다. `sim_facts.md` 의 step cap 항목도 이 값으로 맞춘다.

## 실측 확인이 남은 항목

- **bridge 의 5 Hz**: 위 초당 환산과 action repeat 배수의 전제인데 이 환경에서 측정한 값이 아니다. BridgeData V2 데이터셋 카드 또는 Open-X Embodiment 논문의 주기 표로 확인한다. 틀리면 단위 판정의 근거 문장과 action repeat 값이 같이 바뀐다.
- **gripper 출력 범위**: 계약 표 5번의 `[0, 1]` 은 코드가 아니라 학습 파이프라인에서 추론한 값이다. 실습 6 에서 `raw_action[6]` 을 매 step 로그로 남겨 실제 분포가 0 근처와 1 근처에 몰리는지 확인한다. `-1` 근처 값이 나오면 5번 행의 변환 규칙을 고쳐야 한다.
