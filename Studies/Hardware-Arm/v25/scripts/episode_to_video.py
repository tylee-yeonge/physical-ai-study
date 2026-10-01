"""LeRobot 데이터셋에서 에피소드 하나를 골라 카메라 영상을 나란히 붙인 H.264 mp4 로 뽑는다.

lerobot-record · lerobot-rollout 이 남긴 데이터셋은 카메라마다 에피소드 전체가 한 AV1 mp4 에 이어져 있다.
시연용 클립 (vla-lab 글, 1분 영상) 은 에피소드 단위의 H.264 가 필요하므로 여기서 잘라 다시 인코딩한다.

실행:
    acl
    python /workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/scripts/episode_to_video.py /root/.cache/huggingface/lerobot/tylee-yeonge/so101-pick-cube-v25 0 /workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs/clip_record_ep0.mp4

인자: 데이터셋 폴더, 에피소드 번호, 출력 mp4 경로. 카메라가 2대면 왼쪽 | 오른쪽으로 붙는다 (info.json 의 순서).
"""

import glob
import json
import os
import sys
from typing import List

import av
import numpy as np
import pandas as pd

DATASET = sys.argv[1]  # 데이터셋 폴더 (meta/ data/ videos/ 가 있는 곳)
EPISODE = int(sys.argv[2])  # 에피소드 번호 (0 부터)
OUT = sys.argv[3]  # 출력 mp4 경로
CRF = 23  # H.264 화질 (낮을수록 좋고 큼. 23 이 기본값)

info = json.load(open(os.path.join(DATASET, "meta", "info.json")))  # fps 와 카메라 키가 여기 있다
fps = int(info["fps"])
image_keys: List[str] = [k for k, f in info["features"].items() if f["dtype"] == "video"]  # 예: observation.images.front

# 에피소드별 메타 (어느 mp4 의 몇 초부터 몇 초까지가 이 에피소드인지)
episodes = pd.concat(
    pd.read_parquet(p) for p in sorted(glob.glob(os.path.join(DATASET, "meta", "episodes", "**", "*.parquet"), recursive=True))
)
row = episodes[episodes["episode_index"] == EPISODE]
if row.empty:
    sys.exit(f"에피소드 {EPISODE} 가 없다 (있는 번호: {sorted(episodes['episode_index'].tolist())})")
row = row.iloc[0]
print(f"episode {EPISODE}: {int(row['length'])} frames @ {fps} fps, cameras {image_keys}")


def read_episode_frames(key: str) -> List[np.ndarray]:
    """카메라 key 의 mp4 에서 이 에피소드 구간의 프레임만 RGB 배열로 모은다."""
    path = os.path.join(
        DATASET, "videos", key,
        f"chunk-{int(row[f'videos/{key}/chunk_index']):03d}",
        f"file-{int(row[f'videos/{key}/file_index']):03d}.mp4",
    )
    start, end = float(row[f"videos/{key}/from_timestamp"]), float(row[f"videos/{key}/to_timestamp"])
    frames: List[np.ndarray] = []
    with av.open(path) as container:
        stream = container.streams.video[0]
        for frame in container.decode(stream):
            t = float(frame.pts * stream.time_base)  # 이 프레임의 시각 (초)
            if t < start - 1e-6:
                continue  # 앞 에피소드
            if t >= end - 1e-6:
                break  # 다음 에피소드부터는 필요 없다
            frames.append(frame.to_ndarray(format="rgb24"))
    return frames


per_camera = [read_episode_frames(key) for key in image_keys]
n = min(len(f) for f in per_camera)  # 카메라마다 1-2 프레임 차이가 날 수 있어 짧은 쪽에 맞춘다
print("frames per camera:", [len(f) for f in per_camera], "-> using", n)

os.makedirs(os.path.dirname(os.path.abspath(OUT)), exist_ok=True)
with av.open(OUT, "w") as out:
    first = np.hstack([f[0] for f in per_camera])  # 가로로 붙인 첫 프레임으로 크기를 정한다
    stream = out.add_stream("libx264", rate=fps)  # 어디서나 재생되는 H.264
    stream.width, stream.height = first.shape[1], first.shape[0]
    stream.pix_fmt = "yuv420p"  # 브라우저 · 폰 호환 픽셀 포맷
    stream.options = {"crf": str(CRF), "preset": "medium"}
    for i in range(n):
        side = np.hstack([f[i] for f in per_camera])  # 카메라 순서대로 왼쪽부터
        for packet in stream.encode(av.VideoFrame.from_ndarray(side, format="rgb24")):
            out.mux(packet)
    for packet in stream.encode():  # 인코더 버퍼 비우기
        out.mux(packet)
print(f"saved {OUT} ({first.shape[1]}x{first.shape[0]}, {n / fps:.1f} s)")
