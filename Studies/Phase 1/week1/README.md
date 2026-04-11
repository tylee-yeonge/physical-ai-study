# 선형대수의 본질 (Essence of Linear Algebra) - 3Blue1Brown

## [pin] 개요

이 섹션은 3Blue1Brown의 **"Essence of Linear Algebra"** 시리즈를 통해 선형대수학의 핵심 개념을 **시각적이고 직관적으로** 이해하는 것을 목표로 합니다. 전통적인 수식 중심 접근과 달리, 기하학적 관점에서 선형대수를 이해함으로써 Visual SLAM의 수학적 기초를 탄탄히 다질 수 있습니다.

3Blue1Brown은 복잡한 수학 개념을 아름다운 애니메이션으로 설명하는 것으로 유명하며, 특히 선형대수 시리즈는 **전 세계적으로 수백만 명의 학습자들이 추천**하는 최고의 입문 자료입니다.

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 영상 시청 | - | 3Blue1Brown 선형대수 시리즈 (16개 챕터) |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |

---

## [goal] 학습 목표

Visual SLAM에 필수적인 다음 개념들을 **기하학적 직관**을 중심으로 이해합니다:

- 벡터 연산의 기하학적 의미 이해하기
- 행렬을 선형 변환으로 이해하기
- 고유값/고유벡터 직관 얻기
- 내적과 외적의 의미 파악하기

## [ref] 사전 지식

### 필수 사항
- **기초 수학**: 고등학교 수준의 대수학 (방정식, 함수 개념)
- **기초 기하학**: 2D/3D 공간에서의 점, 선, 평면 개념
- **기본 연산**: 덧셈, 곱셈, 함수 합성

### 권장 사항
- **미적분학 기초**: 미분/적분의 기본 개념 (필수는 아니지만 도움이 됨)
- **프로그래밍 경험**: Python 또는 MATLAB (시각화 및 실습용)

> [!NOTE]
> 이 시리즈는 수학적 엄밀함보다는 **직관적 이해**에 중점을 두므로, 고급 수학 지식이 없어도 학습 가능합니다.

## [time] 예상 학습 시간

- **전체 동영상 시청**: 약 3-4시간
- **개념 정리 및 실습**: 약 4-6시간
- **총 소요 시간**: **1-2주** (하루 1-2시간 학습 기준)

## [video] 공식 자료

- **YouTube 재생목록**: [Essence of Linear Algebra](https://www.youtube.com/playlist?list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab)
- **공식 웹사이트**: [3Blue1Brown](https://www.3blue1brown.com/)
- **한글 자막**: YouTube에서 한글 자막(CC) 제공

## [ref] 주요 학습 내용

### 1. 벡터와 선형 결합
- 벡터의 기하학적 의미 (위치 vs 방향)
- 벡터 덧셈과 스칼라 곱의 시각화
- 선형 결합(linear combination)과 span 개념

### 2. 선형 변환과 행렬
- **행렬 = 선형 변환**의 핵심 직관
- 기저 벡터(basis vector)의 변환으로 행렬 이해하기
- 행렬 곱셈의 기하학적 의미 (변환의 합성)

### 3. 행렬식과 역행렬
- 행렬식(determinant): 변환 후 면적/부피의 스케일링 인수
- 역행렬의 기하학적 의미 (변환 되돌리기)
- 선형 시스템의 해 존재성

### 4. 내적과 외적
- **내적(dot product)**: 투영(projection)의 측도
- **외적(cross product)**: 3D 공간에서 수직 벡터 생성
- SLAM에서의 활용: 카메라 포즈, 법선 벡터 계산

### 5. 고유값과 고유벡터
- **고유벡터**: 변환해도 방향이 바뀌지 않는 특별한 벡터
- **고유값**: 해당 방향으로의 스케일링 정도
- SLAM에서의 활용: PCA, 특징점 추출, 최적화

### 6. 추상 벡터 공간
- 벡터 공간의 일반화 (함수, 다항식도 벡터!)
- 선형대수의 보편적 적용 가능성

## � 챕터별 학습 체크리스트

아래는 3Blue1Brown의 "Essence of Linear Algebra" 시리즈의 전체 챕터 목록입니다. 

- **Chapter 1**: Vectors, what even are they
- **Chapter 2**: Linear combinations, span, basis vectors
- **Chapter 3**: Linear transformations and matrices
- **Chapter 4**: Matrix multiplication as composition
- **Chapter 5**: Three-dimensional linear transformations
- **Chapter 6**: The determinant
- **Chapter 7**: Inverse matrices, column space and null space
- **Chapter 8**: Nonsquare matrices as transformations between dimensions
- **Chapter 9**: Dot products and duality
- **Chapter 10**: Cross products
- **Chapter 11**: Cross products in the light of linear transformations
- **Chapter 12**: Cramer's rule, explained geometrically
- **Chapter 13**: Change of basis
- **Chapter 14**: Eigenvectors and eigenvalues
- **Chapter 15**: A quick trick for computing eigenvalues
- **Chapter 16**: Abstract vector spaces

> [!TIP]
> 각 챕터를 완료한 후에는 해당 개념을 자신만의 언어로 설명해보거나, Python/MATLAB으로 직접 구현해보는 것을 추천합니다.

## �[tip] 학습 팁

1. **각 영상을 2번 이상 시청하세요**
   - 1회: 전체 흐름 파악
   - 2회: 세부 내용 이해 및 노트 정리

2. **직접 그려보세요**
   - 종이에 벡터, 행렬 변환을 그려보면서 학습
   - [Desmos](https://www.desmos.com/calculator) 또는 [GeoGebra](https://www.geogebra.org/)로 시각화

3. **Python으로 실습하세요**
   ```python
   import numpy as np
   import matplotlib.pyplot as plt
   
   # 벡터 시각화 예제
   v1 = np.array([2, 3])
   v2 = np.array([1, -1])
   
   plt.quiver(0, 0, v1[0], v1[1], scale=1, scale_units='xy', angles='xy')
   plt.quiver(0, 0, v2[0], v2[1], scale=1, scale_units='xy', angles='xy')
   plt.xlim(-1, 4)
   plt.ylim(-2, 4)
   plt.grid()
   plt.show()
   ```

4. **개념 연결하기**
   - 각 개념이 Visual SLAM의 어떤 부분에 사용되는지 생각해보세요
   - 예: 고유값 분해 → 공분산 행렬 분석 → 불확실성 표현

## [link] 관련 자료

- **연습 문제**: [3Blue1Brown 공식 연습 문제](https://www.3blue1brown.com/lessons/eola-preview)
- **보충 자료**: Khan Academy - [Linear Algebra](https://www.khanacademy.org/math/linear-algebra)
- **추천 교재**: "Linear Algebra and Its Applications" by Gilbert Strang

## [O] 학습 완료 체크리스트

다음 질문에 답할 수 있다면 이 섹션을 완료한 것입니다:

- [ ] 2x2 행렬이 주어졌을 때, 이것이 어떤 선형 변환을 나타내는지 그림으로 설명할 수 있나요?
- [ ] 행렬식이 0이라는 것이 기하학적으로 무엇을 의미하는지 설명할 수 있나요?
- [ ] 내적의 결과가 0이라는 것은 두 벡터가 어떤 관계인지 설명할 수 있나요?
- [ ] 고유벡터와 고유값을 비유를 사용해 설명할 수 있나요?
- [ ] 3D 회전 행렬의 고유값과 고유벡터가 무엇을 의미하는지 이해하고 있나요?

---

> [!TIP]
> **다음 단계**: 이 시리즈를 완료한 후에는 Gilbert Strang의 MIT OpenCourseWare 강의나 "Introduction to Linear Algebra" 교재로 더 깊이 있는 학습을 진행하는 것을 추천합니다.