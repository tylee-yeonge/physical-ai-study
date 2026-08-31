# v1.5 eval 결과 보고 — zero-shot vs LoRA fine-tuned (PickCube-v1)

> 작성일: 2026-08-31
> 원시 결과: `../week5/outputs/eval_zeroshot.jsonl`, `../week5/outputs/eval_finetuned.jsonl` (하네스 commit f2cabad)
> 방법 정본: `../week5/outputs/stat_method.md` — Wilson score 구간, exact McNemar 양측, alpha 0.05, 무행동 seed {8, 58} 제외 98쌍 기준
> 분석 스크립트: `scripts/analyze_results.py` (week6 cwd 에서 실행)
> 그림: `plots/stage_counts.png`

## 1. 측정 조건 재확인

- 두 파일의 메타 차이는 `model`, `unnorm_key` 2항목뿐 — 분석 관문 통과.
- 공통 조건: `PickCube-v1`, N=100 (seed 0-99), `max_episode_steps` 200, action repeat 4 (정책 결정 50회), 양자화 nf4+dq+fp16 동일, instruction "pick up the cube".
- 무행동 하한 seed {8, 58}: 초기 배치가 성공 조건을 이미 충족하는 degenerate 배치 (`eval_plan.md`). 두 모델에 똑같이 작용하므로 짝지은 비교에는 영향이 없고, 구간 추정의 분자·분모에서만 제외한다 (98쌍 기준).

## 2. 단계별 결과 표

98쌍 기준. 괄호는 100쌍 원값. 구간과 검정은 1차 지표 `placed` 에만 붙인다 — 단계별 지표는 기술 통계만 보고한다 (`stat_method.md` §3, 다중 비교 회피).

| 단계 | zero-shot | 95% Wilson 구간 | fine-tuned | 95% Wilson 구간 | 불일치 쌍 (개선/악화) | p |
|---|---|---|---|---|---|---|
| reached | 0/98 (0/100) | - | 92/98 (92/100) | - | 92 (92/0) | - |
| grasped | 0/98 (0/100) | - | 75/98 (75/100) | - | 75 (75/0) | - |
| lifted | 0/98 (0/100) | - | 1/98 (1/100) | - | 1 (1/0) | - |
| **placed** | **0/98** (2/100) | [0%, 3.8%] | **0/98** (2/100) | [0%, 3.8%] | **0** (0/0) | 판정 불가 |

- `placed` 의 100쌍 원값 2/100 은 두 모델 모두 무행동 seed {8, 58} 그 자체다 — 조작으로 얻은 성공은 양쪽 모두 0건.
- `placed` 2x2 표 (100쌍): 둘 다 성공 2 (무행동 seed) / zero 만 0 / ft 만 0 / 둘 다 실패 98. 불일치 쌍 0 이므로 검정 자체를 하지 않는다 (`stat_method.md` §2.3).

## 3. 보고 문장 (stat_method.md §4 틀)

```
zero-shot   0/98 (95% Wilson 구간 0-3.8%) [전체 2/100]
fine-tuned  0/98 (95% Wilson 구간 0-3.8%) [전체 2/100]
불일치 쌍   개선 c=0건 / 악화 b=0건 -> 판정 불가 (불일치 쌍 0)
단계별      reached 0/98 -> 92/98 / grasped 0/98 -> 75/98 / lifted 0/98 -> 1/98
결론        최종 성공 지표 placed 는 두 모델 모두 조작 성공 0/98 (95% Wilson 상한 3.8%),
            불일치 쌍 0 으로 판정 불가. 부분 도달 지표는 fine-tuned 쪽에서
            reached 0/98 -> 92/98, grasped 0/98 -> 75/98 로 이동했으나
            lifted 1/98 이후로 이어지지 않았다.
```

## 4. 효율 지표 (성공 episode 스텝 수)

조작 성공 episode 가 양쪽 모두 0건이므로 스텝 수 비교는 성립하지 않는다. 무행동 seed 의 성공은 env step 1-2 에서 끝나는 degenerate 배치이며 조작 효율의 관측이 아니다.

## 5. 관측 서술 (기술 통계 수준)

- zero-shot 은 어느 단계에도 도달하지 못했다 (reached 0/98). week0 N=20 관측과 같은 방향이다.
- fine-tuned 는 98쌍 중 92건이 큐브 5cm 이내 접근(reached), 75건이 파지(grasped)까지 도달했으나, 파지한 74건이 들어올리기(lifted, z 4cm)로 이어지지 못했다. lifted 1건 (seed 72)도 placed 로 이어지지 않았다.
- 종료 사유는 양쪽 모두 98건이 step 소진(`env_end`), 2건이 무행동 seed 의 즉시 성공이다.

이 서술은 개수의 보고이며, "성능이 개선됐다" 같은 지표 이름 없는 요약으로 축약하지 않는다 (README §4 과잉 주장 목록).
