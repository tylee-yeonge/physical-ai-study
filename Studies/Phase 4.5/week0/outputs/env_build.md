# week0 sim 환경 구축 기록

> 확인일: 2026-07-29
> 용도: `Measurements/.../environment.md` 의 원본, Section 0 후반 Docker 이미지 명세의 근거

## 확정된 스택

| 항목 | 값 | 확인 방법 |
|---|---|---|
| venv | `Studies/Phase 4.5/.venv-sim` (Phase 4 의 `.venv-vla` 와 분리) | - |
| python | 3.12.3 | `platform.python_version()` |
| mani_skill | 3.0.1 | `mani_skill.__version__` |
| torch | 2.13.0+cu130 (CUDA 13.0) | `torch.__version__` |
| GPU | RTX 4070 | `torch.cuda.get_device_name(0)` |
| 다운로드 에셋 | 없음 | `~/.maniskill/data` 가 존재하지 않는 상태로 PickCube-v1 이 정상 생성됨 |

PickCube-v1 은 primitive shape (큐브 + 테이블) 로만 구성되어 별도 에셋 다운로드가 필요 없다. 자작 팔이나 YCB 객체를 쓰는 task 로 확장할 때는 에셋 확보 단계가 다시 필요하다.

## 관측 해상도 결정

**관측 카메라를 224x224 로 지정한다.** 환경 생성 시 `sensor_configs=dict(width=224, height=224)`.

근거:

- OpenVLA 의 vision encoder (SigLIP + DINOv2) 가 224x224 로 학습됐다. sim 에서 native 224 로 렌더하면 업샘플 열화 없이 학습 분포에 가장 가까운 입력이 된다
- ManiSkill 기본값 128x128 을 유지하면 추론 직전 128 -> 224 업샘플이 필요하고, 그 열화가 zero-shot 성공률에 섞여 들어가 "모델의 한계"와 "입력 화질의 한계"를 분리할 수 없게 된다
- 128 을 유지할 이유는 렌더 비용뿐이다. zero-shot OpenVLA on PickCube 조합에는 128 기준 공개 수치가 없어 비교 가능성을 잃지 않는다. 실습 5 의 upper bound 는 좌표를 직접 읽는 state 기반 scripted 정책이라 해상도와 무관하다

이 값은 baseline 정의의 일부다. 실습 6 의 성공률을 다른 수치와 비교할 때 해상도를 함께 명시해야 한다.

## 관측 / action 구조 (PickCube-v1, obs_mode="rgb", control_mode="pd_ee_delta_pose")

| 경로 | shape | 비고 |
|---|---|---|
| `sensor_data/base_camera/rgb` | (1, 224, 224, 3) | 모델 입력. 실습 6 에서 이 키를 그대로 쓴다 |
| `agent/qpos`, `agent/qvel` | (1, 9) | Panda 7 축 + gripper 2 |
| `extra/tcp_pose` | (1, 7) | position 3 + quaternion 4 |
| `extra/goal_pos` | (1, 3) | 목표 위치 |
| `extra/is_grasped` | (1) | 파지 여부 |
| `sensor_param/base_camera/{intrinsic_cv, extrinsic_cv, cam2world_gl}` | (1,3,3) / (1,3,4) / (1,4,4) | 카메라 파라미터 |

`env.render()` 는 (512, 512, 3) 으로, 사람이 보는 뷰다. 모델 입력과 무관하다.

성공 판정은 `info["success"]`. 부분 신호로 `is_obj_placed`, `is_robot_static`, `is_grasped` 가 함께 실린다 — 실습 6 의 부분 도달률 계산에 쓴다.

## 겪은 증상과 대응

| 증상 | 원인 | 대응 |
|---|---|---|
| `sapien/_vulkan_tricks.py` 의 `Failed to find Vulkan ICD file` UserWarning | 컨테이너에 NVIDIA Vulkan ICD 파일이 없어 SAPIEN 이 자체 ICD 로 폴백 | 무시. 렌더가 실제로 동작함을 `outputs/env_check.png` 의 장면(팔 + 테이블 + 큐브 + goal 마커)으로 확인. 검은 화면이면 폴백이 실패한 것이므로 그때 ICD 설치 필요 |
| `TypeError: can't convert cuda:0 device type tensor to numpy` | ManiSkill 은 GPU 시뮬레이션이라 `env.render()` 가 `cuda:0` 위의 torch tensor 를 반환. `np.asarray()` 가 내부에서 `.numpy()` 를 호출해 실패 | `env.render().cpu().numpy()` 로 호스트 메모리에 먼저 내린다 |
| `FileNotFoundError: 'outputs/env_check.png'` | 스크립트를 `outputs/` 안에서 실행해 저장 경로가 `outputs/outputs/` 로 해석됨 | 실습 스크립트는 `week0/` 루트에 두고 cwd 를 `week0` 으로 두고 실행한다. `outputs/` 는 산출물 전용 |
