"""OpenVLA int4 메모리 상세 실측 (findings.md §4.1 Block 1 / methodology.md §4).

3개 시점에서 torch allocator 값과 nvidia-smi 디바이스 사용량 증가분을 함께 읽는다:
    A. CUDA context 초기화 직후 (텐서 1개만) -- context 단독 크기
    B. 모델 로드 직후                        -- 가중치 footprint (2026-06 값 4.38 GB 대응)
    C. predict_action 1회 후                 -- activation·KV cache 포함 peak

nvidia-smi 값은 프로세스 행 매칭이 아니라 디바이스 전체 사용량의 baseline 대비
증가분(delta)이다 -- Docker 컨테이너에서는 nvidia-smi 가 보여주는 PID 가 호스트
네임스페이스 값이라 os.getpid() 와 매칭이 구조적으로 불가능하기 때문.
(전제: 측정 중 다른 프로세스의 GPU 사용량이 일정)

로드 조건은 2026-06 실측 (practice.ipynb) 과 동일: nf4, double quant,
compute dtype fp16, attn_implementation eager.

실행:
    python memory_breakdown.py
(반드시 새 프로세스로 실행 -- baseline 이 CUDA 초기화 전이어야 함.
 notebook 에서 쓸 때는 커널 재시작 직후 단독 실행)
"""

import subprocess
from collections import Counter

import numpy as np
import torch
from PIL import Image
from transformers import AutoModelForVision2Seq
from transformers import AutoProcessor
from transformers import BitsAndBytesConfig

GB = 1e9  # 기존 기록 (4.38 GB) 과 같은 10진 GB 단위
MIB = 1024**2  # nvidia-smi 출력 단위 (MiB) 환산용


def smi_used_gb():
    """nvidia-smi 로 디바이스 전체 VRAM 사용량을 GB 로 반환한다.

    절대값이 아니라 체크포인트 간 증가분(delta)으로 내 프로세스 몫을
    계산하는 용도 -- 호출부에서 baseline 을 빼서 쓴다.

    Returns:
        디바이스 전체 VRAM 사용량 (10진 GB)
    """
    # 디바이스 전체 사용량 조회 (MiB 숫자만, 헤더/단위 표기 없음)
    out = subprocess.run(
        ["nvidia-smi", "--query-gpu=memory.used", "--format=csv,noheader,nounits"],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    return int(out.strip()) * MIB / GB


# --- 시점 0: baseline (반드시 CUDA 초기화 전) ---------------------------------
base = smi_used_gb()  # 다른 프로세스들의 몫 -- 이후 모든 smi 값은 이 값 대비 증가분

# --- 시점 A: CUDA context 단독 ----------------------------------------------
torch.zeros(1, device="cuda")  # 텐서 1개로 CUDA context 초기화 (커널 이미지 로드 포함)
torch.cuda.synchronize()
smi_a = smi_used_gb() - base  # 증가분 = context 단독 크기의 근사 (하한)
print(f"[A] nvidia-smi (context 단독)      : {smi_a:.2f} GB")

# --- 모델 로드 (2026-06 실측과 동일 조건) ------------------------------------
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

# --- 시점 B: 로드 직후 --------------------------------------------------------
torch.cuda.synchronize()
alloc_b = torch.cuda.memory_allocated()  # 살아있는 텐서 바이트 합
reserved_b = torch.cuda.memory_reserved()  # allocator 가 쥔 블록 전체 (allocated 포함)
smi_b = smi_used_gb() - base
print(f"[B] memory_allocated (로드 직후)   : {alloc_b / GB:.2f} GB")
print(f"[B] memory_reserved  (로드 직후)   : {reserved_b / GB:.2f} GB")
print(f"[B] nvidia-smi       (로드 직후)   : {smi_b:.2f} GB")

# --- dtype 별 합산: findings §4.1 Step 3 의 근거 ------------------------------
acc = Counter()
for _, p in vla.named_parameters():
    # int4 가중치는 uint8 에 2개씩 packing 되어 있어 numel x element_size 가 곧 실바이트
    acc[str(p.dtype)] += p.numel() * p.element_size()
for _, b in vla.named_buffers():
    acc["buffer:" + str(b.dtype)] += b.numel() * b.element_size()

# quant_state (absmax scale 등) 는 named_parameters/buffers 에 안 잡히므로 별도 합산
qs_bytes = 0
for m in vla.modules():
    qs = getattr(getattr(m, "weight", None), "quant_state", None)
    if qs is None:
        continue
    tensors = [qs.absmax, qs.code]  # 1차 scale + 코드북
    if qs.state2 is not None:  # double quant 의 2차 scale
        tensors += [qs.state2.absmax, qs.state2.code]
    qs_bytes += sum(t.numel() * t.element_size() for t in tensors)

print("--- dtype 별 합산 (methodology.md §4 표에 붙여넣기) ---")
for dtype, nbytes in sorted(acc.items(), key=lambda kv: -kv[1]):
    print(f"  {dtype:<24}: {nbytes / GB:.3f} GB")
print(f"  {'quant_state (scale 등)':<24}: {qs_bytes / GB:.3f} GB")
total = sum(acc.values()) + qs_bytes
print(
    f"  합계 {total / GB:.2f} GB vs memory_allocated {alloc_b / GB:.2f} GB "
    f"(잔차 {(alloc_b - total) / GB:.3f} GB)"
)

# --- 시점 C: predict_action 1회 후 --------------------------------------------
torch.cuda.reset_peak_memory_stats()  # peak 기준점을 현재 (로드 직후) 로 리셋
image = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
prompt = "In: What action should the robot take to pick up the can?\nOut:"
inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)
# attention_mask 는 전달하지 않는다 (off-by-one 크래시 -- findings.md §3 workaround)
with torch.no_grad():
    vla.predict_action(
        input_ids=inputs["input_ids"],
        pixel_values=inputs["pixel_values"],
        unnorm_key="bridge_orig",
        do_sample=False,
    )
torch.cuda.synchronize()
peak_c = torch.cuda.max_memory_allocated()  # 추론 중 텐서 peak (KV cache·중간 activation 포함)
reserved_c = torch.cuda.memory_reserved()
smi_c = smi_used_gb() - base
print(f"[C] max_memory_allocated (추론 후) : {peak_c / GB:.2f} GB")
print(f"[C] memory_reserved      (추론 후) : {reserved_c / GB:.2f} GB")
print(f"[C] nvidia-smi           (추론 후) : {smi_c:.2f} GB")

# --- 차이 3분해: findings §4.1 Step 4 의 delta --------------------------------
print("--- 차이 3분해 ---")
print(f"  allocator 예약  (B: reserved - allocated)    : {(reserved_b - alloc_b) / GB:.2f} GB")
print(f"  CUDA context    (A 단독 / B: smi - reserved) : {smi_a:.2f} / {smi_b - reserved_b / GB:.2f} GB")
print(f"  activation·KV   (C: peak - B: allocated)     : {(peak_c - alloc_b) / GB:.2f} GB")
