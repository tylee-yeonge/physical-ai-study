"""SmolVLA (lerobot/smolvla_base) 추론 latency 측정 -- 스파이크 must 4 (D11).

무엇을 재는가:
    policy.select_action() 1회 = action chunk 1개 생성 (n_action_steps 개의 action).
    SmolVLA 는 한 번 모델을 돌려 action 을 여러 개 만들어 큐에 쌓고, 그 뒤 호출은 큐에서
    꺼내기만 한다. 그래서 매 반복 policy.reset() 으로 큐를 비워 반드시 모델이 돌게 만든다.
    (비우지 않으면 n_action_steps 회 중 1회만 모델이 돌아 대부분 0 ms 근처가 찍힌다)

OpenVLA 측정 (Measurements/openvla-rtx4070-int4/methodology.md §1) 과 맞춘 조건:
    n=100, warm-up 5회, batch 1, 매 반복 새 랜덤 이미지, task 문장 고정,
    torch.cuda.synchronize() 를 측정 구간 앞뒤 모두.

OpenVLA 와 다른 점 (RESULT.md 에 수치와 함께 적는다):
    - 1회 = action 1개 (OpenVLA) vs action n_action_steps 개 (SmolVLA chunk)
    - int4 양자화 (OpenVLA) vs 양자화 없음 (SmolVLA -- dtype 은 출력에 찍힌다)
    - 전처리 위치: 설치된 LeRobot 이 preprocessor 파이프라인을 제공하면 측정 구간 밖,
      제공하지 않는 구버전은 정규화·토크나이즈가 policy 안에서 일어나 측정 구간 안.
      어느 쪽이었는지 출력에 찍힌다

실행 (팔·카메라 연결 불필요 -- GPU 만 쓴다):
    acl    # lerobot venv 활성화 (/workspace/venvs/lerobot)
    python Studies/Hardware-Arm/spike/week2/scripts/measure_latency_smolvla.py
출력 (outputs/ 는 gitignore 대상 -- 수치는 RESULT.md §1 행 4 에 옮겨 적는다):
    Studies/Hardware-Arm/spike/week2/outputs/smolvla_latency_4070.npy
    Studies/Hardware-Arm/spike/week2/outputs/smolvla_latency_4070_summary.csv
"""

import contextlib
import csv
import os
import time

import numpy as np
import torch

from lerobot.policies.smolvla.modeling_smolvla import SmolVLAPolicy

try:
    # 신버전 LeRobot: 정규화·토크나이즈가 policy 밖의 파이프라인 (preprocessor) 으로 분리됨
    from lerobot.policies.factory import make_pre_post_processors
except ImportError:
    # 구버전 LeRobot: policy.select_action 이 내부에서 정규화·토크나이즈까지 수행
    make_pre_post_processors = None

MODEL_ID = "lerobot/smolvla_base"  # D10 zero-shot 과 같은 모델
TASK = "Pick up the red cube and place it on the tray."  # D9/D10 과 같은 문장 (고정)
N_WARMUP = 5  # 버리는 예비 실행 횟수 (CUDA 커널 로딩·메모리 할당 비용 제외)
N_ITER = 100  # 본 측정 횟수 (OpenVLA 와 동일)
OUT_DIR = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "outputs")
)

device = torch.device("cuda")  # RTX 4070

# --- 모델 로드 (lerobot-rollout 이 --policy.path 로 하는 것과 같은 경로) ---------------
policy = SmolVLAPolicy.from_pretrained(MODEL_ID)  # HF Hub 에서 가중치 + config 다운로드
policy.to(device)  # GPU 로 이동
policy.eval()  # dropout 등 학습 전용 동작 끔
torch.cuda.synchronize()  # 로드가 끝날 때까지 대기 (아래 메모리 값이 로드 완료 시점이 되게)
load_alloc_gb = (
    torch.cuda.memory_allocated() / 1e9
)  # 가중치 footprint (OpenVLA 4.38 GB 대응)

preprocessor = None
if make_pre_post_processors is not None:
    # 신버전: Hub 의 processor 설정으로 정규화·토크나이즈 파이프라인 생성 (postprocessor 는 불필요)
    preprocessor, _ = make_pre_post_processors(policy.config, pretrained_path=MODEL_ID)

# lerobot-rollout 의 sync 추론과 같은 조건 -- config 가 AMP 를 켜 두었으면 autocast 로 실행
amp = (
    torch.autocast(device_type="cuda")
    if policy.config.use_amp
    else contextlib.nullcontext()
)

n_action_steps = policy.config.n_action_steps  # 한 번 생성해서 실제로 쓰는 action 개수
chunk_size = policy.config.chunk_size  # 모델이 한 번에 생성하는 action 개수
dtype = str(next(policy.parameters()).dtype)  # 가중치 dtype (양자화 없음 확인용)
print(f"입력 키: {list(policy.config.input_features)}")
print(
    f"n_action_steps={n_action_steps} chunk_size={chunk_size} "
    f"dtype={dtype} use_amp={policy.config.use_amp}"
)
print(f"preprocessor 분리: {preprocessor is not None} (True 면 전처리는 측정 구간 밖)")
print(f"로드 직후 memory_allocated: {load_alloc_gb:.2f} GB")

latencies = []  # 본 측정 ms 누적 (warm-up 은 제외)
for i in range(N_WARMUP + N_ITER):
    # 관측 1개 (batch 1): config 가 선언한 모양대로 랜덤 텐서 -- 매 반복 새로 만들어 cache 효과 방지
    batch = {
        key: torch.rand((1, *feature.shape), device=device)
        for key, feature in policy.config.input_features.items()
    }
    batch["task"] = TASK  # 언어 지시문 (고정)
    if preprocessor is not None:
        batch = preprocessor(batch)  # 정규화·토크나이즈 (측정 구간 밖)
        # preprocessor 가 device 를 바꿨을 수 있으므로 텐서를 GPU 로 되돌린다
        batch = {
            k: v.to(device) if isinstance(v, torch.Tensor) else v
            for k, v in batch.items()
        }
    policy.reset()  # action 큐 비움 -> 이번 select_action 은 반드시 모델을 돈다

    torch.cuda.synchronize()  # 앞선 GPU 작업이 남아 있지 않게 비운다
    start = time.perf_counter()
    with torch.no_grad(), amp:
        policy.select_action(batch)  # chunk 1개 생성
    torch.cuda.synchronize()  # 이번 작업이 끝날 때까지 기다린다
    elapsed_ms = (time.perf_counter() - start) * 1000

    if i == N_WARMUP - 1:
        torch.cuda.reset_peak_memory_stats()  # peak 는 본 측정 구간만 센다
        print("warm-up 완료")
    if i >= N_WARMUP:
        latencies.append(elapsed_ms)
        if (i - N_WARMUP + 1) % 10 == 0:
            print(f"{i - N_WARMUP + 1}/{N_ITER}: latest = {elapsed_ms:.1f} ms")

arr = np.array(latencies)  # ms 배열 (n=100)
peak_gb = (
    torch.cuda.max_memory_allocated() / 1e9
)  # 본 측정 중 텐서 peak (activation 포함)
per_action_ms = arr.mean() / n_action_steps  # chunk 를 action 개수로 나눈 상각값
preproc_where = "밖" if preprocessor is not None else "안"  # 전처리가 측정 구간 밖/안

print("\n[latency 통계 -- chunk 1개 생성 기준]")
print(f"mean   : {arr.mean():.1f} ms  (action 당 {per_action_ms:.2f} ms)")
print(f"median : {np.median(arr):.1f} ms")
print(f"std    : {arr.std():.1f} ms")
print(f"min    : {arr.min():.1f} ms")
print(f"max    : {arr.max():.1f} ms")
print(f"p95    : {np.percentile(arr, 95):.1f} ms")
print(f"p99    : {np.percentile(arr, 99):.1f} ms")
print(f"chunk/s: {1000 / arr.mean():.2f} Hz")
print(f"peak VRAM (memory_allocated): {peak_gb:.2f} GB")
print(
    f"\nRESULT.md 1줄: SmolVLA base (4070, {dtype}): chunk mean {arr.mean():.1f} / "
    f"p95 {np.percentile(arr, 95):.1f} ms (n={N_ITER}, {n_action_steps} actions/chunk, "
    f"action 당 {per_action_ms:.2f} ms, 전처리 {preproc_where}) "
    f"vs OpenVLA int4 300.3 / 304.8 ms (action 1개)"
)

# --- 저장: 원본 배열 (npy) + 기계 판독 요약 (csv 1행) -----------------------------------
os.makedirs(OUT_DIR, exist_ok=True)
np.save(os.path.join(OUT_DIR, "smolvla_latency_4070.npy"), arr)
summary = {
    "mean_ms": f"{arr.mean():.3f}",
    "median_ms": f"{np.median(arr):.3f}",
    "std_ms": f"{arr.std():.3f}",
    "min_ms": f"{arr.min():.3f}",
    "max_ms": f"{arr.max():.3f}",
    "p95_ms": f"{np.percentile(arr, 95):.3f}",
    "p99_ms": f"{np.percentile(arr, 99):.3f}",
    "per_action_ms": f"{per_action_ms:.3f}",
    "n": N_ITER,
    "warmup": N_WARMUP,
    "n_action_steps": n_action_steps,
    "chunk_size": chunk_size,
    "dtype": dtype,
    "use_amp": policy.config.use_amp,
    "preprocessor_outside": preprocessor is not None,
    "load_alloc_gb": f"{load_alloc_gb:.3f}",
    "peak_alloc_gb": f"{peak_gb:.3f}",
}
with open(
    os.path.join(OUT_DIR, "smolvla_latency_4070_summary.csv"), "w", newline=""
) as f:
    writer = csv.writer(f)
    writer.writerow(summary.keys())  # 헤더
    writer.writerow(summary.values())  # 값 1행
print(f"저장: {OUT_DIR}/smolvla_latency_4070.npy, smolvla_latency_4070_summary.csv")
