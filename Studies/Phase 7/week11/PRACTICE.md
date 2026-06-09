# Week 11 실습: 패키징 + README


```bash
# 디렉토리 구조 생성
mkdir -p Portfolio/03_VLA_v3/{src,data,scripts,notebooks}


# 코드 복사
cp -r ~/ros2_ws/src/vla_node Portfolio/03_VLA_v3/src/
cp -r ~/ros2_ws/src/safety_node Portfolio/03_VLA_v3/src/
...


# README 작성 (Phase 4 week 14 패턴)
$EDITOR Portfolio/03_VLA_v3/README.md


# Quickstart 스크립트
cat > Portfolio/03_VLA_v3/scripts/setup.sh << EOF
#!/bin/bash
set -e
echo "VLA R2S2R Quickstart"
# ...
EOF
chmod +x Portfolio/03_VLA_v3/scripts/*.sh


# Clean 환경 검증 (Docker 또는 새 conda)
```


체크리스트:
- [ ] 패키지 구조 정리
- [ ] README 10 section 완성
- [ ] Quickstart 스크립트
- [ ] Clean 환경 동작 검증
