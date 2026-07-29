# Week 0 실습: sim 신규 구축 -> 하네스 검증 -> zero-shot baseline


> **실습 목표**: ManiSkill sim 환경을 0부터 구축하고, action 변환 계약을 확정하고, 하네스를 검증한 뒤 zero-shot baseline 을 측정한다.
> **예상 시간**: 12-16시간
> **원칙**: 실습 6 (측정) 는 실습 5 (검증) 를 통과한 뒤에만 한다. 순서를 바꾸면 측정값이 해석 불가가 된다 (README §1).


---


## 이번 주에 v1 에서 가져오는 것과 새로 만드는 것


sim 은 **새로 구축한다.** v1 (Phase 4) 에는 실행 가능한 sim 환경이 없다 — 남아 있는 것은 문서상의 선정 판단뿐이고, 그것으로는 한 episode 도 돌 수 없다.


| 구분 | 내용 | 출처 |
|---|---|---|
| 재사용 (문서) | sim 선정 근거 (ManiSkill), 후보 비교표, 비채택 사유 | [`Phase 4/notes.md`](../../Phase%204/notes.md) 순서 3 |
| 재사용 (문서) | task 선정 (PickCube), instruction 문구, N=20 기준 | 같은 문서의 성공 task 정의 노트 |
| 재사용 (코드) | OpenVLA 4-bit 추론 호출 패턴 | [`Phase 4/week6`](../../Phase%204/week6/PRACTICE.md) |
| **신규 구축** | Vulkan 런타임, ManiSkill 설치, 에셋, venv | 실습 1 |
| **신규 구축** | 환경 생성·관측 추출·성공 판정 확인 | 실습 2-3 |
| **신규 구축** | action 변환 레이어 | 실습 4 |
| **신규 구축** | 하네스 검증, baseline 측정 | 실습 5-6 |


> 이번 주 산출물이 이후 week 전체의 sim 기반이 된다. week1 의 데이터 수집, week5 의 eval 이 모두 여기서 세운 환경과 계약 표를 그대로 쓴다.


---


## 실습 1: sim 환경 신규 구축 (설치 -> 검증)


렌더가 되지 않으면 이후 전부가 무의미하다. 이 실습의 통과 조건은 **ManiSkill 내장 데모가 헤드리스로 완주하는 것**이다.


### 1-1. Vulkan 런타임 확보


SAPIEN (ManiSkill 의 물리·렌더 엔진) 은 OpenGL 이 아니라 **Vulkan** 으로 렌더한다. 여기가 설치 실패의 1순위다.


```bash
sudo apt update
sudo apt install -y libvulkan1 vulkan-tools    # Vulkan 로더 + 진단 도구 (vulkaninfo)
vulkaninfo --summary                           # GPU 가 목록에 나와야 한다
ls /usr/share/vulkan/icd.d/                    # nvidia ICD 파일 존재 확인 (드라이버가 설치한다)
```


`vulkaninfo --summary` 에 RTX 4070 이 나오지 않으면 여기서 멈춘다. GPU 대신 `llvmpipe` 만 보이면 CPU 소프트웨어 렌더로 떨어진 상태이고, 그대로 진행하면 느리거나 검은 화면이 나온다.


### 1-2. sim 전용 venv 생성


Phase 4 의 공용 venv `.venv-vla` 를 건드리지 않는다. `transformers==4.40.1` / `timm==0.9.16` 고정 조합 위에서 Block 1-3 실측이 재현되는데, ManiSkill 설치가 torch 를 갈아 버리면 그 재현성이 깨진다.


```bash
cd "/workspace/study/physical-ai-study/Studies/Phase 4.5"
python3 -m venv .venv-sim                      # sim 전용 (.venv* 는 gitignore 대상)
source .venv-sim/bin/activate
pip install --upgrade pip                      # 휠 해석 실패를 줄인다
mkdir -p week0/outputs                         # 실습 산출물 저장 위치 (outputs/ 는 gitignore)
pip install -r week0/requirements.txt          # ManiSkill 본체
```


`sapien` 휠을 못 찾는다는 오류가 나면 **현재 python 버전에 맞는 휠이 없는 경우**다. 그때는 지원 버전으로 venv 를 다시 만든다 (예: `python3.11 -m venv .venv-sim`). 해당 버전이 시스템에 없으면 apt 로 먼저 설치해야 하므로, 이 분기에 들어갔다는 사실과 최종 사용 버전을 기록한다.


### 1-3. 에셋 확보


ManiSkill 은 로봇·씬 에셋을 패키지와 별도로 내려받는 경우가 있다. 필요 여부는 task 마다 다르므로 **명령의 도움말로 확인하고** 필요한 것만 받는다.


```bash
python -m mani_skill.utils.download_asset --help    # 받을 수 있는 에셋 목록·사용법 확인
python -m mani_skill.utils.download_demo --help     # 데모 궤적 데이터 (있으면 실습 5 후보)
```


PickCube 는 기본 도형 위주라 추가 에셋 없이 도는 것이 정상이다. 실습 2 에서 에셋 관련 오류가 나면 그때 여기로 돌아와 해당 에셋만 받는다.


### 1-4. 설치 검증 — 내장 데모 완주


직접 코드를 쓰기 전에 패키지가 제공하는 데모로 검증한다. 내 코드의 버그와 설치 문제를 분리하기 위한 단계다.


```bash
python -c "import mani_skill; print(mani_skill.__version__, mani_skill.__path__[0])"
# 사용 가능한 예제 모듈 목록 확인 (이름은 버전마다 다르므로 직접 본다)
ls "$(python -c 'import mani_skill; print(mani_skill.__path__[0])')/examples"
```


목록에서 random action 데모를 찾아 **헤드리스로** 실행한다. GUI 모드로 실행하면 디스플레이가 없어 실패하므로, 렌더 모드를 배열 출력으로 지정한다.


```bash
python -m mani_skill.examples.demo_random_action -e PickCube-v1 --render-mode rgb_array
# 옵션 이름이 다르면: python -m mani_skill.examples.demo_random_action --help 로 확인
```


### 1-5. 자주 나오는 증상과 대응


| 증상 | 원인 | 대응 |
|---|---|---|
| pip 가 `sapien` 휠을 못 찾음 | 현재 python 버전용 휠 미제공 | 지원 버전으로 venv 재생성 (1-2) |
| `vulkaninfo` 에 GPU 없음 / `llvmpipe` 만 | Vulkan 로더·ICD 미설치 | `libvulkan1` 설치, ICD 경로 확인 (1-1) |
| 렌더 결과가 전부 검정 | GPU 렌더 컨텍스트 실패 후 fallback | ICD·드라이버 확인. 검은 화면은 통과가 아니다 |
| Vulkan instance/device 생성 실패 | 컨테이너에 GPU 디바이스 미노출 | GPU 패스스루 설정 확인 |
| 에셋 파일 없음 오류 | 에셋 미다운로드 | 해당 에셋만 받는다 (1-3) |
| 데모가 디스플레이 오류로 죽음 | GUI 렌더 모드로 실행 | 렌더 모드를 배열 출력으로 지정 (1-4) |


**기록할 것** (`outputs/env_build.md`): 최종 python 버전, `mani_skill` 버전, 받은 에셋, 겪은 증상과 대응. 이 파일이 나중에 `Measurements/.../environment.md` 의 원본이 되고, Section 0 후반의 Docker 이미지 명세가 된다.


### 1-6. 공용 venv 와의 통합 가능성 조사


실습 6 에서 OpenVLA 와 ManiSkill 을 한 프로세스에서 써야 한다. 합칠 수 있는지 지금 알아 둔다.


```bash
deactivate
source "../Phase 4/.venv-vla/bin/activate"
pip install --dry-run mani_skill 2>&1 | grep -i "would install"   # 실제 설치 없이 영향만 본다
```


출력에 `torch` / `transformers` / `timm` 이 **다른 버전으로** 등장하면 합치지 않는다. 그 경우 실습 6 은 두 프로세스 (sim + 추론) 로 나눠 파일이나 소켓으로 주고받는 구조가 된다. 등장하지 않으면 `.venv-vla` 에 ManiSkill 을 추가해 한 프로세스로 진행한다. 판단과 근거를 `outputs/env_decision.md` 에 두 줄로 남긴다.


---


## 실습 2: 첫 환경 생성 + 관측 구조 확인


**파일명**: `practice_env_check.py`


```python
"""
실습 2: 환경을 직접 생성해 관측 구조를 확인하고 렌더 이미지 1장을 저장
"""
import gymnasium as gym                        # ManiSkill 은 gymnasium 규약을 따른다
import mani_skill.envs                         # import 만으로 ManiSkill task 들이 gym 에 등록된다
import numpy as np                             # 관측 배열 처리
from PIL import Image                          # 렌더 결과 저장


print("=" * 60)
print("실습 2: 환경 생성 + 관측 구조")
print("=" * 60)


# -- 2-1. 등록된 task 이름 확인 (문서를 믿지 않고 설치된 버전에서 직접 본다) --
print("\n[2-1] PickCube 관련 등록 환경")
for env_id in gym.registry.keys():             # gym 에 등록된 모든 환경 이름을 훑는다
    if "PickCube" in env_id:                   # 이름에 PickCube 가 포함된 것만
        print("  ", env_id)                    # 실제 사용할 정확한 id 를 여기서 얻는다


# -- 2-2. 환경 생성 (ENV_ID 를 2-1 출력의 실제 이름으로 교체) --
ENV_ID = "PickCube-v1"                         # <- 2-1 출력의 실제 id 로 교체한다
print(f"\n[2-2] 환경 생성: {ENV_ID}")
env = gym.make(
    ENV_ID,                                    # 사용할 task
    obs_mode="rgb",                            # 관측에 카메라 RGB 이미지를 포함시킨다
    control_mode="pd_ee_delta_pose",           # action 을 EEF 델타 pose 로 해석 (OpenVLA 출력과 대응)
    render_mode="rgb_array",                   # GUI 없이 배열로 렌더 (헤드리스)
)
print("환경 생성 성공 — Vulkan 렌더 컨텍스트가 붙었다")


# -- 2-3. 관측 구조 확인 (키 이름을 추측하지 않고 출력해서 본다) --
print("\n[2-3] 관측 구조")
obs, info = env.reset(seed=0)                  # seed 를 주면 초기 배치가 재현된다
for key in obs:                                # 관측 최상위 키
    value = obs[key]
    if isinstance(value, dict):                # 한 겹 더 들어가는 항목 (카메라 등)
        for sub_key in value:
            inner = value[sub_key]
            if isinstance(inner, dict):        # 카메라별로 한 겹 더 있는 경우
                for leaf in inner:
                    print(f"   {key}/{sub_key}/{leaf}: {getattr(inner[leaf], 'shape', '')}")
            else:
                print(f"   {key}/{sub_key}: {getattr(inner, 'shape', '')}")
    else:
        print(f"   {key}: {getattr(value, 'shape', type(value).__name__)}")
print("\n[2-3b] info 키:", list(info.keys()))  # 성공 플래그가 어디 실리는지 단서


# -- 2-4. 렌더 이미지 저장 (여기가 통과 판정) --
print("\n[2-4] 렌더 이미지 저장")
frame = np.asarray(env.render())               # 현재 장면을 배열로 렌더 (tensor 로 올 수 있어 numpy 통일)
if frame.ndim == 4:                            # (batch, H, W, 3) 형태면 첫 장만 쓴다
    frame = frame[0]
print("frame shape:", frame.shape)             # 해상도 확인 — OpenVLA 는 224x224 를 먹는다
Image.fromarray(frame.astype(np.uint8)).save("outputs/env_check.png")
print("저장 완료: outputs/env_check.png")


env.close()                                    # 렌더 컨텍스트 해제
print("\n실습 2 완료")
```


**확인 포인트**

- `outputs/env_check.png` 가 검은 화면이 아니라 실제 장면인가. 검으면 실습 1-1 로 복귀
- 렌더 해상도가 224x224 인가. 아니라면 실습 6 에서 리사이즈할지, 환경 생성 시 카메라 해상도를 지정할지 **여기서 정해 기록**한다 (한 번 정하면 baseline 정의의 일부다)
- 카메라 이미지의 정확한 키 경로 (실습 6 에서 그대로 쓴다)


---


## 실습 3: sim 단독 루프 + success 정의 확인


**파일명**: `practice_sim_loop.py`


VLA 없이 random action 으로 돌린다. 목적은 "돌아간다"가 아니라 **성공 판정과 action 공간의 정의를 사실로 확정**하는 것이다.


```python
"""
실습 3: random action 으로 episode 를 돌리고 success 정의·action 공간을 확정
"""
import inspect                                 # 성공 판정 코드를 직접 읽기 위해
import gymnasium as gym
import mani_skill.envs


ENV_ID = "PickCube-v1"                         # <- 실습 2 에서 확정한 id


print("=" * 60)
print("실습 3: sim 단독 루프")
print("=" * 60)


env = gym.make(ENV_ID, obs_mode="rgb", control_mode="pd_ee_delta_pose")


# -- 3-1. action 공간의 실제 정의 (범위 규약 확정 — 변환 계약 2번 항목) --
print("\n[3-1] action space")
print("  ", env.action_space)                  # 차원 수와 상한/하한이 그대로 출력된다
print("   low :", env.action_space.low)        # 하한 — -1 이면 정규화된 입력을 기대한다는 뜻
print("   high:", env.action_space.high)       # 상한


# -- 3-2. 선택 가능한 control_mode 목록 --
print("\n[3-2] control modes")
print("  ", env.unwrapped.agent.supported_control_modes)   # 실제 지원 목록 (문서와 다를 수 있다)


# -- 3-3. 성공 판정의 소스코드를 직접 읽는다 (임계값을 추측하지 않는다) --
print("\n[3-3] 성공 판정 코드")
print(inspect.getsource(type(env.unwrapped).evaluate))     # evaluate 가 success 를 계산한다


# -- 3-4. 상태 접근 경로 탐색 (부분 도달률 계산에 필요 — 실습 6) --
print("\n[3-4] env 내부 접근 가능한 이름")
print("  ", [name for name in dir(env.unwrapped) if not name.startswith("_")])


# -- 3-5. random action 으로 20 episode (step cap 확정) --
print("\n[3-5] random action 20 episode")
STEP_CAP = 100                                 # 잠정값. 아래 출력으로 타당성을 판정한다
for episode in range(20):                      # seed 를 바꿔 20회
    obs, info = env.reset(seed=episode)        # 매번 다른 초기 배치
    success = False                            # 이 episode 의 성공 여부
    for step in range(STEP_CAP):               # step cap 까지만 시도
        action = env.action_space.sample()     # 무작위 action (정책 없음)
        obs, reward, terminated, truncated, info = env.step(action)
        if info.get("success", False):         # 성공 플래그 — 키 이름은 3-3 에서 확인한 것으로
            success = True
            break                              # 성공 즉시 종료
        if terminated or truncated:            # 환경이 스스로 끝냈으면 종료
            break
    print(f"   ep{episode:02d}: success={success} steps={step + 1}")


env.close()
print("\n실습 3 완료")
```


**기록할 것** (`outputs/sim_facts.md`)

| 항목 | 확정값 | 출처 |
|---|---|---|
| 환경 id | | 실습 2 의 2-1 출력 |
| action 차원 / 범위 | | 3-1 출력 |
| success 의 정확한 정의 (임계값·축·정지 조건) | | 3-3 소스코드 |
| step cap | | 3-5 결과로 판단 |
| 부분 도달률 계산에 쓸 상태 접근 경로 | | 3-4 출력 |
| 카메라 키 경로 / 해상도 | | 실습 2 의 2-3 출력 |


> random action 은 성공하지 않는 것이 정상이다. 20/20 실패가 나와야 하며, 성공이 나오면 success 판정이 너무 느슨하므로 3-3 의 정의를 다시 읽는다.


---


## 실습 4: action 변환 계약 확정


**산출물**: `outputs/action_contract.md`


코드보다 조사가 많은 실습이다. README §4-§6 의 4항목을 **출처가 붙은 표**로 만든다. 답을 여기 적어 두지 않는다 — 이 표를 채우는 것이 이번 주의 이해 검증이다.


### 4-1. OpenVLA 쪽 사실 확인


`.venv-vla` 에서 실행한다 (모델 로드가 필요하다).


```python
"""
실습 4-1: OpenVLA 의 action 통계를 열어 단위·정규화 규약을 확인
"""
import torch
from transformers import AutoModelForVision2Seq, BitsAndBytesConfig


bnb_config = BitsAndBytesConfig(               # Phase 4 week6 과 동일한 4-bit 설정
    load_in_4bit=True,
    bnb_4bit_quant_type="nf4",
    bnb_4bit_use_double_quant=True,
    bnb_4bit_compute_dtype=torch.float16,
)
vla = AutoModelForVision2Seq.from_pretrained(  # 로드에 수 분 걸린다
    "openvla/openvla-7b",
    attn_implementation="eager",
    torch_dtype=torch.float16,
    low_cpu_mem_usage=True,
    trust_remote_code=True,
    quantization_config=bnb_config,
)


# -- 사용 가능한 unnorm_key 목록 --
print("\n[4-1a] 선택 가능한 unnorm_key")
print(list(vla.norm_stats.keys()))             # 학습 데이터셋별 통계 키 목록


# -- 특정 key 의 통계 내용 (여기서 단위·정규화 대상 차원을 읽는다) --
KEY = "bridge_orig"                            # <- 후보 key. 선택 근거를 표에 적을 것
print(f"\n[4-1b] {KEY} 통계")
for name, value in vla.norm_stats[KEY]["action"].items():
    print(f"   {name}: {value}")               # 분위수·평균·마스크 등이 그대로 나온다
```


읽어야 할 것: 어떤 통계량으로 되돌리는가 / 되돌린 값의 크기 대역이 어느 정도인가 (그것이 미터인지 판단하는 근거) / **차원별 적용 여부 마스크가 있는가**, 있다면 어느 차원이 제외되는가.


### 4-2. 계약 표 작성


아래 표를 `outputs/action_contract.md` 에 만들고 채운다. 각 행에 **출처**(파일:줄 또는 출력 스니펫) 를 반드시 붙인다.


| # | 항목 | OpenVLA 쪽 | ManiSkill 쪽 | 변환 규칙 | 출처 |
|---|---|---|---|---|---|
| 1 | 위치 델타의 단위·스케일 | | | | |
| 2 | 값의 범위 규약 (정규화 여부) | | | | |
| 3 | 기준 프레임 | | | | |
| 4 | 회전 표현 | | | | |
| 5 | gripper 부호 규약 | | | | |


> 이 변환 코드는 실행 보드 `#5` 의 `RobotPolicy` adapter + action schema validation 의 첫 sim 구현체다. 별도로 쓰지 말고 그 인터페이스 위에 얹는다 — 안 그러면 같은 코드를 두 번 쓴다.


---


## 실습 5: 하네스 검증 (상한 대조)


**파일명**: `practice_upper_bound.py`


기성 해법을 **같은 env·같은 step cap·같은 성공 판정**에 넣어 성공률 상한을 본다. 높게 나오면 env·루프·판정은 정상이고, 남는 용의자는 정책과 변환 레이어뿐이다 (README §8).


```python
"""
실습 5: 기성 해법으로 성공률 상한을 확인 (루프·환경·판정 검증)
"""
import mani_skill                              # 패키지 경로를 알기 위해


print("=" * 60)
print("실습 5: 상한 대조")
print("=" * 60)


# -- 5-1. 사용 가능한 기성 해법 탐색 (있는지부터 확인한다) --
print("\n[5-1] motion planning / scripted solution 탐색")
print("패키지 경로:", mani_skill.__path__[0])   # 이 아래에서 solution 스크립트를 찾는다
# 셸에서 실제 파일을 찾는다:
#   find "$(python -c 'import mani_skill; print(mani_skill.__path__[0])')" \
#        \( -iname "*motionplanning*" -o -iname "*solution*" \) -print


# -- 5-2. 찾은 해법을 같은 조건으로 20 episode 실행 --
# 조건 3개가 실습 3·6 과 반드시 같아야 한다.
#   (1) 같은 ENV_ID + 같은 obs_mode/control_mode
#   (2) 같은 STEP_CAP
#   (3) 같은 seed 목록
# 하나라도 다르면 상한 대조가 성립하지 않는다.
ENV_ID = "PickCube-v1"                         # <- 실습 2 확정값
STEP_CAP = 100                                 # <- 실습 3 확정값
SEEDS = list(range(20))                        # <- 실습 6 과 공유할 고정 목록


print("\n[5-2] 기성 해법 20 episode")
# 5-1 에서 찾은 해법의 호출 방식에 맞춰 아래 루프를 채운다.
# 해법이 자체 env 생성을 요구하면, 위 3개 조건을 인자로 강제해 맞춘다.
success_count = 0                              # 성공 episode 수
for seed in SEEDS:
    # solved = <찾은 해법으로 seed 의 episode 를 1회 수행하고 성공 여부를 받는다>
    solved = False                             # <- 위 한 줄을 구현해 교체한다
    success_count += int(solved)               # 성공이면 1 누적
    print(f"   seed{seed:02d}: {solved}")
print(f"\n상한 성공률: {success_count}/{len(SEEDS)}")
```


**통과 판정**

- 상한이 **충분히 높다** (기성 해법이 대부분 성공) -> env·루프·판정 정상. 실습 6 으로 진행
- 상한이 낮다 -> **실습 6 으로 가지 않는다.** step cap / control_mode / success 정의 중 무엇이 문제인지 실습 3 으로 복귀


> 기성 해법이 없거나 붙이기 어려우면 대안은 공개 수치 대조 (README §8 의 (b)) 다. 어느 쪽을 썼는지와 결과를 `outputs/harness_check.md` 에 남긴다 — 이 기록이 없으면 실습 6 의 숫자는 해석 불가 상태로 남는다.


---


## 실습 6: zero-shot baseline 측정


**파일명**: `practice_zeroshot_baseline.py`


실습 5 를 통과한 뒤에만 실행한다. 실습 4 의 계약 표대로 변환하고, N=20 을 고정 seed 로 돌려 **최종 성공률과 부분 도달률을 함께** 기록한다.


```python
"""
실습 6: OpenVLA zero-shot baseline (N=20, 고정 seed, 부분 도달률 병기)
"""
import json                                    # 결과를 원시 형태로 저장하기 위해
import numpy as np
import torch
import gymnasium as gym
import mani_skill.envs
from PIL import Image
from transformers import AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig


ENV_ID = "PickCube-v1"                         # <- 실습 2 확정값
STEP_CAP = 100                                 # <- 실습 3 확정값
SEEDS = list(range(20))                        # <- 실습 5 와 같은 목록 (변인 고정)
UNNORM_KEY = "bridge_orig"                     # <- 실습 4 에서 근거와 함께 확정한 key
INSTRUCTION = "pick up the cube"               # 영어 단문 고정 (OpenVLA prompt 틀)


print("=" * 60)
print("실습 6: zero-shot baseline")
print("=" * 60)


# -- 6-1. 모델 로드 (Phase 4 week6 과 동일 설정) --
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_quant_type="nf4",
    bnb_4bit_use_double_quant=True,
    bnb_4bit_compute_dtype=torch.float16,
)
processor = AutoProcessor.from_pretrained("openvla/openvla-7b", trust_remote_code=True)
vla = AutoModelForVision2Seq.from_pretrained(
    "openvla/openvla-7b",
    attn_implementation="eager",
    torch_dtype=torch.float16,
    low_cpu_mem_usage=True,
    trust_remote_code=True,
    quantization_config=bnb_config,
)
prompt = f"In: What action should the robot take to {INSTRUCTION}?\nOut:"


# -- 6-2. 환경 생성 (실습 3·5 와 동일 조건) --
env = gym.make(ENV_ID, obs_mode="rgb", control_mode="pd_ee_delta_pose")


# -- 6-3. episode 루프 --
records = []                                   # episode 별 결과를 모은다
for seed in SEEDS:
    obs, info = env.reset(seed=seed)           # 고정 seed 로 초기 배치 재현
    stages = {"reached": False, "grasped": False, "lifted": False, "placed": False}
    for step in range(STEP_CAP):
        # (a) 관측에서 카메라 이미지 추출 — 키 경로는 실습 2 의 2-3 출력대로
        frame = np.asarray(obs["sensor_data"]["base_camera"]["rgb"])   # <- 실제 경로로 교체
        if frame.ndim == 4:                    # 배치 차원이 있으면 첫 장만
            frame = frame[0]
        image = Image.fromarray(frame.astype(np.uint8)).resize((224, 224))   # OpenVLA 입력 크기

        # (b) 추론 — attention_mask 는 넘기지 않는다 (Phase 4 week6 의 크래시 회피)
        inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)
        with torch.no_grad():
            raw_action = vla.predict_action(
                input_ids=inputs["input_ids"],
                pixel_values=inputs["pixel_values"],
                unnorm_key=UNNORM_KEY,         # 실습 4 에서 확정
                do_sample=False,               # 결정적 출력 (제어에서는 무작위성 배제)
            )

        # (c) 변환 — 실습 4 계약 표의 규칙을 그대로 적용한다
        action = raw_action                    # <- 계약 표대로 단위·프레임·부호를 변환해 교체

        # (d) 실행
        obs, reward, terminated, truncated, info = env.step(action)

        # (e) 부분 도달률 갱신 — 판정식은 실습 3 의 3-4 접근 경로로 본인이 확정
        # stages["reached"] |= <그리퍼 끝이 큐브에 충분히 접근했는가>
        # stages["grasped"] |= <큐브를 물었는가>
        # stages["lifted"]  |= <큐브가 바닥에서 떴는가>
        stages["placed"] |= bool(info.get("success", False))

        if stages["placed"] or terminated or truncated:
            break

    records.append({"seed": seed, "steps": step + 1, **stages})
    print(f"   seed{seed:02d}: {records[-1]}")


env.close()


# -- 6-4. 요약 + 원시 결과 저장 --
print("\n[6-4] 요약")
for stage in ["reached", "grasped", "lifted", "placed"]:
    hit = sum(record[stage] for record in records)          # 해당 단계 도달 episode 수
    print(f"   {stage}: {hit}/{len(SEEDS)}")
with open("outputs/zeroshot_baseline.json", "w") as f:
    json.dump({"env_id": ENV_ID, "step_cap": STEP_CAP, "unnorm_key": UNNORM_KEY,
               "seeds": SEEDS, "records": records}, f, indent=2)
print("저장 완료: outputs/zeroshot_baseline.json")
```


**확인 포인트**

- `placed` 가 0/20 인 것은 정상 범위다 (README §7). 판정 대상은 그 앞 단계다
- `reached` 까지 0/20 이면 신호가 없다 -> task 를 쉽게 만들지 말고 **환경 정합** (embodiment·카메라 규약) 을 먼저 손본다


---


## 마무리: 증거 이관


측정이 끝나면 결과를 `Measurements/` 로 옮긴다. 순서를 지킨다 — **증거 보존이 Studies 원복보다 먼저**다 ([`Measurements/README.md`](../../../Measurements/README.md)).


```bash
mkdir -p "/workspace/study/physical-ai-study/Measurements/openvla-maniskill-zeroshot/raw"
mkdir -p "/workspace/study/physical-ai-study/Measurements/openvla-maniskill-zeroshot/scripts"
```


| 옮길 것 | 착지점 |
|---|---|
| `outputs/zeroshot_baseline.json` | `raw/` |
| `outputs/env_build.md`, `outputs/env_decision.md` | `environment.md` |
| `outputs/action_contract.md`, `outputs/sim_facts.md` | `methodology.md` |
| `outputs/harness_check.md` | `findings.md` 의 검증 절 |
| `practice_*.py` 중 측정 스크립트 | `scripts/` |
| 성공률·부분 도달률 해석 | `findings.md` 본문 (본인 문장으로) |


마지막에 `Portfolio/evidence-index.md` 에 한 줄 추가한다.
