"""teleop 을 lerobot-record 로 녹화한 데이터셋에서 추종 품질을 계산한다 -- 스파이크 must 1 의 수치 증거.

무엇을 재는가:
    데이터셋의 action (리더가 보낸 목표) 과 observation.state (팔로워의 실제 위치) 를 관절별로 비교한다.
    - 지연: 팔로워를 k 프레임 뒤로 밀었을 때 리더와의 오차 (RMSE) 가 가장 작아지는 k. 1 프레임 = 1000 / fps ms
    - 추종 오차: 그 지연만큼 맞춘 뒤의 RMSE. 지연을 빼고 남는, 순수하게 못 따라간 양
    - 편향: (팔로워 - 리더) 의 평균. 중력 부하 같은 한쪽으로 쏠린 오차
    - 최대 1틱 변화: 연속한 두 프레임 사이의 최대 변화량. 값이 튀면 (인코더 경계 통과) 수백 도가 찍힌다

실행:
    acl
    python Studies/Hardware-Arm/spike/week2/scripts/analyze_teleop_tracking.py <데이터셋 폴더>
    예) .../spike/week2/outputs/evidence/so101-spike-teleop_20260919_233008
"""

import json
import sys

import numpy as np
import pandas as pd

DS = sys.argv[1]  # lerobot 데이터셋 폴더 (meta/ · data/ · videos/ 가 들어 있는 곳)
MAX_LAG = 12  # 지연을 찾아볼 최대 프레임 수 (30 fps 에서 400 ms)
MIN_MOVE = 15  # 이만큼도 안 움직인 관절은 지연을 추정하지 않는다 (도. 그리퍼는 0-100)

info = json.load(open(f"{DS}/meta/info.json"))  # fps 와 관절 이름이 여기 있다
fps = info["fps"]
names = info["features"]["action"]["names"]  # shoulder_pan.pos, ..., gripper.pos
df = pd.read_parquet(f"{DS}/data/chunk-000/file-000.parquet")  # 프레임당 1행
act = np.stack(df["action"].to_numpy())  # (프레임 수, 6) 리더가 보낸 목표
obs = np.stack(df["observation.state"].to_numpy())  # (프레임 수, 6) 팔로워 실제 위치

print(f"{DS}")
print(f"frames={len(df)} fps={fps} duration={len(df) / fps:.1f}s\n")
print(
    f"{'joint':14s} {'leader range':>17s} {'lag':>7s} {'rmse@0':>7s} {'rmse@lag':>9s} {'bias':>6s} {'max step':>9s}"
)
for j, name in enumerate(names):
    a, o = act[:, j], obs[:, j]
    # 팔로워를 k 프레임 늦춰 가며 리더와의 RMSE 를 구한다. 가장 작아지는 k 가 지연이다
    rmses = [np.sqrt(np.mean((o[k:] - a[: len(a) - k]) ** 2)) for k in range(MAX_LAG)]
    k = int(np.argmin(rmses))
    moved = a.max() - a.min()  # 이 관절을 얼마나 움직였는가
    lag = f"{k * 1000 / fps:.0f}ms" if moved > MIN_MOVE else "-"
    # 연속한 두 프레임 사이의 최대 변화. 리더 · 팔로워 중 더 크게 튄 쪽을 쓴다
    step = max(np.abs(np.diff(a)).max(), np.abs(np.diff(o)).max())
    bias = np.mean(o - a)  # 팔로워가 리더보다 평균적으로 얼마나 치우쳐 있는가
    print(
        f"{name.removesuffix('.pos'):14s} {a.min():7.1f}..{a.max():7.1f} {lag:>7s} "
        f"{rmses[0]:7.2f} {rmses[k]:9.2f} {bias:6.2f} {step:9.1f}"
    )
