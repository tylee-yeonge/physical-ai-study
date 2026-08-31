# 한국 Physical AI 채용 시장 조사 — 상방/현실/하방 + 타겟 직군 정의

> 작성일: 2026-08-31 (웹 조사 기준 — JD 원문은 정독 시 재확인 필수, 유통기한 ~재평가 #1)
> 성격: **분기 재평가 #1 (2026.11) 의 시장 신호 입력.** 최종 목표 (FM 을 만지는 역할) 로 가는 현실 경로 (FM 연동 시스템 SW) 의 시장 실체를 확인한다
> **표기 경고**: 본 문서는 사용자 지시 (2026-08-31) 로 **회사 실명을 표기한다 — 리포 실명 금지 원칙 (career-review-sync spec §8.2) 의 명시적 예외.** 이 문서의 어떤 내용도 vla-lab 등 공개 채널로 이관·인용하지 않는다. 이관 시 익명화 필수

---

## 0. TL;DR

1. **"FM 연동 시스템 SW" 는 실재하는 직군이다** — RLWRLD 의 제품 조직 구성 (teleop·데이터 파이프라인 / Training / Benchmark / Robot Control Systems / GPU Infra / API·SDK) 이 공식 서술로 확인됨. 이름 붙이면 **로봇 학습 시스템 (Robot Learning Systems)** 엔지니어.
2. **채용 파도는 커지는 중** — K-휴머노이드 연합 (2030 까지 1조원, 40여 기관), 로보티즈 "역대급 대규모 채용", FM 스타트업 3-4곳 상시 채용. 2027.03 파도 소멸 우려는 현재 관측상 기각 (지속 관측은 유지).
3. 정조준은 **FM 조직의 시스템 SW** (스타트업 Robotics S/W + 로보티즈 시스템 개발직군). 상방 (모델 직접) 은 석박 벽 — 입사 후 내부 사다리로 노린다.
4. 3층 프레임은 유효하되 2가지 수정: 둘째 층의 실체는 LoRA 가 아니라 **데이터 엔진+post-training+평가**, 그리고 시장이 실제로 뽑는 **2.5층 (모델-로봇 시스템 층)** 을 명명한다 — 현 로드맵 산출물이 정확히 이 층이다.

---

## 1. 시장 지형 변화 (2026.08 관측)

| 변화 | 내용 |
|---|---|
| K-휴머노이드 연합 출범 | 2030 까지 1조원, 약 40개 산학연. 서울대 AI 연구원 중심 AI 그룹이 **2028 까지 공용 로봇 AI 파운데이션 모델** 개발. 로보티즈·레인보우로보틱스·에이로봇·홀리데이로보틱스·로브로스 5사가 실기 휴머노이드 제공. 리벨리온·DEEPX (반도체), LG전자·HD현대로보틱스·두산로보틱스·뉴로메카·위로보틱스·엔젤로보틱스·CJ대한통운 참여 |
| FM 전업 스타트업 층 형성 | RLWRLD (RFM 개발, 시드 ~600억, 촉각·힘 포함 자체 모델 RLDX-1, 채용 11건) / 홀리데이로보틱스 (시뮬레이션 기반 RL + 정밀 핸드, 채용 7건) / 에이로봇 / 위로보틱스 (ALLEX 핸드, pouring 70.8%) |
| 오픈소스 노선 확정 | **로보티즈가 휴머노이드 'AI 사피엔스' SW 전면 공개** (보행·균형 제어 코드 + Sim2Real 도구 + RL 워크플로, 상업 활용 허용) + 오픈 휴머노이드 짐 (트레이너 육성). LeRobot 생태계 정합 노선 |
| 대기업 본격화 | 삼성전자 미래로봇추진단 (사내 채용 중심 — 외부 진입 좁음), 현대차 로보틱스랩 (로봇 SW·E2E 관제 4건 + 9월 SDV·AI·로보틱스 대규모 채용), 네이버랩스 (피지컬AI 8개 분야 — 로봇·디지털 트윈·3D 비전), 두산로보틱스 (강화/모방학습 조작·제어 + AI Engineering LLM/VLM), AWS Physical AI Frontier Program (2026.07-12, 15개사) |

## 2. 상방 / 현실 / 하방

| 밴드 | 대상 | 직무 실체 | 진입 판정 (본인 기준) |
|---|---|---|---|
| **상방** — FM 을 직접 만짐 | RLWRLD AI Research Engineer / 서울대 AI 연구원 K-휴머노이드 FM 그룹 / 삼성 미래로봇추진단 AI / 네이버랩스 리서치 | 모델 학습·아키텍처·post-training | 석박·논문 벽. **지금 정면 돌파 대상 아님 — 현실 밴드 입사 후 내부 이동으로** |
| **현실 (정조준)** — FM 조직의 시스템 SW | ① RLWRLD **Robotics S/W Engineer** ② 홀리데이로보틱스 (로봇 시뮬레이션 플랫폼 엔지니어 등) ③ 에이로봇 ④ **로보티즈 휴머노이드 시스템 개발직군** (AI Worker·AI 사피엔스 — ROS2·리눅스·SW 아키텍처 요구) ⑤ 레인보우로보틱스 SW (RB-Y1 = 모방학습 연구 플랫폼, AI 연구용 API) ⑥ 두산로보틱스 로봇 SW ⑦ 현대차 로보틱스랩 로봇 SW·E2E | **모델을 실기에 물리는 배관 전체**: teleop 데이터 수집 → 학습 시스템 → 평가 하네스 → 실기 제어 통합 → 서빙·SDK | **주력 무기 = 양산 ROS 5년 + 실기 통합 증거.** 학위 벽 없음. 12-24명 스타트업은 시니어 실무자 수요 절실. v1.5·v2.5·Stage 1 이 JD 언어와 1:1 매핑 (§4) |
| **하방 (안전망)** | 라이드플럭스·뷰런테크놀로지·서울로보틱스·포티투닷·스트라드비전 (AV/perception), 협동로봇·물류 로봇 SW | 인지·자율주행 스택 | 기존 5년 + Phase 3 로그로 현재도 지원 가능. **부록 E fallback 유효 재확인** |

## 3. 타겟 직군 정의 — "로봇 학습 시스템" (ROS 미들웨어 role 과의 구분)

- ROS 미들웨어 role: 노드 배선·토픽 설계·배포 운영. 모델은 남이 주는 블랙박스 → 매력 저하 우려가 맞음
- **로봇 학습 시스템 role**: teleop 데이터 파이프라인 / 모델 평가 하네스 / 정책의 실기 통합·latency / 데이터셋 품질 — **모델 옆에서, 모델 때문에 존재하는 시스템.** FM 조직 엔지니어의 절반이 이 일을 한다 (RLWRLD 조직 서술이 직접 증거)
- **FM 으로 가는 사다리**: 데이터·eval 을 잡은 엔지니어 → post-training (데이터 큐레이션·RL 후처리) → 모델. FM 조직의 표준 내부 트랙

## 4. 3층 프레임 판정 (README "3개 층" 절 대응)

- **유효**: 첫째 층 범위 밖 / 셋째 층 = 본인 강점 좌표는 시장이 재확인
- **수정 1**: 둘째 층 (adaptation) 의 2026 실체 = **데이터 엔진 + post-training(RL) + 평가** (LoRA 는 그 부분집합)
- **수정 2**: 둘째-셋째 사이에 채용 시장이 실제로 뽑는 **2.5층 = 로봇 학습 시스템 층** 명명. 현 산출물 매핑: v2.5 teleop 데이터셋(HF Hub)=데이터 파이프라인 / v1.5·v2.5 eval 하네스(사전 등록 통계·부분 도달률)=Benchmark / Stage 1 ROS2 래핑+이중 latency=Robot Control Systems / LeRobot 스택=로보티즈 오픈소스 노선 정합
- **실행 함의**: 서사 교체 불요, **진열 순서 교체** — eval 하네스·데이터셋을 포트폴리오 전면에, 양자화·latency 는 받침 (재평가 #1 안건)

## 5. JD 정독 대상 (master roadmap #3 의 목록 — 10월 마감)

| # | 회사 / 직군 | 정독 포인트 (자격요건 vs 산출물 매핑) |
|---|---|---|
| 1 | 로보티즈 — 휴머노이드 로봇 시스템 개발직군 | ROS2·리눅스·SW 아키텍처 요구 ↔ Stage 1 ROS2 래핑 / LeRobot ↔ AI 사피엔스 노선. C++ 비중 확인 |
| 2 | RLWRLD — Robotics S/W Engineer | 제품 조직 5개 시스템 중 어디 뽑나 (Control Systems? Data?) ↔ v2.5·Stage 1 |
| 3 | 홀리데이로보틱스 — 시뮬레이션 플랫폼 등 | Isaac/sim 스택 요구 ↔ Phase 6 경로·isaac-sim 코스 판단 입력 |
| 4 | 에이로봇 — SW 직군 | K-휴머노이드 5사 중 하나. 요구 스택 확인 |
| 5 | 레인보우로보틱스 — SW/AI (5건) | RB-Y1 모방학습 플랫폼 쪽인지 제어 쪽인지 분별 |
| 6 | 두산로보틱스 — 로봇 SW / AI Engineering | 강화·모방학습 조작 vs LLM/VLM — 요구 학위 확인 |
| 7 | 현대차 로보틱스랩 — 로봇 SW·E2E 관제 (+9월 대규모) | 시스템 통합 성격 강함. 9월 공고 열리면 즉시 정독 |
| 8 | 네이버랩스 — 피지컬AI (8분야) | 연구 성향 강도 확인 — 상방/현실 어느 쪽인지 |

산출: 요구 역량 vs v1/v1.5/v2.5/Stage 1 격차 매핑 1페이지 (재평가 #1 입력)

## 6. probe 제안 — AI 사피엔스 기여

1순위 타겟이 코드를 전면 공개한 지금, **공개 코드 정독 + 이슈/기여 1건**이 커피챗보다 강한 신호다 (Stage 1 구간 경량 슬롯 — 커피챗과 택1).

## 7. 출처 (2026-08-31 검색 — 정독 시 원문 재확인)

- 로보티즈: [대규모 채용 (사람인)](https://www.saramin.co.kr/zf_user/jobs/relay/view?rec_idx=52705987) · [AI 사피엔스 SW 공개 (ZDNet)](https://zdnet.co.kr/view/?no=20260803091554) · [오픈 휴머노이드 짐 (ZDNet)](https://zdnet.co.kr/view/?no=20260831134423)
- K-휴머노이드: [출범 (인공지능신문)](https://www.aitimes.kr/news/articleView.html?idxno=34578) · [1조 투자 (굿모닝경제)](https://www.goodkyung.com/news/articleView.html?idxno=261911)
- RLWRLD: [원티드](https://www.wanted.co.kr/company/51934) · [RLDX-1 (벤처스퀘어)](https://www.venturesquare.net/1080678) · [채용 페이지](https://realworld.career.greetinghr.com/)
- 홀리데이로보틱스: [원티드](https://www.wanted.co.kr/company/55786) · [시뮬레이션 플랫폼 (데모데이)](https://demoday.co.kr/recruits/15353) / [에이로봇](https://arobot4all.career.greetinghr.com/ko/intro)
- 대기업: [삼성 미래로봇추진단 (이코노미트리뷴)](https://www.economytribune.co.kr/news/articleView.html?idxno=3902543) · [현대차 로보틱스랩](https://robotics.hyundai.com/lab/careers/list.do) · [현대차 9월 채용 (오토스파이넷)](https://www.autospy.net/news/16246) · [네이버랩스 (더구루)](https://www.theguru.co.kr/news/article.html?no=105310)
- 기타: [AWS Physical AI (벤처스퀘어)](https://www.venturesquare.net/1107216) · [라이드플럭스 Perception](https://pharm.korea.ac.kr/cdc/6304/subview.do)
