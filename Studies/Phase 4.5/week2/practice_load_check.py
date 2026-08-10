import numpy as np
from prismatic.vla.datasets import RLDSDataset
from pathlib import Path

# ~/openvla/vla-scripts/finetune.py
#vla_dataset = RLDSDataset(
#        cfg.data_root_dir,
#        cfg.dataset_name,
#        batch_transform,
#        resize_resolution=tuple(vla.module.config.image_sizes),
#        shuffle_buffer_size=cfg.shuffle_buffer_size,
#        image_aug=cfg.image_aug,
#    )

data_root_dir = Path("~/tensorflow_datasets").expanduser()
data_mix = "maniskill_pickcube_only"


dataset = RLDSDataset(
    data_root_dir,
    data_mix,
    lambda batch: batch,
    resize_resolution=(224, 224),
    shuffle_buffer_size=1000,
)

# dataset = None
batch = next(iter(dataset))
print("배치 키:", list(batch.keys()))

def show(d, prefix=""):
    for k, v in d.items():
        if isinstance(v, dict):
            show(v, prefix + k + ".")
        else:
            a = np.asarray(v)
            print(f"{prefix}{k}: shape={a.shape} dtype={a.dtype}")
show(batch)

print("검사")
# image = np.asarray()
image = np.asarray(batch["observation"]["image_primary"])
print(f"imgae shape={image.shape} dtype={image.dtype}")

action = np.asarray(batch["action"])
print(f"action shape={action.shape}")
assert action.shape[-1] == 7, "action이 7차원이 아니다."

flat = action.reshape(-1, 7)
for dim in range(6):
    column = flat[:, dim]
    print(f"dim{dim}: min={column.min():+.3f} max={column.max():+.3f}")

gripper = flat[:, 6]
print(f"dim6(gripper): unique 근사값 {np.unique(np.round(gripper, 2))[:6]}")

print("instruction:", batch.get("task", {}).get("language_instruction", "<키 확인 필요>"))