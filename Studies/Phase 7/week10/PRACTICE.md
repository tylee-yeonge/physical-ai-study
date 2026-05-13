# Week 10 실습: 통합 영상 마감


```bash
# Storyboard 작성
cat > ~/phase7_notes/week10/storyboard.md
# 영상 capture (각 segment)
# OBS / Kazam 으로 화면 녹화
# 또는 카메라 + Rerun 화면 합쳐서


# Shotcut 편집
shotcut


# Export H.264 1920x1080 30fps
ffmpeg -i edited.mp4 -c:v libx264 -preset slow -crf 23 \
    -c:a aac -b:a 128k -movflags +faststart \
    r2s2r_demo.mp4
```


체크리스트:
- [ ] Storyboard 작성
- [ ] 영상 capture
- [ ] 자막 추가
- [ ] 인트로/아웃트로
- [ ] Final export < 100 MB
