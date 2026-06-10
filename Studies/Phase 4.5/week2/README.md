# Week 2 — OpenVLA 학습 포맷으로 데이터 변환


> **목표**: week1 의 수집 데이터를 OpenVLA 가 LoRA 학습에 받아들이는 포맷으로 변환한다.
> **선행**: week1 데이터셋.


## 학습 순서

1. OpenVLA 모델 카드 / upstream 리포의 학습 데이터 스키마 확인 (RLDS 등)
2. 관측(image) + instruction + action 페어링 규칙 정리
3. action representation 정합 (관절각 / EE-delta / token — v1 에서 정리한 축 재사용)
4. 변환 스크립트 작성 + 소량 샘플로 로드 검증


## 핵심 개념

- **데이터 포맷팅이 adaptation 의 절반**: 모델이 기대하는 입출력 스키마와 1바이트라도 어긋나면 학습이 무의미해진다.
- **action representation 일관성**: 수집 시 action 표현과 OpenVLA 가 출력하는 표현이 같은 좌표/단위인지 확인 (v1 의 제어 인터페이스 매핑과 연결).


## 이번 주 산출 (must / nice)

- must: 변환 스크립트 + OpenVLA 포맷 데이터셋 + 로드 검증 통과
- nice: 포맷 변환 과정의 함정 메모 (블로그 소재)


> PRACTICE/quiz 는 진입 시점에 작성한다 (스캐폴드).
