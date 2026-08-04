# Evidence Index

> 이 리포의 실측·엔지니어링 증거 목록과 각 항목이 입증하는 역량 매핑. 새 실험을 마칠 때마다 한 줄 추가한다 (워크플로우: [`Measurements/README.md`](../Measurements/README.md) §5).

| 증거 | 날짜 | 위치 | 입증 역량 |
|---|---|---|---|
| OpenVLA 7B int4 latency 실측 (RTX 4070) | 2026-06 (재측정 2026-08 예정) | [`Measurements/openvla-rtx4070-int4/`](../Measurements/openvla-rtx4070-int4/) | 양자화 배포 실측·컴퓨트 의사결정 (셋째 층) |
| Phase 3 perception 통합 노드 (supporting) | 2026-06 | [`Studies/Phase 3/`](../Studies/Phase%203/) week8 | TensorRT + ROS2 통합 (VLA wrapper 스캐폴드) |
| OpenVLA zero-shot baseline (ManiSkill PickCube-v1) | 2026-08 | [`Measurements/openvla-maniskill-zeroshot/`](../Measurements/openvla-maniskill-zeroshot/) | 측정 설계 — 상한/하한 대조로 0% 를 해석 가능한 값으로 만들고, 무증상 배선 오류를 코드 근거로 잡아낸 기록 |
