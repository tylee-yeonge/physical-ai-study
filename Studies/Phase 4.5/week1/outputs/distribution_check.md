# 미학습 분포 3단 논증 (실습 5)

> 작성일: 2026-08-06
> 용도: adaptation 의 의미를 주장하는 근거. week6 의 "배제된 후보" 표와 `Measurements/openvla-maniskill-zeroshot/findings.md` 가 인용한다
> 근거: 실습 5 실행 출력(`vla.norm_stats` 키 25개), week0 `outputs/zeroshot_baseline.json`, week0 `outputs/harness_check.md`

## 결론

"내 데이터는 OpenVLA 사전학습 분포와 겹치지 않는다"는 **틀린 진술**이다. 정확한 진술은 이것이다.

> **embodiment 와 task 는 겹친다. 겹치지 않는 것은 시각 도메인 하나다.**

따라서 이 Phase 의 adaptation 은 "새 로봇을 가르치는 것"도 "새 태스크를 가르치는 것"도 아니다. **렌더 화면에 대한 도메인 적응**이다. 이 구분을 흐리면 "팔은 아는 거잖아요" 한 마디에 논거가 무너진다.

## 판정 표

| 층 | 판정 | 근거 | 한계 |
|---|---|---|---|
| embodiment | **겹친다** | 학습 데이터셋 25개 중 13개가 Franka Panda 기반. 내 환경의 로봇도 Panda 7-DoF + 병렬 그리퍼다 | 데이터셋별 로봇 종류는 외부 문서 근거이고 이 환경에서 검증하지 않았다. 또한 추론은 `bridge_orig`(WidowX 250) 통계로 역정규화한다 — Panda 를 쓰면서 WidowX 의 동작 규모를 요구하는 셈이다 |
| 시각 도메인 | **겹치지 않는다** | 25개 전부 실로봇 카메라 영상이다. 내 관측은 SAPIEN/PhysX 래스터 렌더 224x224 합성 이미지다. 이름 검색의 `sim` 후보도 0건 | 이름 검색은 후보를 좁히는 단서일 뿐이다. "전부 실로봇"의 근거는 Open-X Embodiment 의 수집 정의이며 각 데이터셋 카드를 열어 확인한 것은 아니다 |
| task·물체·지시문 | **겹친다** | 집기(pick)는 25개 데이터셋의 지배적 동작이고, 정육면체 물체와 `"pick up the ..."` 형태의 지시문도 흔하다 | 정확한 조합(합성 큐브 + 이 문구)이 존재하는지는 데이터셋 본문을 열어야 판정된다. 이름 목록만으로는 알 수 없다 |
| (간접) zero-shot 성적 | **증거로 쓸 수 있다** | seed 0-19 에서 `reached 0/20`, `grasped 0/20`, `lifted 0/20`, `placed 1/20`. 유일한 성공(seed08)은 무행동 하한과 같은 seed 이므로 **실질 성공 0/20** | **week0 하네스 검증 통과가 선행 조건이고, 그 조건은 충족됐다** (scripted 해법 20/20). 단 그 검증이 배제하지 못한 용의자가 하나 남는다 — OpenVLA 출력을 ManiSkill action 으로 옮기는 변환 레이어. 따라서 성공률 0 의 원인에 "번역 오류" 가 아직 섞여 있다 |

## 층별 상세

### embodiment: 겹친다

학습 데이터셋 25개를 로봇 기준으로 나누면 절반 이상이 Franka Panda 다.

| 로봇 | 데이터셋 | 개수 |
|---|---|---|
| Franka Panda | austin_buds, austin_sailor, austin_sirius, viola, utaustin_mutex, taco_play, stanford_hydra, iamlab_cmu_pickup_insert, furniture_bench, fmb_dataset, toto, berkeley_cable_routing, nyu_franka_play | 13 |
| Google robot | fractal20220817_data, bc_z | 2 |
| Hello Robot Stretch | cmu_stretch, dobbe | 2 |
| WidowX 250 | bridge_orig | 1 |
| KUKA iiwa | kuka | 1 |
| Kinova Jaco | jaco_play | 1 |
| UR5 | berkeley_autolab_ur5 | 1 |
| Fanuc | berkeley_fanuc_manipulation | 1 |
| Sawyer | roboturk | 1 |
| xArm | ucsd_kitchen | 1 |
| DLR EDAN | dlr_edan_shared_control | 1 |

내 환경의 로봇은 ManiSkill 기본 agent 인 Franka Panda 다. 즉 **팔의 형태와 자유도, 병렬 그리퍼 구조는 모델이 이미 대량으로 본 것**이다. 이 층을 미학습 근거로 쓸 수 없다.

여기에 한 가지 비틀림이 있다. 추론 시 `unnorm_key` 로 `bridge_orig` 를 쓰므로(week0 `action_contract.md`), 모델의 출력은 **WidowX 250 의 동작 통계**로 역정규화된다. Panda 를 구동하면서 WidowX 의 step 당 변위 대역을 요구하는 구조다. 이것은 "Panda 를 모른다"는 문제가 아니라 **어느 분포의 스케일로 말하게 할 것인가**의 문제이고, 성적 해석에서 embodiment 미학습과 혼동해서는 안 된다.

### 시각 도메인: 겹치지 않는다

| 항목 | 내 데이터 | 학습 데이터 |
|---|---|---|
| 이미지 생성 방식 | SAPIEN/PhysX 래스터 렌더 | 실카메라 촬영 |
| 해상도 | 224x224 (`sensor_configs` 명시값) | 데이터셋별로 다름, 대개 실사 해상도에서 리사이즈 |
| 조명·재질 | 합성 조명, 단순 재질 | 실내 실조명, 실물 질감 |
| 배경 | 단색 테이블 + 소수 물체 | 실제 작업대, 잡동사니 포함 |
| 카메라 시점 | 고정 3인칭 `base_camera` | 대개 고정 3인칭 (이 항목만 부분 겹침) |

시점 규약은 부분적으로 겹치지만 **픽셀 자체의 생성 경로가 다르다.** 이것이 이 Phase 에서 유일하게 성립하는 미학습 근거다.

### task·물체·지시문: 겹친다

집기 동작은 학습 데이터의 중심 태스크다. 정육면체 물체와 `"pick up the cube"` 류의 지시문도 특이하지 않다. 따라서 "모델이 이 태스크를 처음 본다"는 주장은 성립하지 않는다.

다만 이 판정의 근거는 데이터셋 **이름**과 일반적 성격이다. 실제 지시문 문구와 물체 목록을 대조한 것이 아니므로, "정확히 같은 조합을 봤다"고도 "안 봤다"고도 단정할 수 없다. 판정을 "겹친다"로 두는 이유는 미학습을 주장하는 쪽에 입증 책임이 있기 때문이다 — 겹치지 않는다고 말하려면 근거가 필요하고, 지금은 그 근거가 없다.

### zero-shot 성적: 간접 증거로 성립

week0 `zeroshot_baseline.json` 의 20 episode 집계다.

| 단계 | 도달 수 |
|---|---|
| reached (TCP 가 큐브 0.05 m 안) | 0/20 |
| grasped | 0/20 |
| lifted (0.04 m 이상) | 0/20 |
| placed | 1/20 |

`placed` 1건은 seed08 이고, week0 하한 대조에서 **무행동 정책도 성공하는 seed** 로 확정된 값이다(`noop_seeds: [8]`, `policy_steps: 1`). 조작 능력의 증거가 아니므로 걷어내면 실질 성공은 **0/20** 이다.

`reached` 가 0 이라는 점이 특히 무겁다. 모델이 큐브 근처에도 가지 못했다는 뜻이므로, 성적 저하가 "잡기 정밀도 부족" 같은 미세한 문제가 아니다.

이 숫자를 미학습 분포의 증거로 쓸 수 있는 조건은 **하네스가 정상임이 먼저 증명되는 것**이고, week0 은 scripted 해법으로 20/20 을 받아 그 조건을 충족했다. 따라서 성공률 0 을 "장치 고장" 으로 읽을 필요는 없다.

그러나 week0 검증이 배제하지 못한 용의자가 하나 남는다. scripted 해법은 좌표를 직접 읽어 action 을 만들므로 **OpenVLA 출력을 번역하는 변환 레이어를 지나가지 않는다**(week0 `harness_check.md` §배제 범위). 즉 성공률 0 의 원인은 아직 두 갈래다.

| 후보 | 상태 |
|---|---|
| 모델이 이 렌더 화면을 모른다 (미학습 분포) | 살아 있음 |
| 내가 action 을 잘못 번역했다 (변환 레이어) | **배제되지 않음** |

## 이름 검색만으로 판정하면 어떻게 틀리는가

실습 5 의 이름 검색 결과와 실제 값이 크게 어긋난다. 이 절을 남기는 이유는 판정 방법 자체가 이번 실습의 교훈이기 때문이다.

| 질문 | 이름 검색 결과 | 실제 |
|---|---|---|
| Franka 기반 데이터셋은 몇 개인가 | 1개 (`nyu_franka_play...`) | 13개 |
| sim 데이터셋이 있는가 | 0건 | 0개 (이 건은 우연히 일치) |

Franka 항목을 **13개에서 1개로 축소**해 읽었다면 "embodiment 도 미학습" 이라는 결론이 나왔을 것이고, 3층 논증 전체가 뒤집힌다. 데이터셋 이름에 로봇 종류가 들어가는 것은 예외이지 규칙이 아니다.

## 확인이 남은 항목

- [ ] 데이터셋별 로봇 종류 — 위 13/25 는 외부 문서 근거다. Open-X Embodiment 논문의 데이터셋 표 또는 각 데이터셋 카드로 대조한다. 틀리면 embodiment 행의 판정이 바뀐다
- [ ] "학습 데이터 전부가 실로봇" — 수집 정의에 근거한 진술이다. 반례(합성 데이터 포함 데이터셋)가 하나라도 있으면 시각 도메인 행의 강도가 약해진다
- [ ] 지시문·물체 조합 — 데이터셋 본문의 language instruction 분포를 열어 `"pick up the cube"` 와 얼마나 가까운 문구가 있는지 확인한다
- [ ] 변환 레이어 배제 — 이 문서가 zero-shot 성적을 증거로 쓰는 한, 변환 레이어가 살아 있는 동안은 근거에 조건이 붙는다. week1 실습 3 의 round-trip 검증이 이 조건을 좁힌다

## week2 이후로 넘기는 함의

- **성적 향상을 주장할 때 층을 명시한다.** fine-tuning 후 성공률이 올라도 그것은 "Panda 를 배웠다"가 아니라 "이 렌더 도메인에 맞췄다"다. embodiment 와 task 를 근거로 끌어오면 과장이 된다
- **before/after 의 해석에 무행동 하한 1/20 을 같이 적는다.** 하한을 빼지 않으면 성공률 1/20 을 성과로 오독한다
- **변환 레이어가 배제되기 전까지, zero-shot 0/20 은 단독 증거가 아니다.** 이 문서를 인용할 때 위 두 갈래 표를 함께 인용한다
