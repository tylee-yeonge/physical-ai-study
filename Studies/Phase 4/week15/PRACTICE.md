# Week 15 실습: 영상 최종 마감 + 자막 + thumbnail + GIF


> **실습 목표**: Portfolio/01_VLA_v1/ 의 모든 영상 파일 완성.
> **예상 시간**: 4-6시간


---


## 환경 설정


```bash
# 영상 편집
sudo apt install shotcut


# GIF / thumbnail
sudo apt install ffmpeg gimp
```


---


## 실습 1: 영상 재검토 + 부족 부분 list


**파일명**: `~/phase4_notes/week15/video_review.md`


```markdown
# week 12 영상 재검토


## 검토 항목
- [ ] 인트로 (현재 ___ 초, 권장 3~5초)
- [ ] 본문의 흐름이 자연스러운가
- [ ] Rerun 화면이 잘 보이는가 (UI 깔끔)
- [ ] 모든 자막 가독성
- [ ] 음악이 너무 크거나 작지 않은가
- [ ] 아웃트로 (현재 ___ 초)
- [ ] 총 길이 (목표 60~80초)


## 부족 부분
1. ___
2. ___
3. ___
```


---


## 실습 2: 자막 보강


Shotcut 의 Filters > Text: Simple 또는 Filters > Subtitles:


### 자막 시나리오 (재확인)


```
0:00 -> 0:05 | (인트로) "Phase 4 산출물 v1: OpenVLA -> ROS2"
0:05 -> 0:10 | "AMR ROS 양산 SW 엔지니어 실무 5년"
0:10 -> 0:15 | "OpenVLA HuggingFace inference"
0:15 -> 0:25 | "ROS2 Humble + vla_node 패키지"
0:25 -> 0:40 | "실시간 동작 (Rerun)"
0:40 -> 0:50 | "RTX 4070 4-bit nf4 / mean 165ms / 6Hz"
0:50 -> 0:55 | "양산 의미: hierarchical 구조 필수"
0:55 -> 0:60 | "다음: Phase 7 Real-to-Sim-to-Real (2027.08~)"
```


각 자막:
- 폰트: Pretendard / Apple SD Gothic (한국어 가독성)
- 크기: 24-32 pt
- 색: 흰색 + 검은 outline
- 위치: 화면 하단 중앙


---


## 실습 3: 인트로 / 아웃트로 카드


### 인트로 카드 (3-5초)


```
[배경: 진한 색 단색 또는 코드 배경]


큰 글씨 (가운데):
  OpenVLA -> ROS2
  Minimal Demo


작은 글씨 (위):
  Phase 4 산출물 v1 / 2026 하반기


작은 글씨 (아래):
  AMR ROS Production SW + Physical AI Integration
  AMR ROS SW Engineer (since 2021.06)
```


### 아웃트로 카드 (3-5초)


```
[배경: 같은 색]


큰 글씨 (가운데):
  다음 산출물:
  Real-to-Sim-to-Real
  (Phase 7, 2027.08~)


작은 글씨 (아래):
  github.com/<you>/physical-ai-study
  linkedin: <your-linkedin>
```


---


## 실습 4: Thumbnail 제작


**파일명**: `Portfolio/01_VLA_v1/thumbnail.png`


도구: GIMP / Canva / PowerPoint


```
크기: 1280x720
DPI: 72 (web)


요소:
  1. Title (큰 폰트, 60pt+):
     "OpenVLA + ROS2"
     "minimal demo"


  2. 핵심 수치 (중간 폰트, 32pt):
     "RTX 4070 / 4-bit nf4"
     "Mean 165 ms / 6 Hz"


  3. 배경: Rerun 화면 캡처 + 약간 어둡게


  4. 본인 정보 (작은 폰트, 16pt):
     "AMR ROS 양산 SW + Physical AI"
```


---


## 실습 5: 최종 export + GIF 생성


### 영상 최종 export


```bash
# Shotcut 에서 export
# Preset: H.264 High Profile
# Resolution: 1920x1080
# Frame rate: 30
# Bitrate: 2~3 Mbps
# Output: vla_demo.mp4


# 또는 ffmpeg
ffmpeg -i edited.mp4 \
  -c:v libx264 -preset slow -crf 23 \
  -c:a aac -b:a 128k \
  -movflags +faststart \
  vla_demo.mp4


# 용량 확인 (< 30MB 권장)
ls -lh vla_demo.mp4
```


### GIF preview 생성


```bash
# 영상의 핵심 segment (25 ~ 35초의 Rerun 화면) 5~10초를 GIF
ffmpeg -i vla_demo.mp4 -ss 00:00:25 -t 8 \
  -vf "fps=10,scale=640:-1:flags=lanczos,palettegen" \
  palette.png


ffmpeg -i vla_demo.mp4 -ss 00:00:25 -t 8 \
  -i palette.png \
  -lavfi "fps=10,scale=640:-1:flags=lanczos[x];[x][1:v]paletteuse" \
  -loop 0 demo.gif


# 용량 확인 (< 5MB 권장)
ls -lh demo.gif
```


---


## 실습 체크리스트


- [ ] week 12 영상 재검토
- [ ] 자막 한국어 (모든 segment)
- [ ] 인트로 / 아웃트로 카드 추가
- [ ] thumbnail 1280x720 png 제작
- [ ] vla_demo.mp4 최종 export (< 30MB)
- [ ] demo.gif preview 생성 (< 5MB)
- [ ] 모든 파일 Portfolio/01_VLA_v1/ 에 commit
- [ ] quiz_easy / quiz_medium


---


## 참고 자료


- [Shotcut Tutorial](https://shotcut.org/howtos/)
- [FFmpeg quick reference](https://www.ffmpeg.org/ffmpeg.html)
- [Canva (thumbnail)](https://www.canva.com/)
- [YouTube Audio Library](https://www.youtube.com/audiolibrary)
