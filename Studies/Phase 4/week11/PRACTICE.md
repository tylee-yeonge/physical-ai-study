# Week 11 실습: GitHub 저장소 정리 및 코드 품질 개선

> 🎯 **목표**: 프로젝트 코드를 체계적으로 구조화하고, docstring/type hint를 추가하며, 포트폴리오 수준의 README를 작성한다
> 💻 **언어**: Python
> ⏰ **예상 시간**: 8시간

---

## 📋 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 프로젝트 구조 생성 및 코드 분리 | 필수 | 2시간 |
| 2 | Docstring 및 Type Hint 추가 | 필수 | 2시간 |
| 3 | README.md 작성 실습 | 필수 | 2시간 |
| 4 | 코드 품질 검사 자동화 | 필수 | 2시간 |

---

## 🔧 환경 설정

```bash
pip install -r requirements.txt
```

---

## Step 1: 프로젝트 구조 생성 및 코드 분리

```python
"""
프로젝트 구조 자동 생성 스크립트
체계적인 폴더 구조를 자동으로 생성한다.
"""
import os


def create_project_structure(project_name: str = "camera-3d-perception"):
    """
    프로젝트 폴더 구조 생성

    Args:
        project_name: 프로젝트 루트 디렉토리 이름
    """
    directories = [
        f"{project_name}/src/models",
        f"{project_name}/src/datasets",
        f"{project_name}/src/evaluation",
        f"{project_name}/src/visualization",
        f"{project_name}/configs",
        f"{project_name}/scripts",
        f"{project_name}/notebooks",
        f"{project_name}/results/images",
        f"{project_name}/results/tables",
        f"{project_name}/results/logs",
        f"{project_name}/docs",
    ]

    init_files = [
        f"{project_name}/src/__init__.py",
        f"{project_name}/src/models/__init__.py",
        f"{project_name}/src/datasets/__init__.py",
        f"{project_name}/src/evaluation/__init__.py",
        f"{project_name}/src/visualization/__init__.py",
    ]

    for d in directories:
        os.makedirs(d, exist_ok=True)
        print(f"  디렉토리 생성: {d}/")

    for f in init_files:
        if not os.path.exists(f):
            with open(f, 'w') as fp:
                fp.write('"""모듈 초기화"""\n')
            print(f"  파일 생성: {f}")

    # .gitignore 생성
    gitignore_content = """# Python
__pycache__/
*.py[cod]
*.egg-info/
dist/
build/

# 환경
.env
.venv/

# IDE
.vscode/
.idea/

# 데이터 (용량 큼)
data/
*.pkl
*.pth
*.ckpt

# OS
.DS_Store
Thumbs.db

# Jupyter
.ipynb_checkpoints/
"""
    with open(f"{project_name}/.gitignore", 'w') as f:
        f.write(gitignore_content)
    print(f"  파일 생성: {project_name}/.gitignore")

    print(f"\n프로젝트 구조 생성 완료: {project_name}/")


# 실행
create_project_structure()
```

### 1.2 코드 분리 예시

```python
"""
기존의 하나의 큰 파일을 모듈별로 분리하는 예시

Before (나쁜 예):
  all_in_one.py (500줄)
    - 데이터 로드
    - 모델 정의
    - 학습
    - 평가
    - 시각화

After (좋은 예):
  src/datasets/kitti_dataset.py   # 데이터 로드
  src/models/fcos3d_wrapper.py    # 모델 정의
  src/evaluation/metrics.py       # 평가
  src/visualization/draw_3d_bbox.py # 시각화
  scripts/train.sh                # 학습 스크립트
"""

# === src/datasets/kitti_dataset.py ===
from typing import List, Dict, Optional
import numpy as np


def parse_kitti_label(label_path: str,
                      target_classes: Optional[List[str]] = None
                      ) -> List[Dict]:
    """
    KITTI 레이블 파일을 파싱하여 객체 정보를 반환한다.

    Args:
        label_path: 레이블 파일 경로 (예: '000000.txt').
        target_classes: 파싱할 클래스 목록.
            None이면 모든 클래스를 파싱한다.

    Returns:
        객체 정보 딕셔너리의 리스트.
        각 딕셔너리는 다음 키를 가진다:
        - 'type': str, 객체 클래스 이름
        - 'bbox_2d': [x1, y1, x2, y2], 2D 바운딩 박스
        - 'dimensions': [h, w, l], 3D 크기 (m)
        - 'location': [x, y, z], 3D 중심 좌표 (m)
        - 'rotation_y': float, Y축 회전각 (rad)

    Raises:
        FileNotFoundError: label_path가 존재하지 않을 때.
    """
    if target_classes is None:
        target_classes = ['Car', 'Pedestrian', 'Cyclist']

    objects = []
    with open(label_path, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) < 15:
                continue
            if parts[0] not in target_classes:
                continue

            obj = {
                'type': parts[0],
                'truncated': float(parts[1]),
                'occluded': int(parts[2]),
                'alpha': float(parts[3]),
                'bbox_2d': [float(x) for x in parts[4:8]],
                'dimensions': [float(x) for x in parts[8:11]],
                'location': [float(x) for x in parts[11:14]],
                'rotation_y': float(parts[14]),
            }
            objects.append(obj)
    return objects


def parse_kitti_calibration(calib_path: str) -> Dict[str, np.ndarray]:
    """
    KITTI 캘리브레이션 파일을 파싱한다.

    Args:
        calib_path: 캘리브레이션 파일 경로.

    Returns:
        캘리브레이션 행렬 딕셔너리.
        - 'P2': (3, 4) 좌측 카메라 투영 행렬
        - 'R0_rect': (3, 3) 보정 회전 행렬
        - 'Tr_velo_to_cam': (3, 4) LiDAR → Camera 변환
    """
    calib = {}
    with open(calib_path, 'r') as f:
        for line in f:
            if ':' in line:
                key, value = line.split(':', 1)
                calib[key.strip()] = np.array(
                    [float(x) for x in value.strip().split()]
                )

    result = {}
    if 'P2' in calib:
        result['P2'] = calib['P2'].reshape(3, 4)
    if 'R0_rect' in calib:
        result['R0_rect'] = calib['R0_rect'].reshape(3, 3)
    if 'Tr_velo_to_cam' in calib:
        result['Tr_velo_to_cam'] = calib['Tr_velo_to_cam'].reshape(3, 4)

    return result


# 테스트
print("코드 분리 예시:")
print("  parse_kitti_label() - 레이블 파싱 함수")
print("  parse_kitti_calibration() - 캘리브레이션 파싱 함수")
print("  → 각 함수가 하나의 역할만 담당")
print("  → docstring, type hint 완비")
```

---

## Step 2: Docstring 및 Type Hint 추가

```python
"""
Docstring과 Type Hint 작성 연습
기존 코드에 문서화를 추가하는 방법을 연습한다.
"""
from typing import List, Tuple, Dict, Optional, Union
import numpy as np


# === 나쁜 예 (Before) ===
def compute_corners_bad(dims, loc, ry):
    h, w, l = dims
    x, y, z = loc
    R = np.array([[np.cos(ry), 0, np.sin(ry)],
                   [0, 1, 0],
                   [-np.sin(ry), 0, np.cos(ry)]])
    corners = np.array([
        [l/2, 0, w/2], [l/2, 0, -w/2], [-l/2, 0, -w/2], [-l/2, 0, w/2],
        [l/2, -h, w/2], [l/2, -h, -w/2], [-l/2, -h, -w/2], [-l/2, -h, w/2],
    ])
    corners = (R @ corners.T).T + np.array([x, y, z])
    return corners


# === 좋은 예 (After) ===
def compute_3d_box_corners(
    dimensions: Tuple[float, float, float],
    location: Tuple[float, float, float],
    rotation_y: float,
) -> np.ndarray:
    """
    3D 바운딩 박스의 8개 꼭짓점 좌표를 카메라 좌표계에서 계산한다.

    KITTI 좌표계 기준:
    - x: 오른쪽 (+)
    - y: 아래쪽 (+)
    - z: 전방 (+)

    Args:
        dimensions: (h, w, l) 높이, 너비, 길이 (m).
        location: (x, y, z) 3D 중심 좌표 (m).
            y는 박스 바닥면의 높이를 나타낸다.
        rotation_y: Y축 회전각 (rad).
            0이면 카메라 전방을 향함.

    Returns:
        corners: (8, 3) 8개 꼭짓점의 [x, y, z] 좌표.
            인덱스 0-3: 바닥면 (시계 방향, 전면 시작)
            인덱스 4-7: 윗면 (바닥면과 동일 순서)

    Examples:
        >>> dims = (1.5, 1.8, 4.5)  # 승용차
        >>> loc = (2.0, 1.65, 15.0)
        >>> ry = 0.0
        >>> corners = compute_3d_box_corners(dims, loc, ry)
        >>> print(corners.shape)  # (8, 3)
    """
    h, w, l = dimensions
    x, y, z = location

    # Y축(수직축) 기준 회전 행렬
    cos_ry = np.cos(rotation_y)
    sin_ry = np.sin(rotation_y)
    rotation_matrix = np.array([
        [ cos_ry, 0, sin_ry],
        [ 0,      1, 0     ],
        [-sin_ry, 0, cos_ry],
    ])

    # 중심 기준 8개 꼭짓점 (바닥면 4개 + 윗면 4개)
    corners_local = np.array([
        [ l/2,  0,    w/2],   # 0: 전면 오른쪽 바닥
        [ l/2,  0,   -w/2],   # 1: 전면 왼쪽 바닥
        [-l/2,  0,   -w/2],   # 2: 후면 왼쪽 바닥
        [-l/2,  0,    w/2],   # 3: 후면 오른쪽 바닥
        [ l/2, -h,    w/2],   # 4: 전면 오른쪽 윗면
        [ l/2, -h,   -w/2],   # 5: 전면 왼쪽 윗면
        [-l/2, -h,   -w/2],   # 6: 후면 왼쪽 윗면
        [-l/2, -h,    w/2],   # 7: 후면 오른쪽 윗면
    ])

    # 회전 적용 + 위치 이동
    corners_world = (rotation_matrix @ corners_local.T).T
    corners_world += np.array([x, y, z])

    return corners_world


# 비교
print("=== Docstring & Type Hint 비교 ===\n")
print("나쁜 예: compute_corners_bad(dims, loc, ry)")
print("  → 인자 의미 불명확, 반환값 불명확\n")
print("좋은 예: compute_3d_box_corners(dimensions, location, rotation_y)")
print("  → 인자 의미 명확, Type Hint, Docstring, Examples 포함\n")

# 동작 확인
corners = compute_3d_box_corners(
    dimensions=(1.5, 1.8, 4.5),
    location=(2.0, 1.65, 15.0),
    rotation_y=0.0,
)
print(f"결과 shape: {corners.shape}")
print(f"첫 번째 꼭짓점: {corners[0]}")
```

---

## Step 3: README.md 작성 실습

```python
"""
README.md 자동 생성 보조 스크립트
프로젝트 정보를 수집하여 README 초안을 생성한다.
"""
import os
from datetime import datetime


def generate_readme_template(
    project_name: str,
    description: str,
    methods: list,
    results: list,
    requirements: list,
) -> str:
    """
    README.md 템플릿 생성

    Args:
        project_name: 프로젝트 이름
        description: 프로젝트 설명 (1-2문장)
        methods: 사용한 방법론 리스트
        results: 주요 결과 (dict 리스트)
        requirements: 필요 패키지 리스트

    Returns:
        README.md 내용 문자열
    """
    readme = f"""# {project_name}

> {description}

![결과 이미지](results/images/detection_demo.png)

## 프로젝트 개요

### 배경
카메라 기반 3D Object Detection은 자율주행과 로봇 시스템의 핵심 기술이다.
LiDAR에 비해 카메라는 저렴하고 풍부한 시각 정보를 제공하지만,
깊이(Depth) 추정의 모호성이라는 근본적인 도전이 있다.

### 목표
- 카메라 이미지만으로 3D 객체를 검출하는 파이프라인 구현
- Monocular(단안) 및 Multi-view(다시점) 접근법 비교
- KITTI와 nuScenes 벤치마크에서의 성능 평가

### 사용 방법론
"""
    for m in methods:
        readme += f"- {m}\n"

    readme += "\n## 주요 결과\n\n"
    readme += "| 모델 | 데이터셋 | 지표 | 결과 |\n"
    readme += "|------|---------|------|------|\n"
    for r in results:
        readme += f"| {r['model']} | {r['dataset']} | {r['metric']} | {r['value']} |\n"

    readme += f"""
## 설치 방법

```bash
conda create -n 3d-perception python=3.8 -y
conda activate 3d-perception
pip install -r requirements.txt
```

### 필수 패키지
"""
    for pkg in requirements:
        readme += f"- {pkg}\n"

    readme += f"""
## 실행 방법

```bash
# 3D Detection 시각화
python src/visualization/draw_3d_bbox.py --data_path data/kitti --idx 0

# 성능 평가
python src/evaluation/metrics.py --result_path results/
```

## 프로젝트 구조

```
{project_name}/
├── src/               # 소스 코드
│   ├── models/        # 모델 래퍼
│   ├── datasets/      # 데이터 처리
│   ├── evaluation/    # 평가 메트릭
│   └── visualization/ # 시각화
├── configs/           # 설정 파일
├── scripts/           # 실행 스크립트
├── notebooks/         # 분석 노트북
└── results/           # 결과물
```

## 참고 자료

- [FCOS3D](https://arxiv.org/abs/2104.10956) - ICCV 2021 Workshop
- [BEVFormer](https://arxiv.org/abs/2203.17270) - ECCV 2022
- [KITTI Dataset](http://www.cvlibs.net/datasets/kitti/)
- [nuScenes Dataset](https://www.nuscenes.org/)

---

생성일: {datetime.now().strftime('%Y-%m-%d')}
"""
    return readme


# README 생성
readme_content = generate_readme_template(
    project_name="camera-3d-perception",
    description="카메라 이미지만으로 3D 객체를 검출하는 프로젝트. KITTI와 nuScenes에서 FCOS3D와 BEVFormer를 실습한다.",
    methods=[
        "FCOS3D - Monocular 3D Detection (KITTI)",
        "BEVFormer - Multi-view BEV Detection (nuScenes)",
        "3D BBox 시각화 및 BEV Feature 분석",
        "AP3D, NDS 성능 평가",
    ],
    results=[
        {'model': 'FCOS3D', 'dataset': 'KITTI', 'metric': 'AP3D (Car, Mod.)', 'value': '13.87%'},
        {'model': 'BEVFormer-Base', 'dataset': 'nuScenes', 'metric': 'NDS', 'value': '0.517'},
        {'model': 'BEVFormer-Base', 'dataset': 'nuScenes', 'metric': 'mAP', 'value': '0.416'},
    ],
    requirements=[
        "PyTorch >= 1.13",
        "mmcv-full >= 1.7",
        "mmdet >= 2.28",
        "mmdet3d >= 1.1",
        "numpy, matplotlib, opencv-python",
    ],
)

print("=" * 50)
print("생성된 README.md 미리보기:")
print("=" * 50)
print(readme_content[:1000])
print("...\n")
print(f"총 {len(readme_content)}자")
print("README.md를 저장하려면 이 내용을 파일에 쓰면 됩니다.")
```

---

## Step 4: 코드 품질 검사 자동화

```python
"""
코드 품질 검사 스크립트
프로젝트의 코드 품질을 자동으로 검사한다.
"""
import os
import ast
import sys
from typing import List, Dict


def check_docstrings(file_path: str) -> Dict:
    """
    Python 파일의 docstring 존재 여부를 검사한다.

    Args:
        file_path: 검사할 Python 파일 경로

    Returns:
        검사 결과 딕셔너리
    """
    with open(file_path, 'r', encoding='utf-8') as f:
        source = f.read()

    try:
        tree = ast.parse(source)
    except SyntaxError:
        return {'file': file_path, 'error': 'SyntaxError', 'functions': []}

    functions = []
    for node in ast.walk(tree):
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            has_docstring = (
                node.body and
                isinstance(node.body[0], ast.Expr) and
                isinstance(node.body[0].value, (ast.Str, ast.Constant))
            )
            functions.append({
                'name': node.name,
                'line': node.lineno,
                'has_docstring': has_docstring,
            })

    return {
        'file': file_path,
        'total_functions': len(functions),
        'with_docstring': sum(1 for f in functions if f['has_docstring']),
        'without_docstring': [f for f in functions if not f['has_docstring']],
    }


def check_type_hints(file_path: str) -> Dict:
    """
    Python 파일의 type hint 존재 여부를 검사한다.

    Args:
        file_path: 검사할 Python 파일 경로

    Returns:
        검사 결과 딕셔너리
    """
    with open(file_path, 'r', encoding='utf-8') as f:
        source = f.read()

    try:
        tree = ast.parse(source)
    except SyntaxError:
        return {'file': file_path, 'error': 'SyntaxError'}

    functions = []
    for node in ast.walk(tree):
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            has_return_annotation = node.returns is not None
            annotated_args = sum(
                1 for arg in node.args.args
                if arg.annotation is not None and arg.arg != 'self'
            )
            total_args = sum(
                1 for arg in node.args.args if arg.arg != 'self'
            )

            functions.append({
                'name': node.name,
                'line': node.lineno,
                'has_return_type': has_return_annotation,
                'annotated_args': annotated_args,
                'total_args': total_args,
            })

    return {
        'file': file_path,
        'functions': functions,
    }


def run_quality_check(directory: str = '.'):
    """프로젝트 전체의 코드 품질 검사를 실행한다."""
    print("=" * 50)
    print("코드 품질 검사 실행")
    print("=" * 50)

    py_files = []
    for root, dirs, files in os.walk(directory):
        # 무시할 디렉토리
        dirs[:] = [d for d in dirs if d not in
                   {'__pycache__', '.git', '.venv', 'node_modules'}]
        for f in files:
            if f.endswith('.py') and not f.startswith('__'):
                py_files.append(os.path.join(root, f))

    if not py_files:
        print("  Python 파일이 없습니다.")
        return

    total_functions = 0
    total_with_docstring = 0
    total_with_types = 0

    for fp in py_files:
        # Docstring 검사
        doc_result = check_docstrings(fp)
        total_functions += doc_result.get('total_functions', 0)
        total_with_docstring += doc_result.get('with_docstring', 0)

        if doc_result.get('without_docstring'):
            print(f"\n  {fp}:")
            for func in doc_result['without_docstring']:
                print(f"    line {func['line']}: {func['name']}() - docstring 없음")

        # Type Hint 검사
        type_result = check_type_hints(fp)
        for func in type_result.get('functions', []):
            if func['total_args'] > 0 and func['annotated_args'] == func['total_args']:
                total_with_types += 1

    print(f"\n{'=' * 50}")
    print(f"검사 결과 요약:")
    print(f"  파일 수: {len(py_files)}")
    print(f"  전체 함수: {total_functions}")
    doc_rate = (total_with_docstring / total_functions * 100) if total_functions > 0 else 0
    print(f"  Docstring 비율: {total_with_docstring}/{total_functions} ({doc_rate:.0f}%)")
    type_rate = (total_with_types / total_functions * 100) if total_functions > 0 else 0
    print(f"  Type Hint 비율: {total_with_types}/{total_functions} ({type_rate:.0f}%)")

    # 등급 판정
    if doc_rate >= 80 and type_rate >= 60:
        grade = "A (우수)"
    elif doc_rate >= 60 and type_rate >= 40:
        grade = "B (양호)"
    elif doc_rate >= 40:
        grade = "C (보통)"
    else:
        grade = "D (개선 필요)"

    print(f"\n  코드 품질 등급: {grade}")
    print(f"{'=' * 50}")


# 현재 파일로 테스트
if __name__ == '__main__':
    result = check_docstrings(__file__)
    print(f"현재 파일 검사:")
    print(f"  전체 함수: {result['total_functions']}")
    print(f"  Docstring 있음: {result['with_docstring']}")
    print(f"  Docstring 없음: {len(result['without_docstring'])}")
```

---

## ✅ 체크리스트

### 프로젝트 구조
- [ ] 폴더 구조가 체계적으로 생성되었는가
- [ ] __init__.py 파일이 각 패키지에 존재하는가
- [ ] .gitignore가 올바르게 설정되었는가
- [ ] 코드가 모듈별로 분리되었는가

### 코드 품질
- [ ] 모든 함수에 docstring이 있는가
- [ ] 주요 인자에 type hint가 있는가
- [ ] 매직 넘버가 상수로 정의되었는가
- [ ] 네이밍이 일관적인가 (snake_case)

### README
- [ ] 프로젝트 한 줄 설명이 있는가
- [ ] 결과 이미지가 포함되었는가
- [ ] 설치/실행 방법이 명확한가
- [ ] 프로젝트 구조가 설명되었는가

### Git 관리
- [ ] 커밋 메시지가 규칙적인가
- [ ] 불필요한 파일이 git에 포함되지 않았는가
- [ ] requirements.txt가 최신인가

---

## 💡 추가 실험 아이디어

1. **코드 포매터 적용**: `black` 또는 `autopep8`으로 코드 스타일 통일
2. **정적 분석**: `mypy`로 type hint 검증, `flake8`으로 코드 스타일 검사
3. **테스트 코드**: `pytest`로 핵심 함수의 단위 테스트 작성
4. **GitHub Actions**: push 시 자동 품질 검사 파이프라인 구축

---

이전: [Week 11 README](./README.md)

다음: [Week 12 - 블로그 & 영상](../week12/README.md)
