# Week 9 실습


```bash
# Phase 6 의 측정 script 재실행 (LoRA fine-tune 후)
python latency_bench.py
python repeatability_bench.py
python torque_compare.py
python image_gap_measure.py


# 결과 통합
python create_report.py > sim_real_gap_report_phase7.md
```


체크리스트:
- [ ] 4 gap 재측정
- [ ] 보고서 작성
- [ ] Phase 7 #4 영상에 들어갈 표 마감
