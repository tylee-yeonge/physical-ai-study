# Phase 4 진행 보드 + 작업 노트

> Phase 4 는 이 파일만 보고 진행한다 — 실행 체크리스트(진행 보드)와 순서별 노트 산출물을 한 파일에서 관리하며, 체크 표기는 여기서만 갱신한다.
> 계획의 배경·시간 예산·산출물 정의는 [`Roadmap/Phase 4.md`](../../Roadmap/Phase%204.md), 환경 구축·컴퓨트 수치는 [`SETUP.md`](SETUP.md) (수치 본체는 §1.3).
> 노트(skim 답 등)는 항목당 몇 줄이면 충분하다 — 정독 노트가 아니다. 모든 경로는 `Studies/Phase 4/` 기준.

## 진행 순서 한눈에

이 표의 행 순서가 곧 진행 순서다. 주차 번호는 재부여하지 않는다 — week 디렉토리와 자료 구조는 원안 그대로다 (자료 맵은 `Roadmap/Phase 4.md`의 "원안 week 구조" 절).

| 순서 | 진행할 week | 작업 | 시기 | 계획 투입 | 상태 |
|---|---|---|---|---|---|
| 1 | `SETUP.md` (week 아님) | Step 0: 환경 구축 + 레포 청소 + 4-bit 로드 + latency/VRAM 실측 | 6월 2주차 | 6h | 완료 |
| 2 | `week4`-`week5` 일부만 | 표적 skim: action 표현 / unnorm_key / 입력 형식 / embodiment 가정 (정독 아님 — 정독은 순서 6) | 6월 2-3주차 | 4h | 완료 |
| 3 | `week11` 일부 선행 | sim 환경 선정 + embodiment 정합 + 성공 task 1종 정의 (→ latency 판정 수치 확정) | 6월 2-3주차 | 12h | 완료 |
| 4 | `week8` → `week12` 순서대로 | 실습 압축: VLAInference → ROS2 노드 → sim 단일 task 루프 | 6/17 - 6월 4주차(목표 6/26, 5h 추세 시 6/29) | 34h | 진행 중 |
| 5 | `week11`-`week12` 마무리 | 성공률 N회 측정 + 결과 정리 | 6월 5주차(목표 6/30), 5h 추세 시 일부 7월 첫 주 이월 | 22h | 대기 |
| 6 | `week1`,`week2`,`week4`,`week5`,`week6` (정독만) | RT-2/OpenVLA 정독 — 아키텍처 이해 (블로그 작성 `week3`/`week7` 은 v2 로 이관) | 8-9월 | 20-30h 잠정 (8월 초 체크포인트에서 갱신) | 대기 |

> 순서 7(블로그 마무리 + 패키징 + 1분 영상 + v1 공개)는 v1 에서 제외하고 v2(Phase 6)로 이관했다. v1 의 종착점은 순서 5(성공률 측정 + 레포 결과 기록)이며, 순서 6 정독까지가 v1 범위다. 이관 사유와 범위는 `Roadmap/Phase 4.md` "산출물 v1" 절 참고.

## 진행 원칙

- 순서 1-5 (6-7월) 합계 78h = 계획선. 최대 가용(96h)과의 차이 약 18h 는 버퍼 — 막히는 구간(실습/측정)에 흡수한다.
- 6월 미달 시 절삭 순서 (실가용이 5h 추세로 56h에 미달할 때): (1) 버퍼 소진 (6월 잔여 + 7월 첫 주) → (2) 성공률 측정 N 축소: 베이스라인 20 → 하한 10 → (3) 측정 항목 축소 (latency 분포는 유지, 부가 지표 절삭). v1 산출물 정의(루트 README 부록 B)를 건드리는 절삭은 별도 의사결정 — 발동 시 부록 B 갱신 + 2026.11 분기 재평가 #1 안건 등재가 전제. (영상·패키징·공개는 v1 에서 이미 제외되어 v2 로 이관됐으므로 절삭 대상이 아니다.)
- 가용 시간 (2026-06-17 갱신): 6월은 하루 5h(최소)-8h(최대) 확보. 평일 10일 → 잔여 50-80h. 주말은 가족 우선으로 비운다(계획 비포함, 초과 버퍼로만). 순서 4-5(56h)를 6월 내 완주하는 것을 기준선으로 한다.
- 7월은 전면 불확실하다: 하루 1-2h(퇴근 후)만 될 수도, 종일 8h 될 수도 있어 사전 배분하지 않는다. 7월에 계획을 의존하지 않는다 — v1 기술 코어(순서 5)는 6월 안에 끝나 있어야 한다.
- 7월 가용분 처리: 7월에 시간이 나면 순서 6(정독)을 선행한다. 안 나면 8월로 미뤄도 v1 기술 코어(순서 5)는 이미 6월에 끝나 있으므로 영향받지 않는다 — 순서 6 정독은 본래 8-9월 작업이다.
- 작업-시간 매칭: 연속 블록(통합·디버깅·dry-run)은 6월 종일 가용일에 우선 배치. 순서 5 측정 반복·quiz는 짧은 블록에서도 가능하므로 7월 단시간 가용일의 흡수처로 둔다.
- 8월 초 체크포인트: v1 기술 코어(순서 5) 확보 직후 8-12월 예산·배치를 확정한다 (순서 5 의 마지막 체크 항목). 입력: 7월 실가용 실적(사후 집계), v1 진행 결과. 1순위 검토 안건: Phase 4.5(v1.5) 의 9-10월 조기 진입. 순서 6 정독 잠정치(20-30h)도 여기서 갱신한다.
- RT-2 정독 후행: RT-2 는 비공개 모델로 순수 개념·블로그용이므로 정독 전량을 8-9월로 이동. 6월의 표적 skim 은 구현에 필수인 부분집합(action 표현 / unnorm_key / 입력 형식 / embodiment 가정)만 다룬다.
- git author 재작성(filter-repo) 결정 기한: 8월 초 체크포인트까지 실행 여부를 확정한다. v1 외부 공개는 v2 로 이관됐지만, 공개·fork 이후에는 불가하므로 첫 공개(v2)가 가까워지기 전에 결판내야 한다 — 미루지 말고 이 체크포인트에서 결정한다.

## 순서 1 — Step 0: 환경 구축 + 레포 청소 + 실측 (6월 2주차, 6h) — 완료

- [x] `SETUP.md` §2 사전 점검 체크리스트 통과 (계정 / 로컬 드라이버·디스크·ROS2 / 공통 도구)
- [x] `SETUP.md` §6 로컬 환경 세팅 — 공용 venv `.venv-vla` 생성은 `week8/PRACTICE.md` "환경 설정" 절의 명령 사용
- [x] `SETUP.md` §1.3 실측치 표 숙지 — 이번 실측이 검증할 추정치 (int4 약 7GB / 약 2-3 Hz)
- [x] git 커밋 신원 정리 — 작업 컨테이너에 `git config user.name` / `user.email` 을 GitHub 계정에 연결된 개인 이메일(또는 noreply)로 설정. 기존 커밋 히스토리의 author 재작성(`git filter-repo`) 여부는 v1 공개 전 결정 — 공개·fork 이후에는 불가
- [x] `predict_action` 호출 패턴 일괄 청소 (**순서 4 진입 전 필수**) — `grep -rn "predict_action(\*\*inputs" .` 으로 잔존 위치 확인 후 `input_ids` / `pixel_values` 만 전달하도록 통일. 근거: `predict_action` 은 빈 토큰(29871)을 input_ids 에만 덧붙이므로 processor 출력의 attention_mask 를 그대로 넘기면 eager attention 에서 off-by-one 으로 깨진다
- [x] `week6/PRACTICE.md` 실습 1 (첫 OpenVLA 4-bit inference) — OOM 없이 로드되는지
- [x] `week6/PRACTICE.md` 실습 2 (latency 측정 100회 + 통계 + 결과 저장) — mean/p95 와 `nvidia-smi` VRAM 기록
- [x] 실측치를 `SETUP.md` §1.3 추정치와 비교 — 추정 범위(±50%)를 벗어나면 §1.3 을 실측 기준으로 갱신 (실측 mean 300.3 ms / 3.33 Hz, ±50% 범위 내 — §1.3 에 실측 행 추가)

### 노트: 순서 1 핵심 결과

- int4 OpenVLA 7B 가 RTX 4070 12GB 에 OOM 없이 로드.
- latency 실측 (n=100): mean 300.3 ms / median 301.3 ms / std 3.8 ms / p95 304.8 ms → **3.33 Hz**. 외삽 추정(약 2-3 Hz)의 ±50% 범위 내. 단, `predict_action` 호출만 측정 — 이미지 전처리/ROS2 오버헤드 제외 (제어 루프 전체 주기는 순서 4 의 week11 dry-run 에서 확인).
- 결과 파일: `week6/openvla_latency_4070_int4.npy` (Phase 7 산출물 v3 의 비교 baseline).
- 수치 본체: `SETUP.md` §1.3.

## 순서 2 — 표적 skim (6월 2-3주차, 4h)

skim 목적은 구현에 필요한 사실 확인이지 정독이 아니다. 항목별로 답을 아래 노트 섹션에 적으면 끝.

- [x] 라이선스 확인: OpenVLA HF 모델 카드 — 코드 MIT / weights 는 Llama 2 license. v1 공개물(블로그·영상·레포)에서의 사용 조건 확인 (노트 작성 완료)
- [x] action 표현: `week5/README.md` §3 (Action space 표준화) + §3.5 (관절각/EE-delta/token 비교 축) — OpenVLA 출력 7-DoF 가 무엇을 의미하는지
- [x] unnorm_key / 입력 형식: OpenVLA HF 모델 카드의 사용 예시 코드 + `week6/README.md` "핵심 개념"의 모델 로드/추론 코드 절 — prompt 형식과 `predict_action(input_ids=..., pixel_values=..., unnorm_key=..., do_sample=False)` 호출. **주의**: processor 출력 전체를 `**inputs` 로 넘기는 패턴은 attention_mask 동봉으로 크래시한다 (순서 1 청소 항목의 근거 참조). 참고 자료에 옛 패턴이 남아 있으면 그 자리에서 수정
- [x] embodiment 가정: `week5/README.md` §1 (OpenX-Embodiment 구조) + §2 (대표 embodiment 특징) + 공식 repo README — 어떤 로봇/카메라 시점을 전제로 학습됐는지, sim 이 거기에 맞을 수 있는지
- [x] 아키텍처 최소 골격: `week4/README.md` §5 (Architecture Diagram) + "한 페이지 OpenVLA 요약"의 입출력 인터페이스 절

### 노트 작성 방법 (공통 틀)

아래 각 노트는 "정독 요약"이 아니라 "구현 때 안 막히기 위한 사실 확인(skim)"이다. 섹션마다 미리 적어둔 `파악할 항목`(질문)에 직접 답을 채우는 방식으로 학습한다.

작성 절차:
1. `목표` / `출처` 를 먼저 읽고 무엇을 확인하러 가는지 머리에 넣는다.
2. 출처를 읽으며 `파악할 항목` 의 질문에 하나씩 답한다 — 원문 복붙 금지, 내 문장으로.
3. 답은 `정리:` 아래에 항목당 1-2줄. 막히면 `(미해결: 무엇이 안 풀리는지)` 로 남기고 넘어간다.
4. 구현에 바로 쓰일 사실(인자 이름 / 형식 / 주의점)은 **굵게**.
5. `파악할 항목` 을 원문 없이 말할 수 있으면 그 노트 완료 -> 위 순서 2 체크박스 `[x]`.

`노트: 라이선스` 는 이 틀 이전에 이미 작성됨 — 형식이 달라도 그대로 둔다.

### 노트: 라이선스

- 코드(공식 repo, training codebase 포함): MIT. 복사·개작해 내 레포에 넣으면 MIT 저작권 고지 유지 필요.
- weights(openvla-7b, openvla-v01-7b): Llama-2 파생이므로 Llama 2 Community License 적용. HF 모델 카드의 license 태그는 MIT 로 되어 있으나 GitHub README 가 Llama 2 적용을 명시 — 보수적으로 후자를 따른다.
- v1 공개물 영향: 블로그·영상·레포 공개 모두 의무 없음. weights 를 레포에 포함하지 않고 HF 에서 로드만 하면 재배포에 해당하지 않는다. 출력물(추론 결과, latency 수치, sim 데모) 공개 제약 없음 — Llama 2 의 출력물 제한은 "다른 LLM 개선에 사용 금지"뿐.
- 경계: int4 양자화본을 파일로 저장해 레포나 HF 에 올리면 weights 파생물 재배포 — 라이선스 사본 동봉 + "Llama 2 is licensed under the LLAMA 2 Community License, Copyright (c) Meta Platforms, Inc." 고지 의무 발생. 현재 계획(로컬 로드·양자화, 결과만 공개)에서는 해당 없음.

### 노트: action 표현

> 목표: OpenVLA 출력 7-DoF 숫자 7개가 각각 무엇이고 어떤 형식인지 내 말로 설명한다.
> 출처: `week5/README.md` §3, §3.5 (+ `week4/README.md` §5)

파악할 항목:
[x] 7개 숫자가 각각 무엇을 가리키나? 어떤 묶음으로 나뉘나?
[x] 이 값은 관절각 기준인가, end-effector(손끝) 기준인가?
[x] 절대 위치/자세인가, 변화량(delta)인가? 그래서 매 step 어떻게 쓰이나?
[x] §3.5 의 표현 3단계(관절각 / EE-delta / action token) 중 OpenVLA 출력은 어디에 해당하나?
[x] 모델이 실제로 뱉는 형식은 연속 숫자인가 토큰인가? 둘 사이 변환은 어떻게 일어나나?

정리:
- 로봇마다 명령을 내리는 방법(Action)이 상이함
    - 동작 명령의 표준화 필요
- 입력 -> OpenVLA 표준화
    - VLA가 이미지 + 명령을 받아서 action을 출력하고 통일된 규격으로 일정 주기로 반복해서 로봇을 움직이도록 함
    - [dx, dy, dz, rx, ry, rz, gripper] 의미
        - dx: x축 방향 이동량
        - dy: y축 방향 이동량
        - dz: z축 방향 이동량
        - rx: roll 회전량
        - ry: pitch 회전량
        - rz: yaw 회전량
        - gripper: 0(닫힘), 1(열림)
    - 손끝 변화량(end-effector delta, EE-delta) : 팔의 맨 끝을 공간에서 어디로 옮길지만 지정함
        - 손끝(end-effector) 기준
            - 로봇 팔의 모터 개수에 따른 자유도와 상관없이 손끝 위치에 따라 명령이 가능
        - 자유도
            - 7 DoF: 위치 3 + 회전 3 + gripper 1
    - 정규화(normalization)
        - 정규화를 하는 이유
            - 정규화를 하지 않으면 모델을 학습할 때 특정 조건(ex. step 당 거리가 먼 경우)이 학습 loss에 대부분 반영되며, 다른 조건(ex. step 당 거리가 작은 경우)은 거의 반영되지 않음
                - 기록 주기(Hz)가 다름에 따라 동작을 기록하는 step 간 시간과 거리가 다름
                - 사용 용도에 따라 이동 범위가 다름
                    - 산업 용도: step 당 수십 cm
                    - 서비스 용도: step 당 수 cm
        - 정규화 방법
            - 데이터에서 구한 하위 1% 값 / 상위 1% 값 (99 분위수)를 -1 / +1에 대응시킴
                - python: norm = clip(2 * (x - q01) / (q99 - q01) - 1, -1, +1)
- OpenVLA -> 출력
    - 연속적인 EE-delta에서 출력된 각 차원의 값을 256으로 나눔
    - 각 차원의 값의 범위를 256으로 나눠 256개의 칸을 만들고 각 칸마다 Llama의 단어 사전에서 거의 사용하지 않는 단어와 매칭
    - 추론 때는 거꾸로 수행하여 복원함


### 노트: unnorm_key / 입력 형식

> 목표: `predict_action` 을 올바른 인자로 호출하는 법 + `unnorm_key` 가 출력의 무엇을 바꾸는지 안다.
> 출처: OpenVLA HF 모델 카드 사용 예시 + `week6/README.md` §5, §7

파악할 항목:
[x] prompt 문자열 형식은? instruction 을 어디에 끼워넣나?
[x] `processor` 출력 중 `predict_action` 에 넘기는 인자는 무엇인가? 반대로 넘기면 안 되는 것은 무엇이고 그 이유는? (순서 1 청소 항목의 근거와 연결)
[x] `unnorm_key` 는 무엇을 결정하나? 값을 바꾸면 출력의 무엇이 달라지나?
[x] 대표 key 예시는 각각 어느 embodiment 인가? 자작 6-DoF 팔에는 어떤 key 가 1순위 후보인가? (embodiment 가정 노트와 연결)
[x] `do_sample` 인자의 의미는? 왜 보통 `False` 인가?

정리:
- OpenVLA는 학습할 때 쓴 프롬프트 틀이 정해져있고, 추론 때도 그 틀을 그대로 맞춰야 함
    - 형식: "In: What action should the robot take to {명령}?\nOut:"
        - {명령}에 내가 원하는 영어 문구를 넣으면 됨
        - 이 형식은 OpenVLA 전용 문구이며, 다른 VLA 모델의 경우 전용 프롬프트를 사용해야 함 
- processor 출력 내용
    - OpenVLA의 processor는 PrismaticProcessor로, 텍스트 토크나이저(LLaMA 기반)와 이미지 프로세서(DINOv2 + SigLIP 듀얼 백본)을 묶은 통역사임
        - 호출하면 dict 형태(BatchFeature)를 반환
        - 핵심 키
            - input_ids : 프롬프트 문장을 토큰 ID 수열로 변환
            - attention_mask : 어느 토큰이 실제 입력인지(보통 전부 1)
            - pixel_values : 전처리된 이미지 텐서 (이미지를 모델 입력 규격에 맞춘 숫자로 변환)
- vla.predict_action
    - 이미지 + 명령 입력 -> 7차원 동작값 출력
    - 하는 일
        1) input_ids, pixel_values 입력
        2) input_ids 뒤에 빈 동작 토큰 자리 추가
        3) 디코더가 action 토큰 7개 생성
        4) 토큰 ID -> 정규화된 실수 (-1 ~ 1)로 변환
        5) unnorm_key 통계로 실제 단위 환산
        6) 7차원 동작값 출력
    - processor 출력 값 중 vla.predict_action에 넘기면 안되는 인자
        - attention_mask: input_ids와 같은 개수를 가지지만 vla.predict_action에서 빈 동작 토큰 자리를 추가해서 개수가 달라지게 되므로 입력하면 가리키는 데이터가 틀어지게 됨
    - unnorm_key
        - 위 순서 중 4)에서 정규화된 값을 원하는 로봇의 통계로 역정규화해 실제 물리로 환산하는데 사용됨
        - key가 맞지 않으면 역정규화는 정상적으로 진행되지만 실제 물리 값은 맞지 않게됨
        - 예) key가 "bridge_orig"면 WidowX 팔의 스케일로 환산됨
    - do_sample
        - LM(Language Model)에서 다음 토큰을 고를 때 확률분포에 따른 추첨을 할지 안할지 결정하는 인자
        - 인자 값에 따른 동작
            - True: 확률에 비례해 추첨해서 토큰을 선택, 출력이 일관적이지 않음
            - False: 추첨 없이 확률이 최댓값인 토큰을 선택, 출력이 일관적임


### 노트: embodiment 가정

> 목표: OpenVLA 가 어떤 로봇/관측을 전제로 학습됐는지 파악하고, 내 환경(sim/자작 팔)이 거기 맞는지 판단할 재료를 모은다. (embodiment 개념 자체는 `week5/README.md` §0 참조)
> 출처: `week5/README.md` §1, §2 + OpenVLA 공식 repo README

파악할 항목:
[x] observation 에 무엇이 들어가나? 이미지 해상도와 필수/선택 필드는? (§1 RLDS schema)
[x] 학습 데이터의 로봇은 주로 어떤 형태인가? 팔 위주인가, 이동 로봇도 포함되나?
[x] 카메라 시점 전제는? (3인칭 고정 / 손목 카메라 / depth 유무) — 공식 repo README 에서 확인
[x] 자작 6-DoF 팔과 가장 가까운 embodiment 는 무엇이고 근거는? (§2 표)

정리:
- OpenX-Embodiment: 60개의 dataset을 표준 schema로 통합
    - 표준 schema: RLDS(Reinforcement Learning Dataset Schema) 형식
        - steps: 한 작업을 구성하는 step들의 나열, step 하나는 그 순간 관측 + 그 순간 행동 + 부가 정보로 이루어짐
            - observation: 
                - image<RGB image>: 주로 224x224(OpenVLA), 256x256 사용 (사실상 필수)
                - natural_language_instruction<str>: 자연어로된 명령어(ex. "pick up the can") (사실상 필수)
                - wrist_image<RGB image>: 손목 장착 카메라 (선택)
                - depth<float32 or uint16>: 깊이 정보 (선택)
                - state<float32 [N]>: 로봇 자체 상태(ex. 관절각, EE 위치 등) (선택)
            - action<N-DoF(OpenVLA=7-DoF)>: 팔의 이동 좌표 
            - reward<float>: 이 step의 행동에 대한 평가, 대부분 - 0, 성공한 마지막 step - 1, 혹은 데이터셋에 따라 아예 의미 없음
            - is_first<bool>: 이 step이 episode의 첫 step인지 여부
            - is_last<bool>: 이 step이 episode의 마지막 step인지 여부
            - is_terminal<bool>: 이 step이 작업 종료 상태인지 여부
- 자작 6-DoF 팔과 가장 가까운 embodiment는 WidowX 250임
    - 이유: DoF 일치, 액추에이터 계열이 비슷, episode 개수가 상대적으로 많음

### 노트: 아키텍처 최소 골격

> 목표: 입력 -> ... -> 출력 데이터 흐름을 한 줄로 그리고, 입출력 인터페이스 스펙을 확정한다.
> 출처: `week4/README.md` §5 + "한 페이지 OpenVLA 요약"

파악할 항목:
[x] 입력 형식은? (이미지 해상도 + 그 외 무엇)
[x] 출력 형식은? (action 표현 노트의 결론과 일치하는지 교차 확인)
[x] RGB 입력부터 action 출력까지 거치는 블록을 순서대로 나열 (vision encoder -> ... -> LM -> ...)
[x] vision encoder 가 왜 둘(DINOv2 + SigLIP)인가? 각각의 역할을 한 단어로.
[x] 제어 주기: README 추정치는 얼마인가? 순서 1 실측치(3.33 Hz)와 비교하면? 어긋나면 어느 쪽을 기준으로 삼나?

정리:
- VLA Architecture
    1. RGB image 처리
        1) RGB image 입력 받음
        2) RGB image를 두 개의 비전 인코더에 전달
            - DINOv2(ViT-L/14): label 없이 self-supervised로 학습, 공간적/기하학적 구조 인지 -> 객체의 위치 및 경계 파악 가능
            - SigLIP(ViT-L/14): 이미지-텍스트 대조 학습, 의미적 정보를 얻을 수 있음
            - 두 인코더를 동시에 통과시켜 사용하는 경우 성능이 더 높아짐을 확인(in Prismatic VLM)
        3) 이미지 -> 토큰 변환
            - 두 인코더(DINOv2, SigLIP)가 만든 256개 패치 토큰을 같은 패치 위치에서 두 인코더의 특징 벡터를 이어 붙임 -> concat(이어붙이기)
        4) Projection (비전 토큰 -> Llama의 임베딩 공간으로 사영)
    2. 자연어 명령 처리
        - 수신된 자연어 명령을 Llama의 일반 텍스트 토크나이저로 토큰화
    3. Llama 시퀀스 진행
        1) 전처리된 RGB image + 자연어 명령 입력 받음
        2) action token 생성
            - 로봇 행동의 각 연속값을 256단계로 이산화
            - Llama에서 가장 사용 빈도가 낮은 단어 256개의 토큰에 행동 ID를 매핑하여 재활용
            - VLM의 구조를 전혀 바꾸지 않고 행동 예측기로 사용 가능하며, 학습도 일반 언어 모델과 동일한 cross-entropy loss로 진행 가능
        3) 7-DoF의 action 생성
            - action token 내 매핑된 단어 토큰을 256구간 인덱스로 변환하여 변화량을 계산
            - 손끝 위치 변화량 (3 DoF), 회전 변화량 (3 DoF), 그리퍼 open/close -> 7 DoF
            - 출력된 값은 절대값이 아닌 변화량임
- 제어 주기
    - README 추정치: 5-7 Hz (100-150 ms) — RTX 4090 fp16 기준을 4070 4-bit 에 외삽한 낙관치
    - 순서 1 실측치: 3.33 Hz (mean 300 ms, 4070+int4) — README 추정과 약 2배 어긋남
    - 기준: 타깃 환경(4070+int4) 실측을 기준으로 삼음 (추정은 다른 GPU/정밀도 외삽값, 실측이 우선). SETUP.md §1.3 / 순서 4 노트에서 이미 실측 기준 확정

## 순서 3 — sim 정합 + 성공 task 정의 (6월 2-3주차, 12h)

이 구간은 대응하는 week 자료가 없는 신규 작업이다 (원안에서 week11 에 묻혀 있던 선행 의사결정을 분리한 것).

- [x] 순서 2 의 embodiment 가정에 맞는 sim 후보 비교·선정 — 선정 사유를 아래 노트에 기록
- [x] 성공 task 1종 + 성공률 기준 N 정의
- [x] task 의 제어 주기 요구 확정 → `week8/PRACTICE.md` 실습 체크리스트의 latency placeholder ("2 Hz 이상") 를 확정 수치로 교체
- [x] `week11/README.md` §4 (1분 dry-run 의 success criteria) 미리 읽기 — 순서 4 의 종착점 파악

### 노트: sim 후보 비교·선정 사유

선정: **ManiSkill** (SAPIEN 기반). 사유는 비교표 아래에 정리.

이 실습의 sim 역할은 환경 제작이 아니라 "VLA 추론에 이미지를 공급하고 action 을 받아 실행하는 closed-loop 테스트 무대"다 (학습 아님, 추론 통합 + latency/성공률 측정이 목적). 따라서 평가 기준은 렌더링 품질이 아니라 (1) embodiment/action 정합, (2) 이미지 획득 난이도, (3) 기성 task 제공, (4) 4070(12GB) GPU 부담, (5) VLA 통합 노력이다.

| 후보 | embodiment/action 정합 | 이미지 획득 | 기성 task | GPU 부담(4070 12GB) | 통합 노력 |
|------|----------------------|-----------|----------|--------------------|----------|
| ManiSkill | 팔(Franka) 위주, EEF delta 제어모드로 OpenVLA 7-DoF 출력과 매핑 가능 | obs dict 에 자동 포함 | 내장(PickCube 등) | 헤드리스 렌더 가능, 비교적 가벼움 | 가장 적음 |
| SAPIEN | ManiSkill 과 동일 엔진이나 task 직접 구성 | 카메라 수동 추가 후 take_picture | 없음(직접) | ManiSkill 과 동급 | 중간 |
| MuJoCo | 팔 모델 가능하나 씬/카메라 직접 구성 | XML 카메라 + Renderer 수동 | 없음(직접) | 가벼움 | 중간 |
| PyBullet | 팔 모델 가능하나 모든 것 직접 | view/proj 행렬 직접 계산 | 없음(직접) | 가벼움 | 가장 많음 |
| Isaac Sim | 팔 다수 제공, 렌더 품질 최고 | 카메라 + ROS2 bridge 공식 지원 | 일부 제공 | 매우 무거움, 7B 추론과 VRAM 경쟁 위험 | 무거운 셋업 |

선정 사유 (ManiSkill):

- **통합 노력 최소**: 카메라와 task 가 이미 세팅된 채로 나온다. `obs_mode="rgb"` 만 지정하면 관측 dict 에 카메라 이미지가 들어오므로, 순서 4 에서 "이미지 들어가고 action 나오는지"를 가장 빨리 검증할 수 있다.
- **action 정합**: Franka 의 `pd_ee_delta_pose` 제어 모드가 OpenVLA 출력(7-DoF EEF delta + gripper)과 직접 매핑 가능하다. 별도 변환 레이어 부담이 작다. (정확한 단위/축 정합은 순서 4 통합 시 1회 검증 필요)
- **GPU 부담**: 헤드리스 GPU 렌더로 4070 에서 OpenVLA int4 추론과 병행 여지가 있다. Isaac Sim 은 동일 GPU 에서 7B 추론과 VRAM 을 다투어 4070 환경에서 위험이 크다 — 선정 제외의 핵심 사유.
- **task 매핑**: 기성 PickCube 등이 순서 3 의 "성공 task 1종 정의"에 그대로 매핑된다.
- **설치**: pip 설치, Linux 친화적. Isaac Sim(Omniverse) 의 무거운 셋업과 대비된다.

한계/리스크 (선정했더라도 감수하는 부분):

- **도메인 갭**: OpenVLA 는 실로봇 데이터(OpenX-Embodiment)로 학습됐다. ManiSkill 렌더 이미지는 도메인이 달라 zero-shot 성공률이 낮을 수 있다 — 측정이 목적이므로 낮은 수치도 결과로 수용한다 (순서 5).
- **embodiment 부분 불일치**: 팔 형태는 맞지만 카메라 시점/로봇 외형이 OpenVLA 학습 분포와 정확히 같지 않다. 순서 2 의 embodiment 가정과 ManiSkill 기본 카메라 시점이 어긋나면 성공률에 추가 악영향.
- **ROS2 연계 직접 구현**: Isaac Sim 과 달리 ROS2 bridge 가 기본 제공되지 않는다. sim 이미지를 `sensor_msgs/Image` 로 publish 하는 래핑을 week9/week10 에서 직접 만들어야 한다 — 단 이 실습의 학습 목표 자체가 그 노드 구현이므로 비용이 아니라 과제에 가깝다.

### 노트: 성공 task 정의 + 성공률 기준 N

선정 task: **PickCube**(ManiSkill 내장, Franka Panda + `pd_ee_delta_pose` 제어). 사유는 sim 선정 노트와 동일 맥락 — (1) Roadmap 의 task 예시 "pick-and-place"에 직접 대응, (2) ManiSkill 기성 task 라 환경/카메라/성공 판정이 이미 구현돼 통합 노력이 가장 작음, (3) OpenVLA 학습 분포(테이블탑 단일 물체 집기)와 형태가 가깝다. instruction 은 OpenVLA prompt 틀에 맞춰 `"pick up the cube"` 영어 단문으로 고정.

1회 시도(episode)의 성공 정의:

- ManiSkill 이 step 마다 반환하는 내장 `success` 플래그를 그대로 채택한다 — 자체 임계값을 새로 만들지 않는다.
- PickCube 의 success 는 대략 "큐브가 목표 위치 반경 내(기본 약 2.5cm)에 들어오고 로봇이 정지 상태"로 정의된다. **정확한 임계값/축은 설치된 ManiSkill 버전에서 1회 확인**(순서 4 통합 시) — 버전마다 다를 수 있어 단정하지 않는다.
- 시도 종료 조건: `success == True`(성공) 또는 step cap 도달(미달 = 실패). step cap 은 잠정 100 step, 통합 시 episode 가 실제로 도는 것을 보고 확정한다.
- 주의: sim 은 action 이 올 때까지 대기하므로 추론 latency(실측 3.33 Hz)가 sim 안에서는 성공/실패를 직접 깨뜨리지 않는다. latency 의 task 적합성 판정은 실시간 제어 요구를 따지는 다음 체크박스(제어 주기 확정)에서 별도로 다룬다 — week11 §4 의 dry-run success criteria 는 task 성공률이 아니라 노드 안정성(0 fail / latency 한계) 기준이므로 혼동하지 않는다.

성공률 기준 N:

- 매 시도마다 seed 를 바꿔 큐브 초기 pose 와 목표 위치를 무작위화한다 (동일 조건 반복은 성공률로서 의미가 없다).
- 기록값 = 성공 횟수 / N. 임계 통과/실패가 아니라 **수치 자체가 산출물**이다 — 도메인 갭으로 낮게 나와도 결과로 수용한다 (sim 선정 노트의 한계 항목과 일치).
- N 기준: 베이스라인 **20**, 예산 절삭 시 하한 **10** (진행 원칙의 절삭 순서 2 = "성공률 측정 N 축소"에 해당).
- 한계(명시): N=20 에서 측정 성공률의 신뢰구간은 매우 넓다 (50% 부근에서 약 ±22%p). 이 수치는 통계적 추정이 아니라 "v1 데모가 단일 task 루프를 닫는다"를 보이는 baseline 으로만 쓴다. 정밀 성공률은 v1.5 eval harness(Phase 4.5)에서 N 을 키워 재측정한다.

### 노트: 제어 주기 요구 확정

- task = PickCube 는 quasi-static pick-and-place 라 하드 실시간 deadline 이 없다. 게다가 sim 은 action 이 올 때까지 대기하므로 **제어 주기는 sim 성공률에 영향을 주지 않는다**. 따라서 여기서 정하는 제어 주기 기준은 sim 성공 게이트가 아니라 "실로봇에 옮겼을 때도 말이 되는가"를 보이는 실로봇 plausibility/어필용 하한이다.
- 확정 수치: **전체 제어 루프 2 Hz 이상(step 500 ms 이하)**. 이는 placeholder 를 그대로 둔 게 아니라, task 가 더 빡센 요구를 부과하지 않으니 하한을 결정하는 건 측정된 파이프라인이라는 판단의 결과다. predict_action 실측 300 ms(3.33 Hz, 순서 1)가 baseline 이고, 그 위에 이미지 전처리 + ROS2 publish/subscribe 오버헤드로 약 200 ms headroom 을 두면 500 ms 다.
- week8 실습 4 stress test(int4, 4070, N=100): `predict()` 전체(입력검증 + processor 전처리 + predict_action) mean 298.8 ms / p95 305.0 ms, 100/100 성공. 이 값이 순서 1 의 predict_action 단독(mean 300.3 / p95 304.8 ms)과 사실상 동일하다는 점이 핵심이다 — **processor 전처리 오버헤드는 무시 수준**임이 실측으로 확인됐다. 따라서 위 200 ms headroom 은 전처리가 아니라 거의 전부 카메라 캡처 + ROS2 통신 몫이며, 전체 루프가 500 ms 안에 들어오는지는 그 통신 비용에 달려 있다. 또 p95 가 mean 대비 +6 ms 에 그쳐 지연 꼬리(tail)가 거의 없다 = 실시간 제어 관점에서 안정적.
- 전체 루프 주기(전처리 + 통신 포함) 실측은 순서 4 의 week11 dry-run 에서 한다 — 순서 1·week8 실측은 모두 카메라/ROS2 를 제외한 추론 경로 단독이라 전체 주기가 아니다. 만약 전체 루프가 500 ms 를 넘겨 2 Hz 밑으로 떨어져도 sim 성공률은 영향받지 않으나, 실로봇 일반화 어필이 약해진다. 그때는 placeholder 가 아니라 실측 기반으로 기준을 재조정한다.
- placeholder 교체 완료: `week8/PRACTICE.md` 실습 체크리스트의 "2 Hz 이상" 문구를 위 확정 기준 + 근거로 교체.

### 노트: week11 §4 dry-run success criteria (순서 4 종착점)

순서 4 의 종착점은 week11 실습 3 의 1분 dry-run 이다. 자료(`week11/README.md` §4)가 제시하는 판정 표:

| 지표 | 자료 목표 |
|---|---|
| total inference 수 | 60s × 5 Hz = 300-350 frame |
| Success rate | 100% (0 fail) |
| Mean latency | < 200 ms |
| p95 latency | < 300 ms |
| GPU memory | < 10 GB |
| 노드 안 죽음 | True |

해석 (구현 때 안 헷갈리게):

- 이 표는 task 성공률이 아니라 **노드 안정성 + 처리량** 기준이다 — "1분간 노드가 안 죽고 일정 주기로 추론을 계속 돈다"를 확인하는 것이지, 큐브를 집었는지(= 성공 task 정의 노트의 `success` 플래그)와는 다른 축이다. 둘을 섞지 않는다.
- 자료의 `mean < 200 ms` / `5 Hz` / `p95 < 300 ms` 는 원 강의의 낙관치로, 순서 1 실측(predict_action mean 300.3 ms / p95 304.8 ms / 3.33 Hz)과 어긋난다. int4 + 4070 환경에서 `mean < 200 ms` 와 `5 Hz` 는 **달성 불가**다(전체 루프는 predict_action 보다 더 느림). 따라서 dry-run 판정은 자료 수치 그대로가 아니라 실측 baseline 기준(mean 약 300 ms, 처리량 약 3 Hz, 전체 루프는 그 이하)으로 재조정해 읽는다.
- 그대로 유효한 기준: `0 fail` / `노드 안 죽음` / `GPU < 10 GB`(int4 약 7 GB 로 충족). 이 셋은 환경과 무관하게 dry-run 통과 조건으로 쓴다.

## 순서 4 — week8-12 실습 압축 (6/17 - 6/26 목표(5h 추세 시 6/29), 34h)

각 week 공통 패턴: `README.md` 정독 → `PRACTICE.md` 실습 → `quiz_easy.py` / `quiz_medium.py` (출장일 저녁 등 파편 시간에 배치 가능). 진입 전제: 순서 1 의 `predict_action` 패턴 청소 완료.

- [x] `week8/README.md` + `week8/PRACTICE.md` 실습 1-4 (VLAInference class / image preprocess / exceptions·config / 100회 stress test) + quiz
- [x] `week9/README.md` + `week9/PRACTICE.md` 실습 1-3 (I/O spec 1페이지 / msg <-> Python 변환 / BGR->RGB 검증) + quiz
- [x] `week10/README.md` + `week10/PRACTICE.md` 실습 1-4 (vla_node 패키지 생성 / 골격 노드 / setup.py / 빌드+실행, 실습 5 dummy image 는 선택) + quiz
- [x] `week11/README.md` + `week11/PRACTICE.md` 실습 1-3 (실 inference 통합 / 빌드+실행 / 1분 dry-run + 통계) — 입력은 순서 3 에서 선정한 sim 으로 연결 (자료의 ros2 bag 재생은 대체 수단) + quiz
- [ ] `week12/README.md` + `week12/PRACTICE.md` 실습 1-2 (Rerun 기본 / rerun_logger 노드) — **레포 결과 기록용 시각화로만 선택 수행** (스크린샷/짧은 gif 1장이면 충분). 1분 영상 제작(실습 3-5)은 v1 에서 제외, v2 로 이관 + quiz

## 순서 5 — 성공률 측정 + 결과 정리 (6월 5주차 목표 6/30(5h 추세 시 7월 첫 주 일부), 22h)

- [ ] 정의된 task 를 N회 시도, 성공률 기록 — 측정 루프는 신규 작성, 통계 출력부만 `week11/PRACTICE.md` 실습 3 재사용
  - [ ] 설치된 ManiSkill 에서 PickCube `success` 플래그의 임계값/축 1회 확인 (위 "성공 task 정의" 노트 = 반경 약 2.5cm + 정지, 버전마다 다를 수 있음)
  - [ ] step cap 확정 — 잠정 100 step, 실제 episode 가 도는 것을 보고 결정
  - [ ] N episode 측정 루프 작성: 매 시도 seed 변경 → `env.reset` → vla action 으로 step → `success==True` 또는 step cap 까지 반복
  - [ ] 통계/출력 골격은 실습 3 의 `np.array` -> mean/p95 부분 재사용, 성공 카운트 로직은 신규 (실습 3 은 latency 분석이라 성공률 코드는 없음)
  - [ ] 기록값 = 성공 횟수 / N 산출 (N=20 베이스라인, 예산 절삭 시 하한 10). 임계 통과가 아니라 수치 자체가 산출물
- [ ] mean/p95 latency 실측 + task 제어 주기 충족 여부 판정 (`week8/PRACTICE.md` 실습 체크리스트의 확정 기준)
  - [ ] 순서 4 week11 dry-run 이 산출한 mean/p95/throughput 수치 가져오기 (순서 5 에서 새로 재지 않음)
  - [ ] 전체 제어 루프 2 Hz 이상(step 500 ms 이하) 충족 여부 판정 (위 "제어 주기 요구 확정" 노트)
  - [ ] dry-run 환경 무관 통과 조건 확인: `0 fail` / 노드 안 죽음 / GPU < 10 GB
  - [ ] 500 ms 초과로 2 Hz 밑이면 placeholder 가 아니라 실측 기반으로 기준 재조정 + 사유 메모
- [ ] 결과 표 정리 — `week12/PRACTICE.md` 실습 5 의 "측정 결과" 표 형식 참고. 여기까지가 v1 기술 코어 (6/30 목표)
  - [ ] week12 표 형식(Mean / p95 / Throughput / GPU Memory)에 성공률 행 추가해 작성
  - [ ] 한계 명시: N=20 신뢰구간이 넓음(50% 부근 약 ±22%p), baseline 용도로만 사용 — 정밀치는 Phase 4.5 eval harness 에서 재측정
- [ ] **8월 초 체크포인트 수행** — "진행 원칙"의 입력으로 8-12월 예산·배치 확정 (순서 6-7 잠정치 갱신, v1.5 조기 진입 여부 결정)
  - [ ] 6-7월 실제 진척 속도(시간/일 추세) vs 계획 대조
  - [ ] 8-12월 가용 예산 재산정 → 순서 6-7 잠정치 갱신
  - [ ] v1.5(Phase 4.5) 조기 진입 여부 결정

## 순서 6 — week1-7 정독 (블로그 작성 제외) (8-9월)

정독(아키텍처 이해)만 v1 범위다. 블로그 작성(`week3`/`week7`)은 v2(Phase 6)로 이관 — v2 에서 RT-2/OpenVLA 배경을 v2 공개 글에 녹여 쓴다.

- [ ] `week1/` RT-2 1회독 + reading note (PRACTICE 실습 1-3) + quiz
- [ ] `week2/` Co-fine-tuning + action tokenization (PRACTICE 실습 1-3) + quiz
- [ ] `week4/` OpenVLA 정독 — 순서 2 에서 안 본 부분 중심 (hybrid vision encoder, contribution, 한계) + 실습 1-3 + quiz
- [ ] `week5/` OpenX-Embodiment + LoRA 흐름 정독 + 실습 1-4 + quiz
- [ ] `week6/` README 개념 보충만 — 실습 1-2 는 순서 1 에서 이미 수행, 실습 3 (에러 기록) 은 미기록분 보완
- (이관) `week3/` RT-2 블로그 / `week7/` OpenVLA 블로그 작성 → v2

## 순서 7 — v2(Phase 6)로 이관

원안의 순서 7(블로그 2편 퇴고 + 패키징 + 1분 영상 + v1 외부 공개, `week13`-`week16` + `week12` 실습 3-5)는 **v1 에서 제외하고 v2 로 이관**했다. 사유: v1(sim zero-shot)은 공개 산출물로 약해, 다듬은 영상·블로그·패키징·LinkedIn 공개에 50-70h 를 쓰는 것은 오배치다. 공개 푸시는 실제 팔이 결합되는 v2 가 훨씬 강하므로 그쪽으로 통합한다.

v1 의 공개물 대체: 레포에 결과 기록만 남긴다 — 순서 5 의 README + 성공률 표 + latency 수치 (+ 순서 4 week12 의 Rerun 스크린샷/짧은 gif 선택). 외부 공개(velog/LinkedIn) 없음.

`week3`,`week7`,`week12`(실습 3-5),`week13`-`week16` 자료는 삭제하지 않고 v2 의 패키징·공개 단계 참조용으로 보존한다 (각 README 상단에 v2 이관 배너).
