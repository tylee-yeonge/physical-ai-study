"""iPad Air M2 13인치용 체스보드 패턴 생성기.

생성된 이미지를 iPad에서 전체화면으로 표시하면
실제 물리적 크기와 일치하는 체스보드를 얻을 수 있다.

사용법:
    python generate_checkerboard.py
    python generate_checkerboard.py --square_mm 30
"""

import argparse

from PIL import Image
from PIL import ImageDraw


# iPad Air M2 13인치 스펙
IPAD_AIR_13_WIDTH = 2732
IPAD_AIR_13_HEIGHT = 2048
IPAD_AIR_13_PPI = 264


def generate_checkerboard(
    corners: tuple[int, int] = (9, 6),
    square_mm: float = 25.0,
    screen_w: int = IPAD_AIR_13_WIDTH,
    screen_h: int = IPAD_AIR_13_HEIGHT,
    ppi: int = IPAD_AIR_13_PPI,
    output: str = "checkerboard.png",
) -> None:
    """체스보드 패턴 이미지를 생성한다.

    Args:
        corners: 내부 코너 수 (cols, rows). OpenCV findChessboardCorners와 동일.
        square_mm: 한 칸의 물리적 크기 (mm).
        screen_w: 화면 가로 해상도 (px).
        screen_h: 화면 세로 해상도 (px).
        ppi: 화면 PPI (pixels per inch).
        output: 출력 파일 경로.
    """
    px_per_mm = ppi / 25.4
    square_px = round(square_mm * px_per_mm)

    cols = corners[0] + 1  # 내부 코너 + 1 = 칸 수
    rows = corners[1] + 1

    board_w = cols * square_px
    board_h = rows * square_px

    if board_w > screen_w or board_h > screen_h:
        print(f"경고: 체스보드({board_w}x{board_h})가 화면({screen_w}x{screen_h})보다 큽니다.")
        print(f"  → square_mm를 줄이거나 corners 수를 줄이세요.")
        return

    # 흰색 배경에 체스보드 중앙 배치
    img = Image.new("RGB", (screen_w, screen_h), "white")
    draw = ImageDraw.Draw(img)

    ox = (screen_w - board_w) // 2
    oy = (screen_h - board_h) // 2

    for r in range(rows):
        for c in range(cols):
            if (r + c) % 2 == 0:
                x = ox + c * square_px
                y = oy + r * square_px
                draw.rectangle([x, y, x + square_px, y + square_px], fill="black")

    img.save(output)

    print(f"체스보드 생성 완료: {output}")
    print(f"  내부 코너: {corners[0]}x{corners[1]}")
    print(f"  격자 크기: {square_px}px = {square_mm}mm")
    print(f"  체스보드:  {cols}x{rows}칸 ({board_w}x{board_h}px)")
    print(f"  여백:      좌우 {ox}px, 상하 {oy}px")
    print(f"  화면:      {screen_w}x{screen_h}px ({ppi} PPI)")


def main() -> None:
    """커맨드라인 인자를 파싱하고 체스보드를 생성한다."""
    parser = argparse.ArgumentParser(description="iPad용 체스보드 패턴 생성")
    parser.add_argument(
        "--corners",
        type=int,
        nargs=2,
        default=[9, 6],
        help="내부 코너 수 (cols rows), 기본값: 9 6",
    )
    parser.add_argument(
        "--square_mm",
        type=float,
        default=25.0,
        help="한 칸 크기 (mm), 기본값: 25.0",
    )
    parser.add_argument(
        "--output",
        type=str,
        default="checkerboard.png",
        help="출력 파일명, 기본값: checkerboard.png",
    )
    args = parser.parse_args()

    generate_checkerboard(
        corners=(args.corners[0], args.corners[1]),
        square_mm=args.square_mm,
        output=args.output,
    )


if __name__ == "__main__":
    main()
