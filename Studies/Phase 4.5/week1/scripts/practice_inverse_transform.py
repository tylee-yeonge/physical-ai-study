import numpy as np
data = np.load("outputs/expert_traj.npz")
tcp_poses = data["tcp_poses"]
expert_actions = data["actions"]

print("tcp_poses shape:", tcp_poses.shape)
print("첫 pose:", tcp_poses[0])

positions = tcp_poses[:, 0:3]
delta_position = np.diff(positions, axis=0)
delta_rotation = np.zeros_like(delta_position)
gripper = np.zeros((len(delta_position), 1))

SCALE_POSITION = 1.0
SCALE_ROTATION = 1.0

openvla_actions = np.concatenate([
    delta_position * SCALE_POSITION,
    delta_rotation * SCALE_ROTATION,
    gripper,
], axis=1)
print("openvla_actions shape:", openvla_actions.shape)
print("차원 별 범위")
for dim in range(openvla_actions.shape[1]):
    column = openvla_actions[:, dim]
    print(f"dim{dim}: min={column.min():+.4f} max={column.max():+.4f}")

np.save("outputs/openvla_actions.npy", openvla_actions)
print("저장 완료: outputs/openvla_actions.npy")