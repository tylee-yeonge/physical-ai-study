# Week 3 실습: 이미지 빌드 -> RunPod 이관 -> probe 실측 -> 본 사이클


> **실습 목표**: 학습 환경을 컨테이너로 고정해 RunPod 에서 재현하고, probe 로 예산을 확정한 뒤 LoRA 1사이클을 완주한다.
> **예상 시간**: 10-12시간
> **원칙**: 실습 4 (본 사이클) 는 실습 3 (probe 실측) 뒤에만 한다. 스텝당 비용을 모르는 상태로 긴 학습을 걸면 예산을 초과하거나 중간에 끊긴다.


---


## 실습 1: 학습 측 Docker 이미지


**파일명**: `Dockerfile`


week2 의 `.venv-rlds` 구성과 등록 패치를 이미지로 고정한다. 목적은 두 가지다 — RunPod 에서 같은 환경을 띄우는 것, 그리고 로컬 장애와 무관하게 재현 가능한 상태를 남기는 것.


```dockerfile
# 학습 측 이미지. sim 은 넣지 않는다 (Vulkan 요구로 난이도가 다르고, eval 은 로컬에서 돈다)
# 베이스 태그는 week2 outputs/env_rlds.md 에 기록한 CUDA·python·torch 조합과 맞춘다
FROM pytorch/pytorch:2.4.0-cuda12.1-cudnn9-devel

# 시스템 의존성 (git 은 리포 클론과 패치 적용에 필요)
RUN apt-get update && apt-get install -y --no-install-recommends \
        git rsync && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# OpenVLA 본체. 기준 커밋을 고정한다 -- week2 outputs/openvla_base_commit.txt 의 해시
ARG OPENVLA_COMMIT=<week2 에서 기록한 해시로 교체>
RUN git clone https://github.com/openvla/openvla.git && \
    cd openvla && git checkout ${OPENVLA_COMMIT}

# week2 의 등록 변경(3파일)을 패치로 적용. 손으로 다시 고치지 않는다
COPY openvla_registration.patch /tmp/
RUN cd openvla && git apply /tmp/openvla_registration.patch

# 의존성 설치 (리포 지침을 따른다. 정확한 명령은 openvla README 확인)
RUN cd openvla && pip install -e .

# 데이터셋과 체크포인트는 이미지에 넣지 않는다 -- 실행 시 volume 으로 붙인다
```


빌드와 검증:


```bash
cd "/workspace/study/physical-ai-study/Studies/Phase 4.5/week3"
cp ../week2/outputs/openvla_registration.patch .        # 이미지 빌드 컨텍스트로 복사
docker build -t openvla-train:v1 .                      # 빌드
# 등록이 실제로 들어갔는지 컨테이너 안에서 확인 (이미지가 조용히 잘못 만들어지는 것을 막는다)
docker run --rm openvla-train:v1 python -c \
  "from prismatic.vla.datasets.rlds.oxe.mixtures import OXE_NAMED_MIXTURES; \
   print([k for k in OXE_NAMED_MIXTURES if 'maniskill' in k])"
```


**통과 판정**: 위 명령이 내 mixture 이름을 출력한다. 빈 목록이면 패치가 적용되지 않았거나 파일 경로가 어긋난 것이다.


**기록할 것** (`outputs/image_build.md`): 베이스 이미지 태그, 기준 커밋, 빌드 중 겪은 문제, 이미지 크기.


---


## 실습 2: RunPod 기동 + 이관


**산출물**: `outputs/runpod_setup.md`


절차의 정본은 [`Studies/Phase 4/SETUP.md`](../../Phase%204/SETUP.md) §5 다. 여기서는 이번 학습에 필요한 항목만 확인한다. **요금은 셋업 시점에 직접 확인한다** — 변동하므로 자료에 수치를 박아 두지 않는다.


```bash
# 2-1. pod 생성 전 확인
#   - GPU: 24GB 급 (RTX 4090). VRAM 이 27GB 하한에 가까우므로 배치 조정이 필요하다 (README §1)
#   - network volume 을 만들어 붙인다 (pod 를 멈춰도 남는다)
#   - 컨테이너 이미지: 실습 1 의 이미지를 레지스트리에 올려 지정하거나, pod 에서 직접 빌드


# 2-2. 코드·데이터 전송 (RLDS 데이터셋은 용량이 크다 -- rsync 로 재개 가능하게)
rsync -avP ~/tensorflow_datasets/maniskill_pickcube \
    <pod-ssh>:/workspace/data/                       # <- pod 접속 주소로 교체


# 2-3. 전송 검증 (조용한 손실을 막는다)
#   양쪽에서 파일 수와 총 바이트를 비교한다
find ~/tensorflow_datasets/maniskill_pickcube -type f | wc -l
du -sb ~/tensorflow_datasets/maniskill_pickcube


# 2-4. pod 안에서 로드 검증 재실행 (week2 실습 4 와 같은 검사)
#   로컬에서 통과한 검사가 pod 에서도 통과해야 "이관 재현" 이 확인된다
```


**기록할 것** (`outputs/runpod_setup.md`)

| 항목 | 값 |
|---|---|
| GPU 종류 / VRAM | |
| 시간당 요금 (확인 시점 명기) | |
| network volume 크기 / 마운트 경로 | |
| 데이터 전송 시간 / 용량 | |
| pod 에서 로드 검증 통과 여부 | |


> 2-4 가 Section 0 의 "RunPod 에서 컨테이너 기동 + 재현 확인" 항목을 닫는다. 통과 로그를 남긴다.


---


## 실습 3: probe 실측 + 게이트 판정


**파일명**: `practice_probe_run.sh`


짧게 돌려 **스텝당 시간과 VRAM 피크**를 재고, 그 값으로 본 사이클을 설계한다.


```bash
#!/bin/bash
# 실습 3: 짧은 probe 로 스텝당 시간·VRAM 을 실측
set -e                                          # 실패 시 즉시 중단


cd /workspace/openvla


# 3-1. 인자 이름을 먼저 확인한다 (버전에 따라 다를 수 있다)
grep -n "batch_size\|grad_accumulation_steps\|max_steps\|save_steps\|lora_rank\|dataset_name\|data_root_dir\|run_root_dir" \
    vla-scripts/finetune.py | head -20


# 3-2. VRAM 을 백그라운드로 기록 (피크를 놓치지 않기 위해)
nvidia-smi --query-gpu=memory.used --format=csv -l 5 > /workspace/probe_vram.csv &
SMI_PID=$!


# 3-3. probe 실행 -- 스텝 수를 아주 작게 둔다
#   배치 조합은 24GB 에 맞춰 찾는다: batch_size 를 낮추고 grad_accumulation_steps 를 올려
#   유효 배치를 유지한다 (README §2). 아래는 시작점이며 OOM 이면 batch 를 더 낮춘다.
time torchrun --standalone --nnodes 1 --nproc-per-node 1 vla-scripts/finetune.py \
  --vla_path "openvla/openvla-7b" \
  --data_root_dir /workspace/data \
  --dataset_name maniskill_pickcube_only \
  --run_root_dir /workspace/volume/runs \
  --lora_rank 32 \
  --batch_size 1 \
  --grad_accumulation_steps 16 \
  --max_steps 20 \
  --save_steps 10


kill $SMI_PID                                   # VRAM 기록 종료
sort -t, -k1 -n /workspace/probe_vram.csv | tail -1   # 피크 값 확인
```


**계산해 기록할 것** (`outputs/probe_measure.md`)

| 항목 | 값 | 계산 |
|---|---|---|
| 스텝당 시간 | | probe 총 시간 / 스텝 수 (초기 로딩 시간은 분리) |
| VRAM 피크 | | 3-3 의 마지막 출력 |
| OOM 없이 성립한 배치 조합 | | batch / grad accumulation |
| 1시간에 가능한 스텝 수 | | 3600 / 스텝당 시간 |
| 예산 안에서 가능한 총 스텝 | | 허용 시간 x 위 값 |
| 확정한 `max_steps` | | 위 값에서 여유를 뺀 값 |
| 1사이클 예상 비용 | | 예상 시간 x 시간당 요금 |


**게이트 판정** (Roadmap Section 0)

| 판정 | 조건 | 다음 |
|---|---|---|
| 통과 | OOM 없이 돌고, 예산 안에서 의미 있는 스텝 수가 가능 | 실습 4 |
| 조건부 | 배치를 최소로 해도 아슬아슬하거나 시간이 예산을 넘김 | 데이터 규모 축소 또는 스텝 수 하향 후 재판정 |
| 실패 | 최소 배치에서도 OOM | 롤백 옵션 B — 경량 adaptation 으로 축소 (Roadmap §컴퓨트) |


> `max_steps` 를 지정하지 않으면 기본값이 사전학습급이라 끝나지 않는다 (README §4). probe 의 목적이 이 값을 정하는 것이다.


---


## 실습 4: 본 LoRA 1사이클


**파일명**: `practice_train_cycle.sh`


```bash
#!/bin/bash
# 실습 4: 본 LoRA 사이클. 체크포인트는 network volume 에 남긴다
set -e


cd /workspace/openvla


# 4-1. 실행 -- tmux 안에서 돌린다 (SSH 가 끊겨도 학습이 계속되도록)
#   실습 3 에서 확정한 값으로 채운다
torchrun --standalone --nnodes 1 --nproc-per-node 1 vla-scripts/finetune.py \
  --vla_path "openvla/openvla-7b" \
  --data_root_dir /workspace/data \
  --dataset_name maniskill_pickcube_only \
  --run_root_dir /workspace/volume/runs \
  --lora_rank 32 \
  --batch_size <실습 3 확정값> \
  --grad_accumulation_steps <실습 3 확정값> \
  --max_steps <실습 3 확정값> \
  --save_steps <총 스텝의 1/5 정도> \
  2>&1 | tee /workspace/volume/train.log        # 로그를 volume 에 남긴다 (pod 회수 대비)
```


학습이 끝나면 **회수 목록**을 확인한다. 하나라도 빠지면 다음 주가 막힌다.


```bash
# 4-2. 회수 대상 확인
ls -la /workspace/volume/runs/*/                 # 실행 디렉터리 내용
#   확인할 것:
#     - 머지된 가중치 (체크포인트 저장 시 어댑터가 머지된다 -- README §7)
#     - LoRA 어댑터 원본 (rank 비교나 재머지에 필요)
#     - 데이터셋 통계 파일 (없으면 추론에서 unnorm_key 를 쓸 수 없다 -- README §6)
#     - processor 설정
#     - 학습 로그


# 4-3. loss 추이 추출
grep -i "loss" /workspace/volume/train.log | tail -40
```


**기록할 것** (`outputs/train_log.md`)

- 최종 스텝 수 / 총 소요 시간 / 실제 비용 / VRAM 피크
- loss 추이 (시작·중간·최종 값)
- 회수 목록 4항목의 존재 확인 (특히 통계 파일)
- 학습 설정 전체 (스텝·배치·grad accumulation·rank·학습률)


> **loss 가 내려간 것은 통과 기준이 아니다** (README §8). 통과는 완주 + 실측 + 복구(실습 5) + 회수 목록이다.


---


## 실습 5: 중단 복구 리허설


**산출물**: `outputs/recovery_check.md`


인스턴스가 회수됐을 때 이어서 돌 수 있는지 **실제로 확인한다.** 회수된 다음에 처음 시도하면 그때는 방법을 찾을 시간이 없다.


```bash
# 5-1. 의도적 중단 -- 학습을 짧게 걸고 중간에 죽인다
#   (본 사이클을 끊지 말고, probe 규모로 별도 실행해서 리허설한다)


# 5-2. 체크포인트가 volume 에 남아 있는지 확인
ls -la /workspace/volume/runs/*/


# 5-3. 이어서 재개
#   finetune.py 가 체크포인트에서 재개하는 인자를 제공하는지 먼저 확인한다:
grep -n "resume\|checkpoint" /workspace/openvla/vla-scripts/finetune.py | head
#   - 인자가 있으면 그것을 쓴다
#   - 없으면 저장된 머지 가중치를 --vla_path 로 지정해 이어서 학습하는 방식이 대안이다
#     (이 경우 학습률 스케줄이 초기화되는 등 완전한 재개가 아니므로 그 한계를 기록한다)


# 5-4. pod 중지 -> 재기동 후 volume 이 살아 있는지 확인 (유휴 과금 차단 절차도 함께 검증)
```


**기록할 것**

| 항목 | 결과 |
|---|---|
| 저장 방식 (최신만 덮어쓰기 / 시점별 보관) 과 선택 근거 | |
| 재개 수단 (전용 인자 / 가중치 지정 우회) | |
| 재개의 한계 (학습률 스케줄, 옵티마이저 상태 손실 여부) | |
| pod 중지 후 volume 잔존 확인 | |


---


## 마무리: Section 0 닫기 + 다음 주로 넘기는 것


이번 주로 Roadmap Section 0 의 남은 항목이 닫힌다. 실측 결과를 `Measurements/` 에 남긴다.


```bash
mkdir -p "/workspace/study/physical-ai-study/Measurements/openvla-lora-runpod/raw"
```


| 산출물 | 착지점 |
|---|---|
| `outputs/probe_measure.md`, `outputs/train_log.md` | `Measurements/openvla-lora-runpod/findings.md` + `methodology.md` |
| `outputs/image_build.md`, `outputs/runpod_setup.md` | `Measurements/openvla-lora-runpod/environment.md` |
| `Dockerfile` + 등록 패치 | `Measurements/openvla-lora-runpod/scripts/` |
| VRAM 기록 csv, 학습 로그 | `raw/` |


week4 로 넘기는 것: 머지 가중치 / LoRA 어댑터 / **통계 파일** / 학습 설정. 가중치 본체는 커밋하지 않고 (`*.safetensors` 는 gitignore 대상), 위치와 재현 절차만 기록한다.


> `Portfolio/evidence-index.md` 에 한 줄 추가한다 — "클라우드 GPU 에서 LoRA 1사이클을 실측하고 재현 가능한 이미지로 고정" 은 배포 역량의 증거로 쓰인다.
