"""raw/ 의 npy 세 개에서 latency 분포 그림 (plots/latency_distribution.png) 을 그린다.

세 실행 ((b) mock · (a) LeRobot 직결 · (b) ROS 2 실제 팔) 을 한 축 (ms) 위에 점으로 늘어놓고
mean · p95 를 직접 표기한다. 히스토그램 대신 점을 쓰는 이유: n=100 이라 점 하나하나가
보이고, (b) mock 의 10-20 ms 균등 분포처럼 "모양" 이 곧 해석인 경우가 있기 때문이다.

실행 (matplotlib 은 컨테이너의 시스템 파이썬에 있다. lerobot venv 에는 없다):
    /usr/bin/python3 /workspace/study/physical-ai-study/Measurements/so101-dual-latency/scripts/plot_latency_distribution.py
"""

import glob
import os

import matplotlib
import numpy as np

matplotlib.use("Agg")  # 화면 없이 파일로만 그린다
import matplotlib.pyplot as plt  # noqa: E402  (백엔드를 정한 뒤에 불러야 한다)

HERE = os.path.dirname(os.path.abspath(__file__))
RAW_DIR = os.path.join(HERE, "..", "raw")
OUT_PATH = os.path.join(HERE, "..", "plots", "latency_distribution.png")

# 위에서 아래로 그릴 순서와 라벨. 파일명 stem 은 measure_latency_*.py 의 저장 이름이다
RUNS = [
    ("latency_ros2_mock", "(b) ROS 2, mock hardware"),
    ("latency_lerobot", "(a) LeRobot direct"),
    ("latency_ros2_real", "(b) ROS 2, real arm"),
]
# 색: dataviz 기준 팔레트의 범주형 슬롯 1-3 (blue · orange · aqua), 라이트 표면용
SERIES = ["#2a78d6", "#eb6834", "#1baf7a"]
SURFACE, INK, INK_2, MUTED, GRID, BASELINE = (
    "#fcfcfb",
    "#0b0b0b",
    "#52514e",
    "#898781",
    "#e1e0d9",
    "#c3c2b7",
)


def latest(stem: str) -> np.ndarray:
    """raw/ 에서 stem 으로 시작하는 npy 중 가장 최근 (이름순 마지막) 것을 읽는다.

    Args:
        stem: 파일 이름 앞부분 (예: latency_ros2_real)

    Returns:
        latency ms 배열 (NaN 은 뺀다)
    """
    path = sorted(glob.glob(os.path.join(RAW_DIR, f"{stem}_*.npy")))[-1]
    values = np.load(path)
    return values[~np.isnan(values)]


def main() -> None:
    """세 실행을 읽어 점 분포 그림을 저장한다."""
    rng = np.random.default_rng(0)  # 세로 흔들림 (jitter) 이 매번 같게
    fig, ax = plt.subplots(figsize=(9, 3.6), dpi=150)
    fig.patch.set_facecolor(SURFACE)
    ax.set_facecolor(SURFACE)

    for row, ((stem, label), color) in enumerate(zip(RUNS, SERIES)):
        values = latest(stem)
        y = row + rng.uniform(-0.18, 0.18, size=len(values))  # 점이 겹치지 않게 세로로 흩는다
        ax.scatter(values, y, s=14, color=color, alpha=0.55, linewidths=0, zorder=3)
        mean, p95 = values.mean(), np.percentile(values, 95)
        # mean 은 굵은 세로 눈금, p95 는 가는 눈금으로 직접 표기한다
        ax.plot([mean, mean], [row - 0.3, row + 0.3], color=color, linewidth=2, zorder=4)
        ax.plot([p95, p95], [row - 0.22, row + 0.22], color=color, linewidth=1, zorder=4)
        ax.text(
            values.max() + 2.5,
            row,
            f"mean {mean:.1f}   p95 {p95:.1f} ms   n={len(values)}",
            va="center",
            ha="left",
            fontsize=8.5,
            color=INK_2,
        )

    ax.set_yticks(range(len(RUNS)))
    ax.set_yticklabels([label for _, label in RUNS], fontsize=9, color=INK)
    ax.invert_yaxis()  # RUNS 의 첫 항목이 맨 위
    ax.set_xlim(0, 160)
    ax.set_xlabel("Latency from command to first observed motion (ms)", fontsize=9, color=INK_2)
    ax.tick_params(axis="x", labelsize=8.5, colors=MUTED, length=0)
    ax.tick_params(axis="y", length=0)
    ax.grid(axis="x", color=GRID, linewidth=0.8, zorder=0)
    for side in ["top", "right", "left"]:
        ax.spines[side].set_visible(False)
    ax.spines["bottom"].set_color(BASELINE)
    ax.set_title(
        "SO-101 shoulder_pan, 33-tick step, threshold 3 ticks, 10 ms observation "
        "(thick tick = mean, thin tick = p95)",
        fontsize=9,
        color=INK_2,
        loc="left",
    )
    fig.tight_layout()
    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
    fig.savefig(OUT_PATH, facecolor=SURFACE)
    print(f"저장: {os.path.normpath(OUT_PATH)}")


if __name__ == "__main__":
    main()
