# Week 6 실습: 집계 -> 짝지은 비교 -> 배제 분석 -> 공개


> **실습 목표**: week5 원시 결과에 미리 정한 방법을 적용해 결론을 내고, 배제/미배제 표와 블로그 초고를 만들어 v1.5 를 공개한다.
> **예상 시간**: 8-10시간
> **원칙**: 실습 1 을 시작하기 전에 `week5/outputs/stat_method.md` 를 다시 읽는다. 여기서 방법을 새로 고르지 않는다.


---


## 환경 설정


분석은 GPU 가 필요 없다. 가벼운 venv 하나로 충분하다.


```bash
cd "/workspace/study/physical-ai-study/Studies/Phase 4.5/week6"
mkdir -p outputs/plots
source "../.venv-sim/bin/activate"          # 또는 numpy/scipy/matplotlib 가 있는 아무 venv
pip install -r requirements.txt
cat ../week5/outputs/stat_method.md         # 적용할 방법을 먼저 다시 읽는다
```


---


## 실습 1: 집계 + 구간


**파일명**: `analyze_results.py`


```python
"""
실습 1-2: week5 원시 결과를 집계하고 구간·짝지은 비교를 계산
"""
import json
import numpy as np
from scipy import stats                          # 이항 구간·검정용


STAGES = ["reached", "grasped", "lifted", "placed"]   # week0 에서 정한 단계
ALPHA = 0.05                                     # <- week5 stat_method.md 에 적은 값


def load(path):
    """jsonl 을 읽어 메타와 seed 별 레코드로 나눈다."""
    with open(path) as f:
        lines = [json.loads(line) for line in f]
    return lines[0]["_meta"], lines[1:]


zero_meta, zero_records = load("../week5/outputs/eval_zeroshot.jsonl")
ft_meta, ft_records = load("../week5/outputs/eval_finetuned.jsonl")


# -- 1-1. 조건 재확인 (week5 무결성 검사를 한 번 더 -- 분석 직전 마지막 관문) --
print("=" * 60)
print("[1-1] 메타 차이")
for key in set(zero_meta) | set(ft_meta):
    if zero_meta.get(key) != ft_meta.get(key):
        print(f"   {key}: {zero_meta.get(key)} -> {ft_meta.get(key)}")
assert [r["seed"] for r in zero_records] == [r["seed"] for r in ft_records], "seed 목록 불일치"
n = len(zero_records)
print(f"   N = {n}")


# -- 1-2. 단계별 집계 + 구간 --
# 구간 추정 방법은 week5 stat_method.md 에서 확정한 것을 쓴다.
# 경계(0/N)에서 폭이 0 이 되지 않는 방법이어야 한다.
print("\n[1-2] 단계별 성공 수와 구간")
summary = {}
for stage in STAGES:
    zero_hits = sum(r[stage] for r in zero_records)
    ft_hits = sum(r[stage] for r in ft_records)
    # 이항 비율 구간 (경계에서도 성립하는 방법)
    zero_ci = stats.binomtest(zero_hits, n).proportion_ci(confidence_level=1 - ALPHA)
    ft_ci = stats.binomtest(ft_hits, n).proportion_ci(confidence_level=1 - ALPHA)
    summary[stage] = {"zero": (zero_hits, zero_ci), "ft": (ft_hits, ft_ci)}
    print(f"   {stage:8s} zero {zero_hits:3d}/{n} "
          f"[{zero_ci.low:.3f}, {zero_ci.high:.3f}]  |  "
          f"ft {ft_hits:3d}/{n} [{ft_ci.low:.3f}, {ft_ci.high:.3f}]")
# 0/N 에서 low=0 이지만 high 는 0 이 아닌 것을 확인한다 -- 이 상한이 보고할 값이다
```


**확인 포인트**

- 0/N 인 단계의 구간 상한이 0 이 아닌가 (0 이면 방법이 잘못 적용됐다)
- 메타 차이가 `model`, `unnorm_key` 두 항목뿐인가 — 아니면 분석을 중단하고 week5 로 복귀


---


## 실습 2: 짝지은 비교


같은 스크립트에 이어 쓴다.


```python
# -- 2-1. 단계별 2x2 표 (짝지음의 본체) --
print("\n[2-1] 짝지은 비교")
for stage in STAGES:
    both = only_zero = only_ft = neither = 0
    for zero_record, ft_record in zip(zero_records, ft_records):   # 같은 seed 끼리
        z, f = zero_record[stage], ft_record[stage]
        both += z and f                          # 둘 다 도달
        only_zero += z and not f                 # zero 만 (악화 방향)
        only_ft += f and not z                   # ft 만 (개선 방향)
        neither += not z and not f               # 둘 다 미도달
    discordant = only_zero + only_ft             # 정보를 주는 관측 수
    print(f"\n   {stage}")
    print(f"      둘 다 {both} / zero만 {only_zero} / ft만 {only_ft} / 둘 다 아님 {neither}")
    print(f"      불일치 쌍 {discordant}개 (개선 {only_ft} / 악화 {only_zero})")
    if discordant > 0:
        # 불일치 쌍만 대상으로 한 부호검정 (week5 stat_method.md 의 단측/양측 규칙을 따른다)
        test = stats.binomtest(only_ft, discordant, p=0.5, alternative="two-sided")
        print(f"      p = {test.pvalue:.4f} (유의수준 {ALPHA})")
    else:
        print("      불일치 쌍 없음 -- 차이에 대한 정보가 없다")


# -- 2-2. 효율 비교: 성공한 episode 의 소요 스텝 --
print("\n[2-2] 성공 episode 의 스텝 수")
for records, name in [(zero_records, "zero"), (ft_records, "ft")]:
    steps = [r["steps"] for r in records if r["placed"]]      # 성공한 것만
    if steps:
        print(f"   {name}: n={len(steps)} median={np.median(steps):.0f} "
              f"min={min(steps)} max={max(steps)}")
    else:
        print(f"   {name}: 성공 episode 없음")
```


**해석의 경계**

- 불일치 쌍이 0 이면 "차이 없음" 이 아니라 **"이 N 으로는 차이에 대한 정보가 없음"** 이다
- 불일치 쌍이 한 자리 수면 p 값이 크게 흔들린다. 그 사실을 결과와 함께 적는다


---


## 실습 3: 결과 표·그림


**산출물**: `outputs/results.md`, `outputs/plots/`


보고 형식은 week5 실습 3-3 에서 만든 틀을 쓴다. 숫자만 채운다.


```python
# -- 3-1. 단계별 성공 수 막대 (짧게, inline) --
import matplotlib.pyplot as plt


x = np.arange(len(STAGES))                       # 단계 위치
zero_values = [summary[s]["zero"][0] for s in STAGES]
ft_values = [summary[s]["ft"][0] for s in STAGES]
plt.figure(figsize=(7, 4))
plt.bar(x - 0.2, zero_values, width=0.4, label="zero-shot")
plt.bar(x + 0.2, ft_values, width=0.4, label="fine-tuned")
plt.xticks(x, STAGES)
plt.ylabel(f"count (N={n})")
plt.legend()
plt.tight_layout()
plt.savefig("outputs/plots/stage_counts.png")
print("저장: outputs/plots/stage_counts.png")
```


`outputs/results.md` 에 채울 표:


| 단계 | zero-shot | 95% 구간 | fine-tuned | 95% 구간 | 불일치 쌍 (개선/악화) | p |
|---|---|---|---|---|---|---|
| reached | | | | | | |
| grasped | | | | | | |
| lifted | | | | | | |
| placed | | | | | | |


그리고 한 문장 결론. **README §4 의 과잉 주장 목록에 걸리는지 확인한 뒤** 확정한다.


---


## 실습 4: 배제/미배제 분석


**산출물**: `outputs/causal_analysis.md`


이번 주의 핵심이다. 원인을 나열하지 않고, **앞의 주차들이 실제로 배제한 것**을 근거와 함께 적는다.


### 4-1. 배제된 후보 표 채우기


각 행의 "근거" 칸에 **파일 경로와 판정 결과**를 적는다. "검증했다" 로 끝내지 않는다.


| 배제된 후보 | 근거 (파일 + 판정) |
|---|---|
| 통합 버그 (변환·프레임·부호) | week0 `harness_check.md` — 상한 성공률 __/__ |
| 라벨 변환 손실 | week1 `roundtrip_check.md` — 위치 오차 max __ m, 강한 기준 통과 |
| 데이터가 학습에 안 들어감 | week2 `load_check.log` — 배치 스키마·범위 통과 |
| 라벨 이중 정규화 | week2 `norm_check.md` — 정규화 후 대역 __ |
| 학습이 안 돌았음 | week3 `train_log.md` — __스텝 완주, loss __ -> __ |
| `unnorm_key` 오연결 | week4 `compat_check.md` — 두 키 대역 차이 확인 |
| 양자화 조건 차이 | week4 실습 2 — 양쪽 nf4 동일 설정 |
| 측정 조건 누출 | week5 실습 5-1 — 메타 차이 2항목 |


> 근거를 못 채우는 행이 있으면 **그 후보는 배제되지 않았다.** 아래 표로 옮긴다. 이것이 이 분석의 정직성을 지키는 규칙이다.


### 4-2. 남은 후보 표 채우기


| 남은 후보 | 왜 남는가 | 확인하려면 무엇이 필요한가 |
|---|---|---|
| 데이터 규모 | 최소선으로 정하고 그 이상 시도 안 함 | 규모를 늘린 2차 학습 |
| 상태 분포 협소 (expert-only) | week1 §7 구조적 한계 | 이탈 상태 라벨링 계열 방법 |
| 도메인 갭 | 이번 Phase 의 전제 | 도메인 정합을 높인 환경 |
| 학습 스텝·하이퍼파라미터 | 예산에서 역산, 탐색 안 함 | 탐색 실험 |
| task 난이도 | 단일 task 고정 | 다른 task |
| 학습의 무작위성 | 학습 1회, seed 반복 없음 | 같은 설정 반복 학습 |


세 번째 칸이 있는 이유: **다음에 무엇을 하면 이 후보가 갈리는지**를 적어 두면 이 표가 Phase 6-7 의 입력이 된다.


---


## 실습 5: 블로그 초고


**산출물**: `outputs/blog_draft.md`


README §6 의 6절 구조를 그대로 쓴다. 분량의 절반이 "실행: 무엇을 검증했는가" 와 "분석: 무엇이 배제되고 무엇이 남았는가" 에 간다.


```markdown
# (제목)

## 1. 문제
sim 에서 VLA 를 특정 task 에 adaptation 할 수 있는가. 왜 sim 인가(미학습 분포 + 타이밍),
그리고 그 선택의 한계.

## 2. 설계 — 무엇을 통제했는가
- 변인은 모델 하나. 고정한 항목 목록
- 지표를 두 층으로 (최종 성공률 + 부분 도달률) 둔 이유
- N 과 통계 방법을 결과 전에 정한 이유

## 3. 실행 — 각 단계에서 무엇을 검증했는가
- 하네스 검증 (상한 대조 / 공개 수치 대조)
- 라벨 round-trip
- 로드 검증 / 정규화 계약
- 4층 모델 검증
- 측정 조건 무결성

## 4. 결과
실습 3 의 표와 그림. 구간과 불일치 쌍을 함께.

## 5. 분석 — 배제와 잔여
실습 4 의 두 표. 여기가 이 글의 중심이다.

## 6. 한계와 다음
sim 증거의 한계, real 확장(Phase 7), 남은 후보를 가르려면 무엇이 필요한지.
```


**작성 규칙**

- 수치는 반드시 분자/분모 + 구간으로. 퍼센트 단독 금지
- README §4 목록에 걸리는 문장은 고친다
- "검증했다" 를 쓸 때마다 어떤 판정으로 통과했는지 한 줄 덧붙인다
- 실패·되돌린 지점을 적는다 (예: 라벨을 재생성한 이유). 매끄러운 서사보다 신뢰를 얻는다


---


## 실습 6: v1.5 공개


### 6-1. 코드 정리


| 대상 | 위치 |
|---|---|
| eval harness | week5 `eval_harness.py` |
| action 변환 (정/역) | 한 곳에 모은 구현체 |
| RLDS 빌더 | week2 |
| 등록 패치 + 기준 커밋 | week2 `outputs/` |
| Dockerfile | week3 |
| 분석 스크립트 | 이번 주 `analyze_results.py` |


README 에 적을 것: 재현 절차(순서대로), 각 단계의 검증 방법, 그리고 **재생성 불가한 것과 가능한 것의 구분**.


### 6-2. 증거 마감


```bash
cd /workspace/study/physical-ai-study
ls Measurements/openvla-maniskill-zeroshot/    # week0
ls Measurements/openvla-lora-runpod/           # week3-4
ls Measurements/openvla-lora-eval/             # week5-6
```


각 디렉터리에 `environment.md` / `methodology.md` / `raw/` / `findings.md` 가 채워져 있는지 확인한다. 이번 주 산출물의 착지점:


| 산출물 | 착지점 |
|---|---|
| `outputs/results.md` | `Measurements/openvla-lora-eval/findings.md` |
| `outputs/causal_analysis.md` | 같은 파일의 분석 절 |
| `outputs/plots/` | 같은 디렉터리 `plots/` |
| `analyze_results.py` | 같은 디렉터리 `scripts/` |


### 6-3. 공개


- `Portfolio/evidence-index.md` 에 행 추가 — 증거 / 날짜 / 위치 / 입증 역량
- 블로그 발행 (velog). LinkedIn 공유는 probe 2단 일정에 맞춘다
- 루트 `README.md` 의 실측 결과 절에 v1.5 행 추가 여부 판단 — **구간을 함께 적을 수 있을 때만** 추가한다


### 6-4. Roadmap 완료 체크리스트 대조


[`Roadmap/Phase 4.5.md`](../../../Roadmap/Phase%204.5.md) 의 완료 체크리스트를 열어 항목별로 닫힌 위치를 확인한다 (README §8 의 대조 표). 닫히지 않은 항목이 있으면 **체크하지 않고 사유를 적는다** — 미완을 완료로 표시하면 그 문서가 계획의 진실 공급원 역할을 못 하게 된다.


---


## 마무리


Phase 4.5 가 여기서 닫힌다. 남기는 것은 세 가지다.


| 산출물 | 다음에서의 용도 |
|---|---|
| 배제/미배제 표 | Phase 7 에서 real 로 옮기면 배제 목록이 리셋된다 — 무엇을 다시 검증해야 하는지의 목록 |
| eval harness + 변환 코드 | Phase 7 의 real 데이터 파이프라인이 재사용 |
| 한계 서술 | 2026.11 분기 재평가의 입력 (sim 증거가 둘째 층으로 읽히는지) |
