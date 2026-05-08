# Phase 4: 3D Perception — 학습 노트 가이드

> 이 디렉토리는 **학습 노트 (private)** 다. 면접관이 보지 않는다.
> 채용 담당자가 보는 **포트폴리오 (public)** 는 별도 GitHub Repo `robotics-perception-portfolio` 에 따로 만든다.

## 분리 원칙

| 구분 | 위치 | 대상 | 형식 |
|---|---|---|---|
| **학습 노트 (private)** | 이 디렉토리 (`Studies/Phase 4/`) | 자기 이해용 | 빠르고 거친 형태 OK |
| **포트폴리오 (public)** | 별도 Repo `robotics-perception-portfolio` | 채용 담당자 | README + 영상 + 수치 |

## 각 week PRACTICE.md 진행 시 의식할 것

### "이 주의 공개 가능 결과물"
매주 끝에 산출물 #2 (3D Detection) 디렉토리에 누적:
- KITTI 3D bbox 시각화 이미지
- AP3D 측정 결과
- 학습 곡선
- 실패 사례 분석 노트

### "학습 노트는 면접관이 보지 않는다"
`Studies/Phase 4/` 노트는 본인 이해용. GitHub Public Repo와 분리.

## week11-12 역할 재정의

기존 "포트폴리오 마무리" 였던 week11-12를 **산출물 #2 (3D Detection) 공개까지** 로 축소.

- **week11**: 코드/문서 정리 → 별도 Public Repo 에 산출물 #2 디렉토리 추가
- **week12**: 1분 데모 영상 + 블로그 1개 → 산출물 #2 공개 완료

면접용 통합 패키징 · 이력서 다듬기 · AMR 연결 영상은 별도 **Portfolio Sprint (2026.10-11)** 에서 처리. Phase 4 안에서 다 끝내려 하지 않는다.

## 산출물 #2 최종 공개 시점

**2026.09 (Phase 4 완료)** 까지 별도 Public Repo `robotics-perception-portfolio` 에 산출물 #2 디렉토리 공개:
- KITTI 3D bbox 시각화
- AP3D 측정 표
- 시연 영상 1분
- 블로그 1개

## nuScenes 풀 학습 보류 원칙

Full dataset (~400GB) 은 셋업 비용이 과다하다. **현 시점에는 Mini dataset (10GB) 만 사용**, 풀 학습은 Stage 2 (이직 후) 로 보류 가능. KITTI 3D 데모만으로도 채용 라인 충분.

상세는 [Roadmap/Phase 4.md](../../Roadmap/Phase%204.md) 참조.
