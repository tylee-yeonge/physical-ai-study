# Hardware-Arm Stage 2 - 가이드


> **기간**: 2027.02~04 (3개월)
> **목표**: 6DOF 확장 + teleop + 안전 인터록 + Sim 물리 파라미터 매칭
> **약 100~150만원 추가**


---


## Stage 2 일정


```
2027.02: XM430 추가 + 6DOF + URDF 갱신
2027.03: Teleop + 데이터 수집 + 카메라-팔 캘리브
2027.04: 안전 인터록 (C++) + Sim 물리 파라미터 매칭
        -> Phase 6 / 7 의 하드웨어 기반
```


---


## 학습 파일


| 파일 | 내용 |
|---|---|
| [stage2_6dof_extension.md](stage2_6dof_extension.md) | 6DOF 확장 + URDF |
| [teleop_setup.md](teleop_setup.md) | leader-follower / PS4 |
| [safety_interlock.md](safety_interlock.md) | C++ 안전 인터록 |
| [sim_real_param_match.md](sim_real_param_match.md) | Sim 물리 매칭 |


---


## Phase 6, 7 와의 연결


```
Stage 2 (2027.02~04) Phase 6 (2027.02~05) 와 병행:
- Stage 2 URDF 갱신 -> Phase 6 의 Isaac Sim 임포트
- Stage 2 안전 인터록 -> Phase 7 의 안전 노드
- Stage 2 teleop 데이터 -> Phase 7 의 LoRA fine-tune
```


---


## 완료 체크리스트 (Stage 2)


- [ ] 6DOF 조립 동작
- [ ] URDF 갱신 + RViz
- [ ] teleop 동작
- [ ] teleop 데이터 수집 파이프라인
- [ ] 카메라-팔 base 캘리브
- [ ] 안전 인터록 (C++)
- [ ] Sim 물리 매칭
