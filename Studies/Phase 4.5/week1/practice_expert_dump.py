import json
import numpy as np                          
import gymnasium as gym
import mani_skill.envs
from PIL import Image                     


ENV_ID = "PickCube-v1"                        
STEP_CAP = 100                                
DUMP_SEED = 100 
EXPERT_CONTROL_MODE = "pd_joint_pos"         

env = gym.make(ENV_ID, obs_mode="rgb", control_mode=EXPERT_CONTROL_MODE, sim_backend="physx_cpu")
obs, info = env.reset(seed=DUMP_SEED)
print("control mode:", env.unwrapped.control_mode)

frames = []
actions = []
tcp_poses = []
success = False


for step in range(STEP_CAP):
    # 카메라 렌더링은 sim_backend와 무관하게 GPU에서 수행되므로 CUDA 텐서를 CPU로 내린 뒤 numpy로 변환
    frame = obs["sensor_data"]["base_camera"]["rgb"].cpu().numpy()
    if frame.ndim == 4:
        frame = frame[0]
    frames.append(frame.astype(np.uint8))

    tcp_poses.append(np.asarray(env.unwrapped.agent.tcp.pose.raw_pose).reshape(-1))

    action = env.action_space.sample()
    actions.append(np.asarray(action).reshape(-1))

    obs, reward, terminated, truncated, info = env.step(action)
    if info.get("success", False):
        success = True
        break
    if terminated or truncated:
        break

env.close()
print(f"수집 스텝: {len(actions)}, success: {success}")

np.savez("outputs/expert_traj.npz",
         frames=np.stack(frames),
         actions=np.stack(actions),
         tcp_poses=np.stack(tcp_poses))
Image.fromarray(frames[0]).save("outputs/expert_traj_first.png")

with open("outputs/expert_traj_meta.json", "w") as f:
    json.dump({"env_id": ENV_ID, "seed": DUMP_SEED, "steps": len(actions), "control_mode": EXPERT_CONTROL_MODE, "success": bool(success)}, f, indent=2)
print("저장 완료: outputs/expert_traj.npz")