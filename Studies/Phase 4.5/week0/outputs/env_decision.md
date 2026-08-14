# 공용 venv 통합 판단 (실습 1-6)

> 작성일: 2026-07-29
> 대상: `Studies/Phase 4/.venv-vla` (OpenVLA 공용) 와 ManiSkill 의 동거 가능 여부
> 결론이 정하는 것: 실습 6 을 단일 프로세스로 쓸 수 있는지, 아니면 sim 과 추론을 두 프로세스로 나눠야 하는지

## 판단

**합친다.** 실습 6 (zero-shot baseline) 은 `.venv-vla` 한 프로세스에서 sim 과 추론을 함께 돌린다. 파일·소켓으로 주고받는 2 프로세스 구성은 필요 없다.

`.venv-sim` 은 폐기하지 않고 sim 단독 실습(2, 3, 5)용으로 유지한다. `.venv-vla` 에 렌더 문제가 생겼을 때 sim 자체의 정상성을 대조할 기준이 된다.

## 근거

### 1. ManiSkill 은 이미 `.venv-vla` 에 설치되어 있다

`pip install --dry-run mani_skill` 의 출력 전체가 `Requirement already satisfied` 다 — `mani_skill 3.0.1`, `sapien 3.0.3`, `pytorch-kinematics 0.7.6` 를 포함해 새로 받을 것이 하나도 없다. 즉 통합 여부는 "설치하면 어떻게 되는가"의 문제가 아니라 **이미 설치된 상태가 기록된 측정 환경을 훼손했는지**의 사후 대조 문제다.

### 2. 측정 기록의 고정 버전이 전부 그대로다

`Measurements/openvla-rtx4070-int4/environment.md` 의 기록값과 현재 `.venv-vla` 를 대조했다.

| 항목 | 기록값 | 현재 `.venv-vla` | 일치 |
|---|---|---|---|
| Python | 3.12.3 | 3.12.3 | 예 |
| PyTorch | 2.12.0 | 2.12.0+cu130 (`torch.version.cuda` 13.0, `cuda.is_available()` True) | 예 |
| transformers | 4.40.1 | 4.40.1 | 예 |
| tokenizers | 0.19.1 | 0.19.1 | 예 |
| timm | 0.9.16 | 0.9.16 | 예 |
| accelerate | 1.0.1 | 1.0.1 | 예 |
| bitsandbytes | 0.49.2 | 0.49.2 | 예 |

`Studies/Phase 4/week6/requirements.txt` 가 고정을 요구하는 4개 항목(transformers, tokenizers, timm, accelerate)이 모두 보존되어 있다. ManiSkill 의 요구가 `torch` (버전 하한 없음) 였고 기존 2.12.0 이 이를 만족했기 때문에 torch 교체가 일어나지 않았다.

### 3. 두 스택이 한 인터프리터에서 동시에 import 된다

`.venv-vla` 에서 확인한 결과:

| 확인 항목 | 결과 |
|---|---|
| `import mani_skill.envs` 후 `gymnasium.registry` 의 PickCube 항목 수 | 4 |
| `mani_skill` / `sapien` | 3.0.1 / 3.0.3 |
| `from transformers import AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig` | 성공 |
| import 이후 `transformers.__version__` | 4.40.1 (변동 없음) |
| `pip check` | `No broken requirements found.` |
| numpy / gymnasium | 2.5.0 / 1.3.0 |

재현 명령:

```bash
source "/workspace/study/physical-ai-study/Studies/Phase 4/.venv-vla/bin/activate"
python -c "import gymnasium, mani_skill.envs, transformers; \
print(sum(1 for k in gymnasium.registry if 'PickCube' in k), transformers.__version__)"
pip check
```

### 4. 이 판단의 근거가 되지 못한 것

`pip install --dry-run mani_skill 2>&1 | grep -i "would install"` 의 빈 출력은 근거로 쓸 수 없다. pip 은 이미 설치된 패키지에 대해 `Would install` 행을 내지 않으므로, 빈 출력은 "충돌 없음"과 "이미 설치됨"을 구분하지 못한다. 위 §2-§3 의 직접 대조가 실제 근거다.

## 이 판단이 보장하지 않는 것

- **numpy 는 대조 불가.** `environment.md` 에 numpy 버전 기록이 없어 ManiSkill 설치 전후를 비교할 수 없다 (현재 2.5.0). Block 1-3 수치의 재현성은 재측정으로만 확인된다.
- **import 성공은 렌더 성공이 아니다.** `.venv-vla` 에서 Vulkan 렌더 컨텍스트가 실제로 붙는지는 확인하지 않았다. 실습 2 를 `.venv-vla` 로 한 번 더 통과시켜야 한다.
- **두 venv 의 torch 가 다르다.** `.venv-sim` 은 2.13.0+cu130, `.venv-vla` 는 2.12.0+cu130 이다. 신규 설치 쪽이 최신으로 해석된 결과이며, 같은 sim 코드가 두 venv 에서 다르게 거동하면 이 차이를 첫 용의자로 본다.
- **Vulkan ICD 조치가 `.venv-vla` 에도 필요하다.** `sapien` import 시점의 ICD 탐색 실패는 venv 와 무관하게 재현되므로, `env_build.md` 에 기록한 `VK_ICD_FILENAMES` 설정을 이 venv 에서도 적용한 상태로 실습 6 을 돌린다.
