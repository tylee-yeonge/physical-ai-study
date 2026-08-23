import numpy as np
import gymnasium as gym
import mani_skill.envs

ENV_ID = "PickCube-v1"
MAX_EPISODE_STEPS = 200
DUMP_SEED = 100
STRONG_TOL = 1e-3

openvla_actions = np.load("outputs/openvla_actions.npy")
original = np.load("outputs/expert_traj.npz")["tcp_poses"]

env = gym.make(ENV_ID, obs_mode="rgb", control_mode="pd_ee_delta_pose", max_episode_steps=MAX_EPISODE_STEPS, sim_backend="physx_cpu")
obs, info = env.reset(seed=DUMP_SEED)
replay_poses = []
success = False

for action_7d in openvla_actions:
    replay_poses.append(np.asarray(env.unwrapped.agent.tcp.pose.raw_pose).reshape(-1))
    action = action_7d
    obs, reward, terminated, truncated, info = env.step(action)
    if info.get("success", False):
        success = True
        break
    if terminated or truncated:
        break

env.close()
replay_poses = np.stack(replay_poses)

compare_len = min(len(replay_poses), len(original))
position_error = np.linalg.norm(
    replay_poses[:compare_len, 0:3] - original[:compare_len, 0:3], axis=1
)
print(f"\n비교 스탭 수: {compare_len} (재생 {len(replay_poses)} / 원본 {len(original)})")
print(f"위치 오차 mean={position_error.mean():.6f} max={position_error.max():.6f} m")
print(f"강한 기준(max < {STRONG_TOL}): {'통과' if position_error.max() < STRONG_TOL else '실패'}")

print(f"약한 기준(success): {'통과' if success else '실패'}")