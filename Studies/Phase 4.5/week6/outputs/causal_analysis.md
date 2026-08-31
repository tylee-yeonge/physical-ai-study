# 원인 배제/미배제 분석 (v1.5)

> 작성일: 2026-08-31
> 입력: `results.md` — placed 조작 성공 0/98 (양쪽), 불일치 쌍 0 (판정 불가), reached 0/98 -> 92/98, grasped 0/98 -> 75/98, lifted 0/98 -> 1/98
> 용도: "최종 성공률이 오르지 않은" 결과에 대해, week0-5 검증 기록이 원인 후보에서 실제로 배제한 것과 배제하지 못한 것을 구분한다. Phase 7 에서 real 로 옮길 때 재검증 목록으로 재사용된다.

## 0. 분석의 규칙

원인을 나열하지 않는다. 각 행의 근거 칸에는 파일 경로와 판정 수치를 적고, 근거를 채울 수 없는 후보는 배제 표에 넣지 않고 남은 후보 표로 옮긴다.

## 1. 배제된 후보

| 배제된 후보 | 근거 (파일 + 판정) |
|---|---|
| 측정 장치 고장 (환경 설정·episode 루프·성공 판정·step 예산) | week0 `outputs/harness_check.md` — 정답 좌표를 아는 scripted 해법이 같은 장치에서 20/20 성공. 소비 step 최대 49 / 예산 200 (약 4배 여유) |
| 라벨 변환의 수치 손실·규약 불일치 (위치 스케일·회전 부호·gripper 부호) | week1 `outputs/roundtrip_check.md` — 왕복 항등 최대 오차 1.49e-8 (float32 한계), week0 계약 표 실측 6/6 일치. week2 `outputs/norm_check.md` §4 — gripper 규약 (0=닫힘) 이 로더 규약과 일치, 라벨 고유값 {0, 1}. 보조 방증: fine-tuned 가 reached 92/98 — 변환 경로가 크게 틀렸다면 접근 자체가 성립하기 어렵다 |
| 회전 표현 불일치 (axis-angle vs euler) | week2 `outputs/format_spec.md` — 회전 표현 게이트 통과: 라벨 (euler delta 3차원) 이 로더의 EEF_POS 인코딩 구성과 일치, 라벨 재생성 없이 진행 확정 |
| 데이터가 학습에 안 들어감 | week2 `outputs/norm_check.md` §5.1 — 통계 파일 num_trajectories 100 / num_transitions 3760 이 라벨 원본 (`week1/outputs/dataset/ep*.npz`) 과 일치. §5.4 — 배치의 absolute_action_mask 실측 확인. week3 `outputs/train_log.md` — action token 정확도 0.12 -> 0.87 (그 데이터에 적합됨) |
| 라벨 이중 정규화 | week2 `outputs/norm_check.md` §5.2 — 통계 실측이 물리 단위 (위치 ±0.015 m, 회전 ±0.011 rad). 미리 정규화돼 있었다면 ±1 대역이었을 것 |
| 학습이 안 돌았음 | week3 `outputs/train_log.md` — 2,000 스텝 (8.5 epoch) 완주, 94.3분, train_loss 10.75 -> 0.40, l1_loss 0.412 -> 0.026 |
| 체크포인트 손상·재머지 오류 | week4 `outputs/remerge_check.md` — 색인 total_size 일치 (15082474368) + 텐서 이름 집합 일치. `outputs/compat_check.md` — 로드 시 가중치 관련 경고 0건, 결정성 3회 시행 bit 동일 |
| `unnorm_key` 오연결 | week4 `outputs/compat_check.md` §3 — `maniskill_pickcube` 키 추론 출력의 위치 델타 (±0.012) 가 그 키의 q01/q99 대역 (`norm_check.md` §5.2) 안이고 `bridge_orig` 대역 (±0.029-0.042, `roundtrip_check.md` 라벨 대역 표) 과 구분된다. eval 메타에서 모델-키 짝 일치 (zero-shot: bridge_orig / fine-tuned: maniskill_pickcube) |
| 양자화 조건 차이 | 두 eval 메타의 quant 필드 동일 (`nf4+dq+fp16`, `eval_*.jsonl` `_meta`). week4 `outputs/compat_check.md` §2 — 4bit NF4 적재 VRAM 4.38 GB, baseline 대비 +0.00 GB |
| 측정 조건 누출 | week6 `scripts/analyze_results.py` [1-1] — 메타 차이 `model`, `unnorm_key` 2항목뿐, seed 목록 순서까지 일치 (assert 통과) |

## 2. 남은 후보

배제하지 못했으므로 남긴다. 세 번째 칸은 Phase 6-7 계획의 입력이다.

| 남은 후보 | 왜 남는가 | 확인하려면 무엇이 필요한가 |
|---|---|---|
| 데이터 규모 (100 episode / 3,760 transition) | 최소선으로 정했고 그 이상을 시도하지 않았다 | 규모를 늘린 2차 학습 |
| 상태 분포 협소 (expert-only) | week1 §7 의 구조적 한계 — 계획된 궤적에서 이탈한 상태의 정답이 데이터에 없다. grasp 이후 lift 로 넘어가지 못한 74/75 관측과 양립하지만, 이 관측만으로 확정할 수 없다 | 이탈 상태 라벨링 계열 방법 (DAgger 류), 또는 파지 이후 상태·action 로그의 정밀 분석 |
| 도메인 갭 | 이번 Phase 의 전제 — sim 렌더는 OpenVLA 사전학습 분포 밖이다 | 도메인 정합을 높인 환경, 또는 real 데이터 (Phase 7) |
| 학습 스텝·하이퍼파라미터 | 예산에서 역산한 값 (lr 5e-4, rank 32, 2,000스텝) 이고 탐색하지 않았다 | 탐색 실험 |
| task 난이도 | PickCube 단일 task 로 고정했다 | 다른 task |
| 학습 실행의 무작위성 | 학습을 1회만 돌렸다 — seed 를 바꾼 반복이 없다 | 같은 설정 반복 학습 |
| 라벨 재생 무결성 (강한 기준) | week1 `outputs/roundtrip_check.md` 판정 표의 "sim 재생 위치 오차 max < 0.001 m" 행이 기록상 미측정 (재실행 대기) 으로 남아 있다 — 왕복 항등·계약 정합만으로는 재생 경로가 닫히지 않는다 | week1 실습 3 재생 스크립트 재실행 후 판정 표의 두 행을 채우기 |

## 3. 관측이 주는 힌트와 그 한계

fine-tuned 의 실패는 접근·파지 단계가 아니라 그 이후에 몰려 있다 (grasped 75/98 중 lifted 1/98). 이 패턴은 "이탈 상태에서의 회복 능력 부재" (상태 분포 협소) 나 "파지 이후 구간의 라벨 품질" 쪽 후보와 양립하지만, 성공 관측이 0 이라 이 데이터만으로는 남은 후보들 사이의 우선순위를 정할 수 없다. 패턴의 해석은 가설이지 판정이 아니다.

## 4. 이 분석이 보장하지 않는 것

- 남은 후보 중 어느 것이 실제 원인인지 이 데이터로 가릴 수 없다.
- 배제는 이 조건 (PickCube-v1, N=100, seed 0-99, nf4+dq+fp16, 이 학습 1회) 에 한정된다. real 로 옮기면 배제 목록이 리셋되며, 이 표가 그때의 재검증 목록이 된다 (Phase 7).
