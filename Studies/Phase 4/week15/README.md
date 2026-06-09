# Week 15: 1분 데모 영상 최종 + 자막 + thumbnail


> **이번 주 목표**: week 12 의 영상을 최종 마감. 자막 / 음악 / 인트로 / 아웃트로 / thumbnail. 면접관 진입점으로 완성.
> **예상 시간**: 6시간
> **핵심 질문**: "내 영상이 다른 후보 영상과 비교해 면접관이 가장 오래 머무는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | week 12 영상 재검토 | `PRACTICE.md` 1 | 부족한 부분 list |
| 2 | 자막 보강 | `PRACTICE.md` 2 | 한국어 / 영어 (선택) |
| 3 | 인트로 / 아웃트로 | `PRACTICE.md` 3 | 3-5초 title 카드 |
| 4 | thumbnail | `PRACTICE.md` 4 | YouTube / LinkedIn 용 |
| 5 | 최종 export | `PRACTICE.md` 5 | mp4 + gif |
| 6 | 퀴즈 | quiz_easy / quiz_medium | 영상 패키징 |


---


## 핵심 개념


### 1. 영상 최종 마감 체크리스트


```
- [ ] 인트로 (3~5초): 제목 + 본인 정보 + 핵심 메시지
- [ ] 본문 (50초): system 구조 + 실시간 동작 + 결과
- [ ] 아웃트로 (3~5초): "다음 산출물 Phase 7" + GitHub link
- [ ] 자막: 한국어, 한 줄 15자 이내, 2초+ 유지
- [ ] 음악: 배경음 (무료 license 만, ccmixter 등)
- [ ] thumbnail: 매력적 + 핵심 단어 ("OpenVLA + ROS2")
- [ ] 길이: 60~80초 (60초 권장)
- [ ] 형식: 1920x1080 mp4 H.264 30fps
- [ ] 용량: < 30 MB (GitHub 직접 호스팅)
```


### 2. 자막 작성 가이드 (재정리)


```
주의:
- 화면 하단 가운데, 폰트 18pt 이상
- 한 줄 15자 이내 (한국어 기준)
- 2초 이상 유지 (가독성)
- 흰색 + 검은 outline (배경에 관계없이 가독성)


예시:
0:05 "Phase 4 산출물 v1 - OpenVLA + ROS2"
0:10 "AMR ROS 양산 SW 실무 5년 엔지니어"
0:15 "ROS2 노드 구조"
0:20 "/camera/image_raw -> vla_node -> /vla/action"
...
0:55 "다음: Phase 7 Real-to-Sim-to-Real (2027.08~)"
```


### 3. Thumbnail 의 요소


YouTube / LinkedIn 미리보기에 노출되는 이미지:


```
- 크기: 1280x720 (16:9)
- 핵심 단어 2~3개 (큰 폰트):
  "OpenVLA + ROS2"
  "RTX 4070"
  "165 ms / 6 Hz"
- 다이어그램 일부 또는 Rerun 화면 캡처
- 본인 사진 (선택, 신뢰감)
```


도구: Canva / Figma / 그냥 PowerPoint.


### 4. 인트로 / 아웃트로 패턴


**인트로 (3-5초)**:
```
[Title 카드]
큰 글씨: "OpenVLA -> ROS2 Minimal Demo"
작은 글씨: "Phase 4 산출물 v1"
하단: "AMR ROS 양산 SW + Physical AI 통합"
```


**아웃트로 (3-5초)**:
```
[End 카드]
"다음 산출물: Real-to-Sim-to-Real (2027.08~)"
"github.com/<you>/physical-ai-study"
"contact: <linkedin>"
```


### 5. 배경음 license


| Source | 사용 가능성 |
|---|---|
| YouTube Audio Library | 무료 + 출처 표시 |
| ccmixter.org | CC BY (출처) |
| freemusicarchive.org | 다양 (각 license 확인) |
| epidemicsound.com | 유료 |


본 phase 권장: YouTube Audio Library, 출처 표시는 description 에.


### 6. 영상 export 표준


```bash
# Shotcut export 설정
- 해상도: 1920x1080
- 프레임률: 30 fps
- Codec: H.264
- Bitrate: 2~3 Mbps (file size 작게)
- Audio: AAC, 128 kbps


# 또는 ffmpeg
ffmpeg -i input.mp4 -c:v libx264 -preset slow -crf 23 \
  -c:a aac -b:a 128k -movflags +faststart output.mp4
```


### 7. GIF 변환 (README 의 미리보기용)


```bash
# 영상의 5~10초 segment 를 GIF 로
ffmpeg -i vla_demo.mp4 -ss 00:00:25 -t 8 \
  -vf "fps=10,scale=640:-1:flags=lanczos" \
  -loop 0 demo.gif
```


- 길이: 5-10초
- 크기: < 5 MB
- README 의 [![demo](demo.gif)](vla_demo.mp4) 식으로 embed


### 8. 최종 산출물 list


```
Portfolio/01_VLA_v1/
  README.md # week 14
  vla_demo.mp4 # 본 주 (60s, < 30MB)
  demo.gif # 본 주 (5s preview, < 5MB)
  thumbnail.png # 본 주 (1280x720, < 500KB)
  vla_node/ # ROS2 패키지
  vla_inference/ # Python 패키지
  bag/ # 압축 bag
  latency_data.csv # 측정 데이터
  scripts/setup.sh # week 14
```


---


## 자체 점검


**Q1. 영상의 인트로 / 아웃트로 길이 권장?**
> 각 3-5초. 너무 길면 면접관이 본문 도달 전 떠남.


**Q2. 자막의 한 줄 글자 수 권장?**
> 한국어 15자 이내. 영어 30자 이내.


**Q3. Thumbnail 의 핵심 요소 3가지?**
> 1) 큰 폰트 핵심 단어, 2) 다이어그램 / Rerun 스크린샷, 3) (선택) 본인 사진.


**Q4. README 의 demo GIF 의 권장 크기?**
> < 5 MB, 5-10초 길이.


**Q5. 최종 영상 형식의 권장?**
> 1920x1080, 30fps, H.264, 2-3 Mbps bitrate, < 30MB.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. week 12 영상 재검토
2. 자막 / 인트로 / 아웃트로 최종
3. thumbnail 제작
4. GIF preview 생성
5. Final export
6. 모든 파일을 Portfolio/01_VLA_v1/ 에 정리
7. quiz_easy / quiz_medium


### 다음 주 (week 16) 준비
- 산출물 v1 공개 (git push + LinkedIn 공지)


---


## 이번 주 핵심 요약


1. **인트로 / 아웃트로** 각 3-5초.
2. **자막** 한국어, 한 줄 15자, 2초+ 유지.
3. **Thumbnail** 1280x720, 핵심 단어 큰 폰트.
4. **GIF preview** 5-10초, < 5MB.
5. **최종 export** 1920x1080, < 30MB.


---


- 이전: [Week 14 - README + 환경 가이드](../week14/README.md)


다음: [Week 16 - 산출물 v1 공개](../week16/README.md)
