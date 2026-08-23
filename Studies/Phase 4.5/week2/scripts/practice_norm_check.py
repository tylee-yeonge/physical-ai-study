import glob
import json
import os

candidates = glob.glob(os.path.expanduser("~/tensorflow_datasets/maniskill_pickcube/**/dataset_statistics_*.json"), recursive=True)

for path in candidates:
    print("  ", path)

STATS_PATH = candidates[0]
with open(STATS_PATH) as f:
    stats = json.load(f)

print("통계 키:", list(stats.keys()))
action_stats = stats["action"]
for name, value in action_stats.items():
    print(f"{name}: {value}")

mask = action_stats.get("mask")
print("정규화 마스크: ", mask)

print("시각 비교")
print("통계 파일: ", os.path.getmtime(STATS_PATH))
data_files=glob.glob(os.path.expanduser("~/tensorflow_datasets/maniskill_pickcube/**/*.tfrecord*"), recursive=True)

if data_files:
    print("데이터 파일:", max(os.path.getmtime(p) for p in data_files))