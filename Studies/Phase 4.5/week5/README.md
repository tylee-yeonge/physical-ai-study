# Week 5 — eval harness: zero-shot vs fine-tuned (N회)


> **목표**: v1 의 sim 단일 task 루프를 재사용해, zero-shot 과 fine-tuned 를 **동일 조건 N회** 비교하는 eval harness 를 만든다.
> **선행**: week4 fine-tuned 모델, v1 의 sim task 루프.


## 학습 순서

1. eval harness 골격: **모델만 교체**(zero-shot / fine-tuned)하고 나머지 변인 고정
2. N (반복 횟수) 결정 + 시도별 성공/실패 + (가능하면) 성공까지 step 기록
3. 양쪽을 각각 N회 실행 → 원시 결과 저장 (`eval/outputs/`, gitignore)
4. 성공률 + **표준편차/신뢰구간** 산출


## 핵심 개념

- **변인 통제**: 양자화 조건/시드/초기 상태 분포를 zero-shot 과 fine-tuned 에 동일 적용. 안 그러면 차이가 adaptation 때문인지 노이즈인지 구분 불가.
- **N 과 분산이 본체**: 단일 성공률 숫자는 무의미. "N=30, 성공률 40% vs 47%, 신뢰구간 겹침" 같은 보고가 둘째 층 역량의 증거.
- **노이즈 가능성 인정**: 차이가 통계적으로 유의하지 않을 수 있고, 그 결론도 정당한 산출물(week6).


## 이번 주 산출 (must / nice)

- must: eval harness 코드 + zero-shot/fine-tuned 각 N회 원시 결과 + 성공률·분산 표
- nice: 시도별 성공/실패 타임라인 시각화


> PRACTICE/quiz 는 진입 시점에 작성한다 (스캐폴드).
