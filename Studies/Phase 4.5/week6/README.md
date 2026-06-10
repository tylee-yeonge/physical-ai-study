# Week 6 — before/after 분석 + 블로그 + v1.5 공개


> **목표**: week5 결과를 분석해 adaptation **설계-실행-분석** 서사로 블로그 1편을 쓰고 v1.5 를 공개한다.
> **선행**: week5 eval 결과(성공률+분산).


## 학습 순서

1. before/after 정량 분석: 차이가 분산 범위 안인지 밖인지 판정
2. (상승) 무엇이 올렸나 / (정체·하락) 왜 안 올랐나 — 데이터 규모·분포·학습 설정으로 원인 분석
3. 블로그 1편 작성: adaptation 파이프라인(데이터→학습→머지→eval) + 결과 + 해석
4. LoRA 파이프라인 + eval harness 코드 정리 + README + velog/LinkedIn 공개


## 핵심 개념

- **negative 결과도 산출물**: "성공률이 안 올랐다"는 실패가 아니라, 소규모 sim LoRA 의 한계를 정량적으로 보인 분석. 둘째 층 역량은 **분석의 질**로 증명된다.
- **sim 증거의 한계 명시**: sim/합성 데이터 adaptation 은 real 도메인과 채용 측 설득력이 다름 — 본문에 정직히 적고, real 확장(Phase 7)을 예고.


## 이번 주 산출 (must / nice)

- must: 블로그 1편 + 정리된 코드/README + 공개 + 성공률·분산 표
- nice: "consumer GPU 로 VLA adaptation" 각도의 검색 친화 글감 분리


## v1.5 공개 체크 (Roadmap 완료 체크리스트와 동기화)

- [ ] LoRA 파이프라인 + eval harness 코드 정리 + README
- [ ] 블로그 1편 (설계-실행-분석 서사) + velog/LinkedIn
- [ ] before/after 를 N·분산과 함께 보고
- [ ] negative 대응 논지 포함


> PRACTICE/quiz 는 진입 시점에 작성한다 (스캐폴드).
