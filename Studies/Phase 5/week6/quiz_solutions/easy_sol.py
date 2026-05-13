"""Phase 5 Week 6 - 기초 정답"""


def p1():
    print("\n정답: B) image + dynamic prompts")
    print("  CLIP 의 open-vocabulary: 분류 class 자유")
    print("  학습 후 새 class 추가 가능 (prompt 만 작성)")


def p2():
    print("\n정답: B) 시연 / 면접 UI 빠르게")
    print("  Gradio: Python 코드 5줄로 web UI")
    print("  본 phase 의 1순위 회사 면접 시 'live demo' 강력")


def p3():
    print("\n정답: B) Spatial reasoning 약함")
    print("  CLIP 한계:")
    print("    1. spatial ('leftmost', 'on top of')")
    print("    2. instance ('the same cup')")
    print("    3. OCR")
    print("    4. dynamic scene")
    print()
    print("  OpenVLA 의 LLM 통합으로 spatial 보강 (emergent capability)")


def p4():
    print("\n정답: B) 환경 이해 (0:25-0:30)")
    print("  Phase 4 영상 구조:")
    print("    0:00-0:10 인트로")
    print("    0:10-0:25 system 구조")
    print("    0:25-0:45 실시간 동작 <- CLIP 보강 위치")
    print("    0:45-0:55 결과/한계")
    print("    0:55-1:00 next")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
