# expert 확정 사실 (week1 실습 1)

> 작성일: 2026-08-05
> 근거: week0 `outputs/harness_check.md` / `outputs/harness_check.json` / `outputs/sim_facts.md`, week1 `outputs/expert_traj_meta.json` / `outputs/expert_traj.npz`

## 확정값

| 항목 | 확정값 | 출처 |
|---|---|---|
| expert 종류 | **scripted 정책**(privileged state) — 큐브·목표 좌표를 sim 에서 직접 읽고(카메라 불사용) above -> descend -> close -> lift -> hold 5단계로 움직인다. motion planning 해법은 mplib-numpy ABI 불일치 세그폴트로 이 환경에서 실행 불가 | week0 `harness_check.md` §어떤 해법을 썼는가 |
| expert 의 control_mode | `pd_ee_delta_pose` | week0 `harness_check.json` `control_mode`, `harness_check.md` §검증 조건 |
| action 차원과 의미 | `Box(-1.0, 1.0, (7,), float32)` — EEF delta 위치 3 + 회전 3 + gripper 1, 전 차원 `[-1, 1]` 정규화 입력. scripted 해법은 회전 델타를 전 단계에서 0 으로 둔다 (초기 TCP 가 이미 그리퍼 아래 방향) | week0 `sim_facts.md` action 행 + `harness_check.md` |
| TCP pose 접근 경로 | `env.unwrapped.agent.tcp.pose.raw_pose` — `(1, 7)` CPU 텐서(physx_cpu 기준), `reshape(-1)` 하면 위치 3 + 쿼터니언 4(wxyz). 실측 첫 pose 의 쿼터니언이 `(w=-0.011, x=1.000, ...)` 로 x 축 180도 회전 — week0 기록과 일치 | week0 `sim_facts.md` 상태 접근 행 + 실습 1 실행 확인 |
| expert 성공 여부 (seed 100) | **미확정** — 아래 미완 항목 참조. 참고로 week0 상한 대조에서 seed 0-19 는 20/20 성공 (seed 8 은 무행동 성공) | week0 `harness_check.md` §상한 대조 결과 |

## 미완 항목 — 현재 덤프는 expert 궤적이 아니다

현재 `outputs/expert_traj.npz` 는 실습 1 코드의 자리표시(`env.action_space.sample()`)가 만든 **random action 궤적**이다. 근거:

| 증거 | 값 | 해석 |
|---|---|---|
| `expert_traj_meta.json` 의 `control_mode` | `pd_joint_pos` | expert 의 확정 모드(`pd_ee_delta_pose`)와 다르다 — 코드의 `EXPERT_CONTROL_MODE` 가 자리표시 값 그대로다 |
| `expert_traj.npz` 의 `actions` shape | `(50, 8)` | `pd_joint_pos` 의 8차원(관절 7 + gripper 1). expert 라면 `pd_ee_delta_pose` 의 7차원이어야 한다 |
| `expert_traj_meta.json` 의 `success` | `false`, `steps=50` (truncation) | random action 이라 당연한 결과. expert 성공 여부의 근거가 될 수 없다 |

실습 1 을 닫으려면:

- [ ] week0 `harness_check.py` 의 scripted 정책을 expert 로 호출하도록 action 자리표시(`env.action_space.sample()`)를 교체
- [ ] seed 100 으로 재덤프 후 이 문서의 "expert 성공 여부" 행을 실측값으로 채운다
- [x] `EXPERT_CONTROL_MODE = "pd_ee_delta_pose"` — 실습 1/4 코드와 노트북에 반영됨
- [x] step 예산 — week0 검증 조건과 동일한 `max_episode_steps=200` 으로 확정, 실습 1/3/4 코드와 노트북에 반영됨 (루프 상한 `STEP_CAP=250` 은 환경 상한보다 크게 두어 truncation 출처를 구분)
- [x] 관측 카메라 해상도 — week0 baseline·week2 빌더 명세와 동일한 224x224 수집으로 확정 (`sensor_configs`), 실습 1/4 코드와 노트북에 반영됨. 기존 `expert_traj.npz` 는 128 수집본이므로 재덤프 대상에 포함

## 실습 2 에 넘기는 함의

- expert 가 이미 EEF 공간(`pd_ee_delta_pose`)으로 움직이므로, "관절 공간이라 action 을 직접 못 쓴다"는 실습 1 본문의 우려 시나리오에는 해당하지 않는다. 다만 expert 의 action 은 `[-1, 1]` **정규화 값**이고 학습 라벨은 **원시 물리 단위**여야 하므로, TCP pose 차분으로 라벨을 만드는 경로는 여전히 필요하다
- pose 의 회전은 쿼터니언(wxyz) 4개로 저장된다. 실습 2 의 상대 회전 계산(`inverse(q_t) * q_{t+1}`) 입력이 이 형식이다
