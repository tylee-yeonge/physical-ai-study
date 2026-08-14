# sim 확정 사실 (실습 2-3)

> 작성일: 2026-07-29
> 환경: `.venv-sim` (mani_skill 3.0.1 / sapien 3.0.3 / torch 2.13.0+cu130)
> 이 문서의 값이 week1 데이터 수집, week5 eval 의 변인 고정 기준이 된다

## 확정값

| 항목 | 확정값 | 출처 |
|---|---|---|
| 환경 id | `PickCube-v1` (그 외 `PickCubeSO100-v1`, `PickCubeWidowXAI-v1`, `PickCubeYCB-v1` 등록됨) | 실습 2 의 2-1 출력 |
| action 차원 / 범위 | `Box(-1.0, 1.0, (7,), float32)` — 7차원, 전 차원 `[-1, 1]` 정규화 입력 | 3-1 출력 |
| control mode | `pd_ee_delta_pose` (지원 11종 중 선택) | 3-2 출력 |
| success 의 정확한 정의 | `is_obj_placed & is_robot_static` — 아래 §success 참조 | 3-3 소스코드 |
| step cap | **50** (변경 불가 값이 아니라 등록 기본값. 늘리려면 `gym.make(..., max_episode_steps=N)` 명시) | `@register_env("PickCube-v1", max_episode_steps=50)`, 3-5 의 `steps=50` |
| 부분 도달률 계산에 쓸 상태 접근 경로 | `env.unwrapped` 의 `cube`, `goal_site`, `goal_thresh`, `cube_half_size`, `agent.tcp_pose`, `agent.is_grasping(cube)`, `agent.is_static(0.2)`. 매 step `info` 에 `is_obj_placed` / `is_robot_static` / `is_grasped` 가 이미 실려 온다 | 3-4 출력, 3-3 소스코드 |
| 카메라 키 경로 / 해상도 | 정책 입력: `obs["sensor_data"]["base_camera"]["rgb"]` = `(1, 128, 128, 3)`. 사람용 렌더: `env.render()` = `(1, 512, 512, 3)` | 실습 2 의 2-3 출력, `pick_cube.py` `_default_sensor_configs` |

## success 의 정확한 정의

```python
is_obj_placed  = ||goal_site.pose.p - cube.pose.p|| <= goal_thresh    # 3D 유클리드 거리
is_robot_static = max(|qvel[:-2]|) <= 0.2                             # 손가락 2관절 제외한 arm 7관절
success = is_obj_placed & is_robot_static
```

| 항목 | 값 |
|---|---|
| `goal_thresh` | 0.025 m |
| `cube_half_size` | 0.02 m |
| `cube_spawn_half_size` | 0.1 m (큐브 xy 를 `[-0.1, 0.1]^2` 에서 균일 추출) |
| `max_goal_height` | 0.3 m |
| goal 위치 추출 | xy 는 큐브와 **독립적으로** `[-0.1, 0.1]^2`, z 는 `cube_half_size + U(0, 0.3)` |
| 정지 판정 축 | arm 7관절만 (`qvel[..., :-2]`), 임계 0.2 (max-abs, L2 아님) |
| 판정 주체 | `agent.is_static` 은 Panda 구현, `goal_thresh` 는 `PICK_CUBE_CONFIGS["panda"]` |

**`is_grasped` 는 success 에 들어가지 않는다.** `evaluate()` 가 계산해 `info` 로 내보내지만 AND 조건에서 빠져 있다. 이것이 아래 degenerate seed 문제의 원인이다.

## degenerate seed — reset 직후 이미 success 인 seed 가 있다

3-5 의 `ep08: success=True` 는 random action 이 task 를 푼 것이 아니다. **seed 8 은 reset 직후부터 success 조건을 만족한다.** goal sphere 가 큐브 초기 위치와 겹쳐 생성되었고 (거리 0.0148 m < `goal_thresh` 0.025 m, z 차이 0.0042 m), 초기 로봇은 `qvel = 0` 이라 `is_robot_static` 이 참이다.

reset 직후 20개 seed 의 goal-cube 거리:

| seed | 거리 (m) | z 차이 (m) | `is_obj_placed` | reset 시점 success |
|---|---|---|---|---|
| 0 | 0.2760 | +0.2689 | False | False |
| 1 | 0.2277 | +0.2263 | False | False |
| 2 | 0.0313 | +0.0287 | False | False |
| 3 | 0.2601 | +0.2231 | False | False |
| 4 | 0.3056 | +0.2739 | False | False |
| 5 | 0.2635 | +0.2214 | False | False |
| 6 | 0.2270 | +0.2244 | False | False |
| 7 | 0.1902 | +0.1889 | False | False |
| **8** | **0.0148** | **+0.0042** | **True** | **True** |
| 9 | 0.2914 | +0.2860 | False | False |
| 10 | 0.2940 | +0.2908 | False | False |
| 11 | 0.1171 | +0.0385 | False | False |
| 12 | 0.1281 | +0.0726 | False | False |
| 13 | 0.2828 | +0.2590 | False | False |
| 14 | 0.0600 | +0.0436 | False | False |
| 15 | 0.1797 | +0.1694 | False | False |
| 16 | 0.1930 | +0.1901 | False | False |
| 17 | 0.1683 | +0.1639 | False | False |
| 18 | 0.1657 | +0.1632 | False | False |
| 19 | 0.1730 | +0.0050 | False | False |

`is_robot_static` 은 20개 seed 전부 reset 시점에 True 다 (초기 `qvel = 0`). 즉 **유일한 관문은 `is_obj_placed`** 이고, goal z 가 `U(0, 0.3)` 에서 낮게 뽑히고 xy 까지 우연히 가까우면 "아무 동작 없이 성공"이 성립한다.

seed 2 는 거리 0.0313 m 로 임계값을 간신히 넘겼고, seed 19 는 z 차이가 0.0050 m 인데 xy 가 멀어 통과했다. 즉 degenerate 는 확률적이며 **seed 목록을 바꾸면 다시 나온다.**

3-5 에서 성공이 step 15 에 잡힌 이유: 루프가 reset 직후 `info` 를 검사하지 않고 첫 step 뒤부터 검사하는데, 첫 step 의 random action 으로 `qvel` 이 0.2 를 넘어 `is_robot_static` 이 깨졌다가 관절 속도가 다시 0.2 아래로 내려온 시점이 step 15 였다. 큐브는 그때까지 로봇이 건드리지 않아 goal 반경 안에 그대로 있었다.

재현:

```python
obs, info = env.reset(seed=8)
print(info["success"], info["is_obj_placed"], info["is_grasped"])   # True True False
```

## step cap 판정

`PickCube-v1` 은 `max_episode_steps=50` 으로 등록되어 있어 gymnasium `TimeLimit` 이 50 step 에서 `truncated=True` 를 낸다. 실습 3 의 `STEP_CAP = 100` 은 **도달 불가능한 값**이었고, 출력의 `steps=50` 전부가 이 truncation 이다.

50 step 이 zero-shot 측정에 충분한지는 실습 5 의 상한 대조로 판정한다 — 기성 해법이 50 step 안에 대부분 성공하면 50 을 확정값으로 쓰고, 상한이 낮으면 `gym.make(..., max_episode_steps=N)` 로 늘린 뒤 그 값을 실습 3·5·6 에 동일 적용한다.

## 카메라 2개를 혼동하지 않기

| 용도 | 경로 | 해상도 |
|---|---|---|
| 정책 입력 (실습 6 에서 OpenVLA 에 넣는 것) | `obs["sensor_data"]["base_camera"]["rgb"]` | 128x128 |
| 사람용 렌더 (실습 2 의 통과 확인, eval 영상) | `env.render()` | 512x512 |

`outputs/env_check.png` 는 512x512 사람용 렌더다 (픽셀 평균 94.6 — 검은 화면 아님, 실습 2 통과). 실습 6 의 입력은 128x128 이고 OpenVLA 는 224x224 를 먹으므로 **업스케일**이 된다. 업스케일로 갈지 `CameraConfig` 를 224x224 로 바꿔 생성할지는 baseline 정의의 일부이므로 실습 6 착수 전에 정하고 여기 기록한다.

두 텐서 모두 CUDA 텐서로 반환되므로 `np.asarray()` 가 바로 실패한다. `.cpu().numpy()` 를 거쳐야 한다.

## 결정 필요 — degenerate seed 처리

측정 지표(success)는 상류 정의를 그대로 쓰고, **seed 목록에서 reset 시점 `is_obj_placed == True` 인 seed 를 배제**하는 방식을 권고한다. 위 표에서는 seed 8 이 배제 대상이고, N=20 을 채우기 위해 seed 20 부터 같은 선별을 통과하는 것으로 대체한다.

동시에 매 episode 의 `is_grasped` / `is_obj_placed` / `is_robot_static` 를 기록해 "grasp 없는 성공"을 사후에 식별할 수 있게 한다. 이 세 플래그는 실습 6 의 부분 도달률 4단계와 그대로 대응된다.

goal 최소 높이를 강제하는 env 개조는 채택하지 않는다 — task 정의가 달라지면 공개 수치 대조(README §8 (b))를 쓸 수 없다.

선별 후 확정된 seed 목록은 실습 5 와 실습 6 이 **동일하게** 사용해야 하며, 목록과 배제 사유를 이 문서에 남긴다.
