import glob
import json
import numpy as np
import gymnasium as gym
import mani_skill.envs

from expert_policy import ScriptedExpert   # 실습 1 과 같은 정책을 부른다

ENV_ID = "PickCube-v1"
MAX_EPISODE_STEPS = 200
STEP_CAP = 250
EXPERT_CONTROL_MODE = "pd_ee_delta_pose"
N_EPISODES = 100
SMOKE_EPISODES = 3                              # 앞 N episode 로 expert 호출부를 먼저 검사한다
SUCCESS_FLOOR = 0.8                             # 이 아래면 수집물을 신뢰하지 않는다 (week0 상한 20/20)

# 이전 수집물이 남아 있으면 실습 6 의 glob 이 옛 episode 까지 함께 센다
stale = glob.glob("outputs/dataset/ep*.npz")
assert not stale, f"이전 수집물 {len(stale)}개가 남아 있다 -- rm outputs/dataset/ep*.npz 후 다시 실행"

with open("../week0/outputs/zeroshot_baseline.json") as f:
    eval_seeds = set(json.load(f)["seeds"])
print(f"eval seed {len(eval_seeds)}개 예약됨: {sorted(eval_seeds)}")

train_seeds = [seed for seed in range(1000, 1000 + N_EPISODES * 2)
               if seed not in eval_seeds][:N_EPISODES]
assert not (set(train_seeds) & eval_seeds), "학습 seed 에 eval seed 가 섞였다"
print(f"학습 seed {len(train_seeds)}개, 겹침 0 확인")

env = gym.make(ENV_ID, obs_mode="rgb", control_mode=EXPERT_CONTROL_MODE,
           max_episode_steps=MAX_EPISODE_STEPS, sim_backend="physx_cpu",
           sensor_configs=dict(width=224, height=224))
expert = ScriptedExpert(env)
episode_index = 0                               # 저장 파일 번호
success_count = 0                               # expert 성공률 통계용

for episode_ordinal, seed in enumerate(train_seeds):
    obs, info = env.reset(seed=seed)
    expert.reset()                              # 단계 상태를 되돌린다 -- 빼면 앞 episode 의 phase 를 물고 간다
    frames, actions, tcp_poses = [], [], []
    success = False
    for step in range(STEP_CAP):
        frame = obs["sensor_data"]["base_camera"]["rgb"].cpu().numpy()
        if frame.ndim == 4:
            frame = frame[0]
        frames.append(frame.astype(np.uint8))
        tcp_poses.append(np.asarray(env.unwrapped.agent.tcp.pose.raw_pose).reshape(-1))
        action = expert.act()
        actions.append(np.asarray(action).reshape(-1))
        obs, reward, terminated, truncated, info = env.step(action)
        if info.get("success", False):
            success = True
            break
        if terminated or truncated:
            break

    success_count += int(success)
    # 실패 episode 는 학습에 넣지 않되 통계에는 남긴다
    if success:
        np.savez(f"outputs/dataset/ep{episode_index:04d}.npz",
                 frames=np.stack(frames),
                 actions=np.stack(actions),
                 tcp_poses=np.stack(tcp_poses),
                 seed=seed)                     # seed 를 데이터에 박아 둔다 (재현·검증용)
        episode_index += 1
    print(f"   seed{seed}: success={success} steps={len(actions)}")

    # 조기 차단: 100 episode 를 다 돌린 뒤에 알아차리면 수집 시간을 통째로 버린다
    if episode_ordinal + 1 == SMOKE_EPISODES:
        assert success_count == SMOKE_EPISODES, \
            f"앞 {SMOKE_EPISODES} episode 중 {success_count}개만 성공 -- expert 호출부를 확인한다"

env.close()

# 성공률이 바닥을 밑돌면 데이터가 아니라 잡음을 모은 것이다.
# 메타를 먼저 쓰면 잘못된 수집이 정상 산출물처럼 남으므로 저장 전에 판정한다.
success_rate = success_count / len(train_seeds)
assert success_rate >= SUCCESS_FLOOR, \
    f"expert 성공률 {success_rate:.2f} < {SUCCESS_FLOOR} -- 데이터로 쓸 수 없다"

with open("outputs/dataset/collect_meta.json", "w") as f:
    json.dump({"env_id": ENV_ID, "control_mode": EXPERT_CONTROL_MODE,
               "max_episode_steps": MAX_EPISODE_STEPS,
               "step_cap": STEP_CAP, "train_seeds": train_seeds,
               "eval_seeds_excluded": sorted(eval_seeds),
               "episodes_saved": episode_index,
               "expert_success_rate": success_rate,
               "instruction": "pick up the cube"}, f, indent=2)
print(f"\n저장 episode: {episode_index} / expert 성공률: {success_count}/{len(train_seeds)}")