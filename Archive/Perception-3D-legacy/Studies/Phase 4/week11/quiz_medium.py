"""
Phase 6 Week 11 - 코드 및 문서 정리 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import ast
import os


def problem1_refactoring():
    """
    문제 1: 코드 리팩토링

    아래 코드에는 여러 가지 코드 품질 문제가 있다.
    문제점을 식별하고, 개선된 코드를 작성하시오.

    문제점:
    1. 함수 이름이 불명확하다
    2. 매직 넘버가 사용되었다
    3. docstring이 없다
    4. type hint가 없다
    5. 한 함수에서 너무 많은 일을 한다

    TODO: 아래 bad_code의 문제점을 3가지 이상 나열하시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: 코드 리팩토링")
    print("━" * 36 + "\n")

    bad_code = '''
def process(d, t):
    r = []
    for item in d:
        if item[0] in ['Car', 'Pedestrian', 'Cyclist']:
            x = float(item[11])
            z = float(item[13])
            dist = (x**2 + z**2)**0.5
            if dist < 50:
                r.append({'type': item[0], 'dist': dist})
    if t == 1:
        for obj in r:
            print(f"{obj['type']}: {obj['dist']:.1f}m")
    return r
'''

    print("  나쁜 코드:")
    print(bad_code)

    print("  문제점을 3가지 이상 나열하시오:")
    print("  1. ___________")
    print("  2. ___________")
    print("  3. ___________")
    print()

    # 힌트
    print("  힌트:")
    print("  - 함수 이름 'process'가 의미하는 바는?")
    print("  - 'd', 't', 'r' 같은 변수명은?")
    print("  - 50이라는 숫자의 의미는?")
    print("  - 한 함수에서 필터링과 출력을 모두 하고 있는가?")
    print()
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def problem2_docstring_writing():
    """
    문제 2: Docstring 작성

    아래 함수의 docstring을 Google 스타일로 작성하시오.

    함수 설명:
    - KITTI 좌표계에서 3D 점을 2D 이미지에 투영
    - P2 투영 행렬 사용
    - 카메라 앞에 있는 점만 유효

    TODO: docstring을 작성하시오 (Args, Returns, Examples 포함).
    """
    print("\n" + "━" * 36)
    print("문제 2: Docstring 작성")
    print("━" * 36 + "\n")

    code = '''
import numpy as np

def project_to_2d(points_3d, P2):
    """
    TODO: 여기에 docstring을 작성하시오.

    Args:
        points_3d: ???
        P2: ???

    Returns:
        ???

    Examples:
        ???
    """
    N = points_3d.shape[0]
    pts_homo = np.hstack([points_3d, np.ones((N, 1))])
    projected = P2 @ pts_homo.T
    projected = projected.T
    valid = projected[:, 2] > 0
    pts_2d = projected[:, :2] / projected[:, 2:3]
    return pts_2d, valid
'''
    print("  함수 코드:")
    print(code)

    print("  docstring을 작성해보세요. 포함해야 할 내용:")
    print("  1. 함수 설명 (한 줄)")
    print("  2. Args (각 인자의 shape과 설명)")
    print("  3. Returns (반환값의 shape과 설명)")
    print("  4. Examples (간단한 사용 예시)")
    print()
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def problem3_project_structure():
    """
    문제 3: 프로젝트 구조 설계

    아래 파일 목록을 체계적인 프로젝트 구조로 재배치하시오.

    현재 파일:
      train.py
      test.py
      kitti_loader.py
      nuscenes_loader.py
      fcos3d_model.py
      bevformer_model.py
      draw_3d_bbox.py
      bev_viz.py
      compute_ap3d.py
      compute_nds.py
      README.md
      requirements.txt

    TODO: 각 파일을 어느 디렉토리에 넣을지 결정하시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: 프로젝트 구조 설계")
    print("━" * 36 + "\n")

    files = [
        'train.py', 'test.py',
        'kitti_loader.py', 'nuscenes_loader.py',
        'fcos3d_model.py', 'bevformer_model.py',
        'draw_3d_bbox.py', 'bev_viz.py',
        'compute_ap3d.py', 'compute_nds.py',
        'README.md', 'requirements.txt',
    ]

    print("  재배치할 파일 목록:")
    for f in files:
        print(f"    {f}")

    print()
    print("  폴더 구조 예시:")
    print("    project/")
    print("    ├── src/")
    print("    │   ├── models/       → ???")
    print("    │   ├── datasets/     → ???")
    print("    │   ├── evaluation/   → ???")
    print("    │   └── visualization/→ ???")
    print("    ├── scripts/          → ???")
    print("    ├── README.md")
    print("    └── requirements.txt")
    print()
    print("  각 파일을 어디에 넣을지 결정해보세요.")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 11 Quiz - Medium")
    print("━" * 40)
    problem1_refactoring()
    problem2_docstring_writing()
    problem3_project_structure()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
