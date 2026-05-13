# Week 11: 코드 및 문서 정리 - GitHub 포트폴리오 완성


> **이번 주 목표**: 지금까지의 3D Perception 프로젝트 코드를 GitHub에 체계적으로 정리하고, 면접관이 읽기 좋은 README와 문서를 작성한다.
> **예상 시간**: 12-15시간
> **핵심 질문**: "내 GitHub 저장소를 처음 본 면접관이 5분 안에 내 역량을 파악할 수 있는가?"


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | README 필수 요소, Git 커밋 메시지, 코드 품질 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | 코드 리팩토링, Docstring/Type Hint 실습 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | GitHub 저장소 구조 설계, README 작성, 코드 정리 |


---


## 시작하기 전에


### 왜 코드 정리가 중요한가?


```
면접관의 GitHub 리뷰 프로세스:
  1. README 확인 (30초) → "이 사람이 뭘 했는지?"
  2. 프로젝트 구조 확인 (1분) → "코드가 체계적인가?"
  3. 핵심 코드 리뷰 (3분) → "실력이 어느 정도인가?"
  4. 결과 확인 (1분) → "실제로 동작하는가?"


→ 총 5분 안에 판단!
→ 코드 품질 = 실무 역량의 척도
```


**좋은 프로젝트 vs 나쁜 프로젝트:**
```
나쁜 예:
+-- test.py
+-- test2.py
+-- final_final.py
+-- untitled.ipynb
+-- (README 없음)


좋은 예:
+-- README.md ← 프로젝트 설명
+-- requirements.txt ← 의존성
+-- configs/ ← 설정 파일
+-- src/
| +-- models/ ← 모델 정의
| +-- datasets/ ← 데이터 처리
| +-- evaluation/ ← 평가 코드
| +-- visualization/ ← 시각화
+-- scripts/ ← 실행 스크립트
+-- results/ ← 결과 이미지
+-- docs/ ← 추가 문서
```


---


## 핵심 개념 자세히 알아보기


### 1. GitHub Repository 구조 설계


#### 1.1 권장 구조


```
camera-3d-perception/
+-- README.md # 프로젝트 메인 설명
+-- LICENSE # 라이선스
+-- requirements.txt # Python 의존성
+-- setup.py # 패키지 설치 (선택)
+-- .gitignore # Git 무시 파일
|
+-- configs/ # 설정 파일
| +-- fcos3d_kitti.py
| +-- bevformer_nuscenes.py
|
+-- src/ # 소스 코드
| +-- __init__.py
| +-- models/ # 모델 관련
| | +-- __init__.py
| | +-- fcos3d_wrapper.py
| | +-- bevformer_wrapper.py
| +-- datasets/ # 데이터 처리
| | +-- __init__.py
| | +-- kitti_dataset.py
| | +-- nuscenes_dataset.py
| +-- evaluation/ # 평가
| | +-- __init__.py
| | +-- metrics.py # AP3D, NDS 계산
| | +-- analysis.py # 오류 분석
| +-- visualization/ # 시각화
| +-- __init__.py
| +-- draw_3d_bbox.py
| +-- bev_visualization.py
| +-- bev_feature_viz.py
|
+-- scripts/ # 실행 스크립트
| +-- train.sh
| +-- test.sh
| +-- visualize.sh
|
+-- notebooks/ # Jupyter 노트북
| +-- 01_kitti_exploration.ipynb
| +-- 02_3d_bbox_visualization.ipynb
| +-- 03_bevformer_analysis.ipynb
|
+-- results/ # 결과물
| +-- images/ # 시각화 결과
| +-- tables/ # 성능 표
| +-- logs/ # 학습 로그
|
+-- docs/ # 추가 문서
    +-- installation.md
    +-- dataset_preparation.md
    +-- model_comparison.md
```


#### 1.2 .gitignore 설정


```
# .gitignore


# Python
__pycache__/
*.py[cod]
*.egg-info/
dist/
build/
*.egg


# 환경
.env
.venv/
env/


# IDE
.vscode/
.idea/
*.swp


# 데이터 (용량 큼)
data/
*.pkl
*.pth
*.ckpt


# 결과 중 큰 파일
*.mp4
*.avi


# OS
.DS_Store
Thumbs.db


# Jupyter
.ipynb_checkpoints/
```


---


### 2. README 작성 가이드


#### 2.1 README 필수 구성 요소


```
좋은 README의 구조:


1. 프로젝트 제목 + 한 줄 설명
2. 데모 이미지/GIF (시각적 임팩트!)
3. 프로젝트 배경 및 동기
4. 사용한 방법론 간략 설명
5. 주요 결과 (수치 + 시각화)
6. 설치 방법
7. 실행 방법
8. 프로젝트 구조
9. 참고 논문/자료
```


#### 2.2 README 템플릿


```markdown
# Camera-based 3D Object Detection


> 카메라 이미지만으로 3D 객체를 검출하는 프로젝트입니다.
> KITTI와 nuScenes 데이터셋에서 Monocular/Multi-view 3D Detection을 실습합니다.


![결과 이미지](results/images/detection_demo.png)


## 프로젝트 배경


자율주행과 로봇 시스템에서 3D 공간 인지는 필수적이다.
본 프로젝트는 카메라 기반 3D Object Detection의 핵심 기술을 학습하고,
FCOS3D(Monocular)와 BEVFormer(Multi-view)를 실습한다.


### 다루는 내용
- 3D Bounding Box 이해 및 시각화
- KITTI 좌표계 및 캘리브레이션
- Monocular 3D Detection (FCOS3D)
- Multi-view BEV 기반 Detection (BEVFormer)
- 성능 평가 (AP3D, NDS, mAP)


## 주요 결과


| 모델 | 데이터셋 | 주요 지표 | 값 |
|------|---------|----------|-----|
| FCOS3D | KITTI | AP3D (Car, Mod.) | 13.87% |
| BEVFormer-Base | nuScenes | NDS | 0.517 |
| BEVFormer-Base | nuScenes | mAP | 0.416 |


### 시각화 결과


| KITTI 3D Detection | BEV 시각화 | BEV Feature |
|:---:|:---:|:---:|
| ![](results/images/kitti_3d.png) | ![](results/images/bev_det.png) | ![](results/images/bev_feat.png) |


## 설치


```bash
# 환경 생성
conda create -n 3d-perception python=3.8 -y
conda activate 3d-perception


# 의존성 설치
pip install -r requirements.txt
```


## 실행 방법


```bash
# KITTI 3D Detection 시각화
python src/visualization/draw_3d_bbox.py --data_path data/kitti --idx 0


# BEVFormer 추론
python scripts/inference_bevformer.py --config configs/bevformer_nuscenes.py


# 성능 평가
python src/evaluation/metrics.py --result_path results/predictions.json
```


## 프로젝트 구조


```
camera-3d-perception/
+-- src/ # 소스 코드
+-- configs/ # 설정 파일
+-- scripts/ # 실행 스크립트
+-- notebooks/ # 분석 노트북
+-- results/ # 결과물
```


## 참고 자료


- [FCOS3D](https://arxiv.org/abs/2104.10956) (ICCV 2021 Workshop)
- [BEVFormer](https://arxiv.org/abs/2203.17270) (ECCV 2022)
- [KITTI Dataset](http://www.cvlibs.net/datasets/kitti/)
- [nuScenes Dataset](https://www.nuscenes.org/)
```


---


### 3. 코드 품질 가이드


#### 3.1 Docstring 작성법


```python
def compute_3d_iou(box1: dict, box2: dict) -> float:
    """
    두 3D 바운딩 박스의 IoU를 계산한다.


    축 정렬(axis-aligned) 3D 박스의 IoU를 계산하는 간소화 버전이다.
    회전이 있는 경우 Shapely 라이브러리를 사용하는 것을 권장한다.


    Args:
        box1: 첫 번째 3D bbox.
            - 'center': [x, y, z] 중심 좌표 (m)
            - 'size': [l, w, h] 크기 (m)
        box2: 두 번째 3D bbox. box1과 동일한 형식.


    Returns:
        3D IoU 값 (0.0 ~ 1.0).


    Examples:
        >>> box1 = {'center': [0, 0, 10], 'size': [4.5, 1.8, 1.5]}
        >>> box2 = {'center': [0.5, 0, 10.5], 'size': [4.5, 1.8, 1.5]}
        >>> iou = compute_3d_iou(box1, box2)
        >>> print(f"IoU: {iou:.3f}")
        IoU: 0.584


    Note:
        회전이 있는 3D IoU는 이 함수로 계산할 수 없다.
        mmdet3d.core.bbox.iou_calculator를 참고하라.
    """
    # 구현...
    pass
```


#### 3.2 Type Hint 활용


```python
from typing import List, Tuple, Dict, Optional
import numpy as np




def project_3d_to_2d(
    points_3d: np.ndarray,
    intrinsic: np.ndarray,
    extrinsic: Optional[np.ndarray] = None,
) -> Tuple[np.ndarray, np.ndarray]:
    """
    3D 점을 2D 이미지 좌표로 투영한다.


    Args:
        points_3d: (N, 3) 3D 좌표. 카메라 좌표계 기준.
        intrinsic: (3, 3) 카메라 내부 파라미터 행렬.
        extrinsic: (4, 4) 카메라 외부 파라미터 행렬.
            None이면 항등 변환 적용.


    Returns:
        points_2d: (N, 2) 투영된 2D 좌표 (u, v).
        valid_mask: (N,) 카메라 앞에 있는 점 마스크.
    """
    pass




def load_kitti_label(
    label_path: str,
    classes: List[str] = ['Car', 'Pedestrian', 'Cyclist'],
) -> List[Dict[str, any]]:
    """
    KITTI 레이블 파일을 파싱한다.


    Args:
        label_path: 레이블 파일 경로.
        classes: 로드할 클래스 목록.


    Returns:
        객체 정보 딕셔너리의 리스트.
    """
    pass
```


#### 3.3 코드 구조화 원칙


```
좋은 코드 구조의 3가지 원칙:


1. 단일 책임 원칙 (Single Responsibility)
   - 하나의 함수/클래스는 하나의 역할만
   - 나쁜 예: load_and_visualize_and_evaluate()
   - 좋은 예: load_data(), visualize(), evaluate()


2. 재사용성 (Reusability)
   - 공통 로직은 유틸리티 함수로 분리
   - 하드코딩 대신 인자로 받기
   - 나쁜 예: K = np.array([[721.5, 0, 609.5], ...])
   - 좋은 예: K = load_calibration(calib_path)


3. 일관성 (Consistency)
   - 네이밍 규칙 통일 (snake_case for Python)
   - 들여쓰기, import 순서 통일
   - 파일 구조 패턴 통일
```


---


### 4. Git 커밋 관리


#### 4.1 좋은 커밋 메시지


```
커밋 메시지 규칙:


형식: <type>(<scope>): <subject>


type:
  feat: 새로운 기능 추가
  fix: 버그 수정
  docs: 문서 수정
  style: 코드 포매팅 (기능 변경 없음)
  refactor: 코드 리팩토링
  test: 테스트 추가
  chore: 빌드, 설정 변경


예시:
  feat(visualization): 3D bbox BEV 시각화 함수 추가
  fix(evaluation): AP3D IoU threshold 조건 수정
  docs(readme): 설치 방법 및 실행 예시 추가
  refactor(dataset): KITTI 레이블 파싱 함수 분리
  style: black 포매터 적용
```


#### 4.2 Branch 전략


```
main (또는 master):
  - 안정된 코드만
  - 직접 push 지양


develop:
  - 개발 중인 코드
  - 기능 완성 후 main에 merge


feature/xxx:
  - 특정 기능 개발
  - 예: feature/bev-visualization
  - 완료 후 develop에 merge
```


---


### 5. 코드 리뷰 체크리스트


#### 5.1 자가 리뷰


```
코드 품질 체크리스트:


[ ] 모든 함수에 docstring이 있는가?
[ ] 주요 변수에 type hint가 있는가?
[ ] 매직 넘버가 상수로 정의되어 있는가?
[ ] 에러 처리(try-except)가 적절한가?
[ ] 불필요한 주석이나 print문이 제거되었는가?
[ ] import가 정리되어 있는가? (표준→서드파티→로컬 순)
[ ] 파일/함수 이름이 직관적인가?
[ ] 중복 코드가 없는가?
[ ] README에 실행 방법이 명확한가?
[ ] requirements.txt가 최신인가?
```


#### 5.2 면접관 관점 체크


```
면접관이 보는 것:


1. README 품질:
   [ ] 프로젝트 목적이 한 줄로 설명되는가?
   [ ] 결과 이미지/GIF가 있는가?
   [ ] 설치/실행이 즉시 가능한가?


2. 코드 구조:
   [ ] 폴더 구조가 체계적인가?
   [ ] 모듈 분리가 적절한가?
   [ ] 네이밍이 일관적인가?


3. 기술적 깊이:
   [ ] 핵심 알고리즘을 이해하고 구현했는가?
   [ ] 단순 복붙이 아닌 자체 구현이 있는가?
   [ ] 오류 분석/개선 시도가 있는가?


4. 실행 가능성:
   [ ] 실제로 동작하는 코드인가?
   [ ] 결과가 재현 가능한가?
   [ ] 의존성이 명확한가?
```


---


### 6. 시각화 자료 정리


#### 6.1 포트폴리오용 이미지 가이드


```
필수 시각화:
1. KITTI 3D Detection 결과 이미지 (3D bbox on image)
2. BEV 시각화 (위에서 본 검출 결과)
3. BEV Feature Map 히트맵
4. 학습 Loss 커브
5. 거리별/클래스별 성능 그래프


이미지 품질 기준:
- 해상도: 최소 150 DPI
- 형식: PNG (손실 없음)
- 크기: README에서 잘 보이는 크기
- 라벨: 축 이름, 범례, 제목 포함
- 색상: 색맹 친화적 팔레트 권장
```


#### 6.2 결과 표 정리


```
결과 표 예시 (README에 포함):


## 실험 결과


### Monocular 3D Detection (KITTI)


| 모델 | Backbone | AP3D Easy | AP3D Mod. | AP3D Hard |
|------|----------|-----------|-----------|-----------|
| FCOS3D | ResNet-101 | 18.52 | 13.87 | 11.23 |


### Multi-view 3D Detection (nuScenes)


| 모델 | NDS | mAP | mATE | mASE | mAOE | mAVE |
|------|-----|-----|------|------|------|------|
| BEVFormer-Base | 0.517 | 0.416 | 0.673 | 0.274 | 0.372 | 0.394 |
```


---


## 꼭 이해해야 할 핵심 개념


### 1. README가 면접의 첫인상


```
README = 프로젝트의 첫인상
  - 면접관은 README를 30초 안에 읽음
  - 결과 이미지가 없으면 관심을 잃음
  - 실행 방법이 불명확하면 신뢰를 잃음


핵심: "시각적 결과 + 명확한 설명 + 쉬운 실행"
```


### 2. 코드 품질이 실력의 증거


```
면접에서 자주 듣는 질문:
  "이 코드를 직접 작성하셨나요?"
  "왜 이렇게 구조를 잡으셨나요?"
  "개선하고 싶은 부분이 있나요?"


→ 코드를 깊이 이해하고 있어야 답변 가능
→ 복붙 코드는 질문에 취약
→ 직접 구조화한 코드는 자신있게 설명 가능
```


### 3. Git 히스토리의 중요성


```
좋은 Git 히스토리:
  feat(model): FCOS3D 모델 래퍼 구현
  feat(dataset): KITTI 데이터 로더 구현
  feat(viz): 3D bbox 시각화 함수 추가
  feat(eval): AP3D 평가 메트릭 구현
  docs(readme): 프로젝트 설명 및 결과 추가
  fix(eval): IoU 계산 시 빈 박스 처리 수정


→ 개발 과정이 체계적으로 보임
→ 점진적으로 기능을 추가한 것이 명확
```


---


## 자체 점검 - 이해했는지 확인!


**Q1. README에 결과 이미지가 중요한 이유는?**


> 면접관은 코드를 직접 실행해보지 않는 경우가 많다. 결과 이미지가 있으면 프로젝트가 실제로 동작한다는 것을 시각적으로 증명할 수 있고, 기술적 깊이를 한눈에 보여줄 수 있다. "백문이 불여일견"으로, 3D bbox가 이미지에 정확히 그려진 시각화 하나가 긴 설명보다 효과적이다.


**Q2. Type Hint를 사용하는 이점은?**


> Type Hint는 코드의 가독성을 높이고, IDE의 자동 완성과 정적 분석 도구(mypy)를 활용할 수 있게 한다. 함수의 입력과 출력이 명확해져서 다른 사람이 코드를 이해하기 쉽고, 타입 관련 버그를 조기에 발견할 수 있다. 특히 numpy 배열의 shape 정보를 docstring에 적어두면 디버깅에 큰 도움이 된다.


**Q3. 왜 데이터 파일(.pkl, .pth)은 git에 포함하지 않는가?**


> 데이터 파일과 모델 가중치는 용량이 크기 때문이다 (수 MB ~ 수 GB). Git은 텍스트 기반 버전 관리에 최적화되어 있어, 대용량 바이너리 파일은 저장소를 비대하게 만들고, clone 속도를 저하시킨다. 대신 README에 다운로드 링크를 제공하거나, Git LFS(Large File Storage)를 사용하거나, Google Drive 등 외부 저장소를 활용한다.


**Q4. 단일 책임 원칙이 코드 품질에 미치는 영향은?**


> 하나의 함수가 하나의 역할만 담당하면, 코드를 이해하기 쉽고, 테스트하기 쉽고, 재사용하기 쉽다. 예를 들어 `load_and_visualize()`는 데이터 로딩 로직을 변경할 때 시각화 코드도 함께 건드려야 하지만, `load_data()`와 `visualize()`로 분리하면 각각 독립적으로 수정할 수 있다. 이는 면접관에게 소프트웨어 엔지니어링 역량을 보여주는 좋은 지표이다.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제


1. **저장소 구조 설계**: 위 권장 구조에 따라 폴더 생성 및 코드 정리
2. **README 작성**: 프로젝트 설명, 결과 이미지, 설치/실행 방법 포함
3. **코드 리팩토링**: docstring 추가, type hint 적용, 매직 넘버 제거
4. **시각화 정리**: 포트폴리오용 고품질 이미지 생성 및 저장
5. **.gitignore 설정**: 불필요한 파일 제외
6. **커밋 정리**: 의미 있는 커밋 메시지로 히스토리 정리


자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고


### 다음 주 준비


- 블로그 플랫폼 결정 (Tistory, Velog, Medium 등)
- 데모 영상 녹화 환경 준비 (OBS Studio 등)
- LinkedIn 프로필 초안 작성


---


## 이번 주 핵심 요약


1. **GitHub 저장소 구조**는 체계적인 폴더 구조, .gitignore, requirements.txt가 기본이다. 면접관은 구조만 봐도 코드 품질을 가늠한다.
2. **README**는 프로젝트의 첫인상으로, 한 줄 설명 + 결과 이미지 + 설치/실행 방법이 필수이다. 30초 안에 핵심을 전달해야 한다.
3. **코드 품질**은 docstring, type hint, 단일 책임 원칙, 일관된 네이밍으로 증명한다. 면접에서 "왜 이렇게 했는가?"에 답할 수 있어야 한다.
4. **Git 커밋**은 체계적인 메시지로 개발 과정을 기록하며, 이는 면접관에게 개발 습관과 역량을 보여준다.
5. **시각화 자료**는 고품질 PNG로 결과를 증명하며, README에 포함하여 프로젝트의 완성도를 높인다.


---


이전: [Week 10 - BEVFormer 실습](../week10/README.md)


다음: [Week 12 - 블로그 & 영상](../week12/README.md)
