# Week 6 실습: 3D Detection 성능 분석 및 개선


> **목표**: Week 5 학습 결과를 분석하여 오류 유형을 파악하고, 개선 전략을 적용하여 AP3D를 향상시킨다
> **언어**: Python (NumPy, Matplotlib, MMDetection3D)
> **예상 시간**: 10시간


---


## 실습 개요


| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 오류 분석 프레임워크 구현 | 필수 | 3시간 |
| 2 | 거리별 / 유형별 성능 분석 | 필수 | 2시간 |
| 3 | Data Augmentation 적용 | 필수 | 2시간 |
| 4 | 3D NMS 및 Depth Loss 실험 | 필수 | 3시간 |


---


## 환경 설정


```bash
# Week 5 환경 활성화
conda activate mmdet3d


# 추가 패키지
pip install numpy matplotlib opencv-python pandas seaborn
```


---


## Step 1: 오류 분석 프레임워크


### 1.1 예측 vs GT 비교 분석


```python
# error_analysis.py
"""
3D Detection 오류 분석 프레임워크
예측 결과를 GT와 비교하여 오류 유형을 분류합니다.
"""
import numpy as np
import matplotlib.pyplot as plt




def compute_iou_2d(box1, box2):
    """
    2D bbox IoU 계산


    Args:
        box1, box2: [x1, y1, x2, y2]


    Returns:
        iou: 2D IoU 값
    """
    x1 = max(box1[0], box2[0])
    y1 = max(box1[1], box2[1])
    x2 = min(box1[2], box2[2])
    y2 = min(box1[3], box2[3])


    intersection = max(0, x2 - x1) * max(0, y2 - y1)
    area1 = (box1[2] - box1[0]) * (box1[3] - box1[1])
    area2 = (box2[2] - box2[0]) * (box2[3] - box2[1])
    union = area1 + area2 - intersection


    return intersection / union if union > 0 else 0




def compute_iou_3d_axis_aligned(center1, size1, center2, size2):
    """
    축 정렬 3D bbox IoU 계산 (간소화 버전)


    Args:
        center1, center2: [x, y, z] 중심 좌표
        size1, size2: [l, w, h] 크기


    Returns:
        iou: 3D IoU 값
    """
    def overlap_1d(c1, s1, c2, s2):
        min1, max1 = c1 - s1/2, c1 + s1/2
        min2, max2 = c2 - s2/2, c2 + s2/2
        return max(0, min(max1, max2) - max(min1, min2))


    overlap_x = overlap_1d(center1[0], size1[1], center2[0], size2[1])
    overlap_y = overlap_1d(center1[1], size1[2], center2[1], size2[2])
    overlap_z = overlap_1d(center1[2], size1[0], center2[2], size2[0])


    intersection = overlap_x * overlap_y * overlap_z
    vol1 = size1[0] * size1[1] * size1[2]
    vol2 = size2[0] * size2[1] * size2[2]
    union = vol1 + vol2 - intersection


    return intersection / union if union > 0 else 0




def classify_error(pred, gt, iou_2d, iou_3d, angle_diff):
    """
    오류 유형 분류


    Args:
        pred: 예측 결과
        gt: Ground Truth
        iou_2d: 2D IoU
        iou_3d: 3D IoU
        angle_diff: rotation_y 차이 (도)


    Returns:
        error_type: 오류 유형 문자열
    """
    if iou_3d >= 0.7:
        return 'TP' # True Positive
    elif iou_2d >= 0.7 and iou_3d < 0.7:
        depth_error = abs(pred['z'] - gt['z'])
        if depth_error > 1.0:
            return 'DEPTH_ERROR'
        elif angle_diff > 15:
            return 'ORIENT_ERROR'
        else:
            return 'SIZE_ERROR'
    elif iou_2d >= 0.5:
        return 'LOCALIZATION_ERROR'
    else:
        return 'FP'




def analyze_predictions(predictions, ground_truths):
    """
    전체 예측 결과에 대한 오류 분석


    Returns:
        analysis: 오류 유형별 통계 딕셔너리
    """
    analysis = {
        'TP': 0,
        'DEPTH_ERROR': 0,
        'ORIENT_ERROR': 0,
        'SIZE_ERROR': 0,
        'LOCALIZATION_ERROR': 0,
        'FP': 0,
        'FN': 0,
        'depth_errors': [], # 깊이 오차 목록
        'angle_errors': [], # 각도 오차 목록
        'distance_bins': {}, # 거리별 성능
    }


    # GT 매칭 (간소화: greedy matching)
    matched_gt = set()


    for pred in predictions:
        best_iou_3d = 0
        best_gt_idx = -1
        best_iou_2d = 0
        best_angle_diff = 0


        for i, gt in enumerate(ground_truths):
            if i in matched_gt:
                continue


            iou_2d = compute_iou_2d(pred['bbox_2d'], gt['bbox_2d'])
            iou_3d = compute_iou_3d_axis_aligned(
                pred['location'], pred['dimensions'],
                gt['location'], gt['dimensions']
            )


            if iou_3d > best_iou_3d:
                best_iou_3d = iou_3d
                best_gt_idx = i
                best_iou_2d = iou_2d
                best_angle_diff = abs(pred['rotation_y'] - gt['rotation_y'])
                best_angle_diff = np.degrees(best_angle_diff)


        if best_gt_idx >= 0:
            matched_gt.add(best_gt_idx)
            gt = ground_truths[best_gt_idx]


            error_type = classify_error(
                pred, gt, best_iou_2d, best_iou_3d, best_angle_diff
            )
            analysis[error_type] += 1
            analysis['depth_errors'].append(abs(pred['location'][2] - gt['location'][2]))
            analysis['angle_errors'].append(best_angle_diff)


            # 거리별 분류
            dist_bin = int(gt['location'][2] // 10) * 10
            dist_key = f'{dist_bin}-{dist_bin+10}m'
            if dist_key not in analysis['distance_bins']:
                analysis['distance_bins'][dist_key] = {'TP': 0, 'total': 0}
            analysis['distance_bins'][dist_key]['total'] += 1
            if error_type == 'TP':
                analysis['distance_bins'][dist_key]['TP'] += 1
        else:
            analysis['FP'] += 1


    # 미검출 GT
    analysis['FN'] = len(ground_truths) - len(matched_gt)


    return analysis




def visualize_error_analysis(analysis):
    """오류 분석 결과 시각화"""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))


    # 1. 오류 유형 분포 (파이 차트)
    ax1 = axes[0, 0]
    error_types = ['TP', 'DEPTH_ERROR', 'ORIENT_ERROR', 'SIZE_ERROR', 'FP', 'FN']
    counts = [analysis[et] for et in error_types]
    colors = ['#2ecc71', '#e74c3c', '#f39c12', '#9b59b6', '#e67e22', '#95a5a6']
    labels = ['정확 (TP)', 'Depth 오류', 'Orientation 오류',
              'Size 오류', '오검출 (FP)', '미검출 (FN)']


    non_zero = [(l, c, col) for l, c, col in zip(labels, counts, colors) if c > 0]
    if non_zero:
        labels_nz, counts_nz, colors_nz = zip(*non_zero)
        ax1.pie(counts_nz, labels=labels_nz, colors=colors_nz,
                autopct='%1.1f%%', startangle=90)
    ax1.set_title('오류 유형 분포', fontsize=13, fontweight='bold')


    # 2. Depth 오차 히스토그램
    ax2 = axes[0, 1]
    if analysis['depth_errors']:
        ax2.hist(analysis['depth_errors'], bins=20, color='#e74c3c', alpha=0.7,
                edgecolor='black')
        ax2.axvline(x=1.0, color='red', linestyle='--', label='1m 기준선')
        ax2.set_xlabel('Depth 오차 (m)')
        ax2.set_ylabel('빈도')
        ax2.legend()
    ax2.set_title('Depth 오차 분포', fontsize=13, fontweight='bold')


    # 3. Angle 오차 히스토그램
    ax3 = axes[1, 0]
    if analysis['angle_errors']:
        ax3.hist(analysis['angle_errors'], bins=20, color='#f39c12', alpha=0.7,
                edgecolor='black')
        ax3.axvline(x=15.0, color='red', linestyle='--', label='15도 기준선')
        ax3.set_xlabel('Orientation 오차 (도)')
        ax3.set_ylabel('빈도')
        ax3.legend()
    ax3.set_title('Orientation 오차 분포', fontsize=13, fontweight='bold')


    # 4. 거리별 성능
    ax4 = axes[1, 1]
    if analysis['distance_bins']:
        bins = sorted(analysis['distance_bins'].keys())
        accuracies = []
        for b in bins:
            data = analysis['distance_bins'][b]
            acc = data['TP'] / data['total'] * 100 if data['total'] > 0 else 0
            accuracies.append(acc)
        ax4.bar(range(len(bins)), accuracies, color='#3498db', alpha=0.7)
        ax4.set_xticks(range(len(bins)))
        ax4.set_xticklabels(bins, rotation=45)
        ax4.set_ylabel('검출 성공률 (%)')
    ax4.set_title('거리별 검출 성공률', fontsize=13, fontweight='bold')


    plt.suptitle('3D Detection 오류 분석', fontsize=15, fontweight='bold')
    plt.tight_layout()
    plt.savefig('error_analysis.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("저장: error_analysis.png")




# 시뮬레이션 데이터로 테스트
if __name__ == '__main__':
    np.random.seed(42)


    # 시뮬레이션 GT
    gts = []
    for i in range(50):
        z = np.random.uniform(5, 50)
        gts.append({
            'bbox_2d': [100+i*10, 200, 250+i*10, 350],
            'location': [np.random.uniform(-5, 5), 1.65, z],
            'dimensions': [4.5, 1.8, 1.5],
            'rotation_y': np.random.uniform(-np.pi, np.pi),
        })


    # 시뮬레이션 예측 (노이즈 추가)
    preds = []
    for gt in gts[:40]:
        preds.append({
            'bbox_2d': [b + np.random.normal(0, 5) for b in gt['bbox_2d']],
            'location': [
                gt['location'][0] + np.random.normal(0, 0.5),
                gt['location'][1] + np.random.normal(0, 0.3),
                gt['location'][2] + np.random.normal(0, 2.0),
            ],
            'dimensions': [d + np.random.normal(0, 0.2) for d in gt['dimensions']],
            'rotation_y': gt['rotation_y'] + np.random.normal(0, 0.2),
            'z': gt['location'][2] + np.random.normal(0, 2.0),
        })


    analysis = analyze_predictions(preds, gts)


    print("=" * 50)
    print("3D Detection 오류 분석 결과")
    print("=" * 50)
    for key in ['TP', 'DEPTH_ERROR', 'ORIENT_ERROR', 'SIZE_ERROR', 'FP', 'FN']:
        print(f"{key}: {analysis[key]}")


    if analysis['depth_errors']:
        print(f"\n 평균 Depth 오차: {np.mean(analysis['depth_errors']):.2f}m")
        print(f"평균 Angle 오차: {np.mean(analysis['angle_errors']):.2f}도")


    visualize_error_analysis(analysis)
```


---


## Step 2: 거리별 성능 분석


### 2.1 거리 구간별 AP3D 분석


```python
# distance_analysis.py
"""
거리 구간별 3D Detection 성능 분석
어느 거리에서 성능이 급감하는지 파악합니다.
"""
import numpy as np
import matplotlib.pyplot as plt




def analyze_by_distance(predictions, ground_truths, distance_bins=None):
    """
    거리 구간별 성능 분석


    Args:
        predictions: 예측 결과 리스트
        ground_truths: GT 리스트
        distance_bins: 거리 구간 경계 (기본: [0, 10, 20, 30, 40, 50])


    Returns:
        results: 구간별 TP, FP, FN, AP3D
    """
    if distance_bins is None:
        distance_bins = [0, 10, 20, 30, 40, 50]


    results = {}
    for i in range(len(distance_bins) - 1):
        key = f"{distance_bins[i]}-{distance_bins[i+1]}m"
        results[key] = {
            'gt_count': 0,
            'pred_count': 0,
            'tp_count': 0,
            'depth_errors': [],
            'precision': 0.0,
            'recall': 0.0,
        }


    # GT를 거리별로 분류
    for gt in ground_truths:
        z = gt['location'][2]
        for i in range(len(distance_bins) - 1):
            if distance_bins[i] <= z < distance_bins[i+1]:
                key = f"{distance_bins[i]}-{distance_bins[i+1]}m"
                results[key]['gt_count'] += 1
                break


    return results




def plot_distance_performance():
    """거리별 성능 시각화 (시뮬레이션 데이터)"""
    distances = ['0-10m', '10-20m', '20-30m', '30-40m', '40-50m']
    ap3d_values = [35.2, 24.8, 12.1, 5.3, 1.2]
    depth_errors = [1.2, 1.8, 3.1, 4.5, 6.8]


    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))


    # AP3D by distance
    colors = ['#2ecc71', '#27ae60', '#f39c12', '#e74c3c', '#c0392b']
    bars = ax1.bar(distances, ap3d_values, color=colors, alpha=0.8, edgecolor='black')
    ax1.axhline(y=15, color='blue', linestyle='--', alpha=0.5, label='목표: 15%')
    ax1.set_xlabel('거리 구간')
    ax1.set_ylabel('AP3D (%)')
    ax1.set_title('거리별 AP3D (Car Moderate)', fontsize=13, fontweight='bold')
    ax1.legend()


    for bar, val in zip(bars, ap3d_values):
        ax1.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.5,
                f'{val:.1f}%', ha='center', fontsize=10, fontweight='bold')


    # Depth error by distance
    ax2.plot(distances, depth_errors, 'ro-', linewidth=2, markersize=8)
    ax2.fill_between(range(len(distances)), depth_errors, alpha=0.2, color='red')
    ax2.axhline(y=1.0, color='green', linestyle='--', alpha=0.5,
                label='IoU 0.7 허용 오차 (~1m)')
    ax2.set_xlabel('거리 구간')
    ax2.set_ylabel('평균 Depth 오차 (m)')
    ax2.set_title('거리별 평균 Depth 오차', fontsize=13, fontweight='bold')
    ax2.legend()


    plt.tight_layout()
    plt.savefig('distance_analysis.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("저장: distance_analysis.png")




if __name__ == '__main__':
    plot_distance_performance()
```


---


## Step 3: Data Augmentation 적용


### 3.1 3D 인식 Augmentation


```python
# augmentation_3d.py
"""
3D Detection을 위한 Data Augmentation
이미지 변환 시 3D 레이블 일관성을 유지합니다.
"""
import numpy as np
import cv2




def random_flip_3d(image, labels, calib_P2):
    """
    좌우 반전 Augmentation (3D 일관성 유지)


    Args:
        image: BGR 이미지
        labels: 3D 레이블 리스트
        calib_P2: 3x4 투영 행렬


    Returns:
        flipped_image, flipped_labels, flipped_P2
    """
    h, w = image.shape[:2]


    # 이미지 좌우 반전
    flipped_image = cv2.flip(image, 1)


    # 레이블 반전
    flipped_labels = []
    for label in labels:
        new_label = label.copy()


        # 2D bbox 반전
        x1, y1, x2, y2 = label['bbox_2d']
        new_label['bbox_2d'] = [w - x2, y1, w - x1, y2]


        # 3D location 반전 (x축 반전)
        new_label['location'] = [
            -label['location'][0], # x 반전
            label['location'][1], # y 유지
            label['location'][2], # z 유지
        ]


        # rotation_y 반전
        new_label['rotation_y'] = -label['rotation_y']


        # alpha 반전
        new_label['alpha'] = -label['alpha']


        flipped_labels.append(new_label)


    # 캘리브레이션 행렬 수정
    flipped_P2 = calib_P2.copy()
    flipped_P2[0, 2] = w - calib_P2[0, 2] # cx 반전
    flipped_P2[0, 3] = -calib_P2[0, 3] # tx 반전


    return flipped_image, flipped_labels, flipped_P2




def color_jitter(image, brightness=0.3, contrast=0.3, saturation=0.3):
    """
    색상 변환 Augmentation


    Args:
        image: BGR 이미지
        brightness: 밝기 변화 범위
        contrast: 대비 변화 범위
        saturation: 채도 변화 범위


    Returns:
        augmented_image
    """
    img = image.astype(np.float32)


    # 밝기 변화
    beta = np.random.uniform(-brightness, brightness) * 255
    img = np.clip(img + beta, 0, 255)


    # 대비 변화
    alpha = np.random.uniform(1 - contrast, 1 + contrast)
    mean = img.mean()
    img = np.clip((img - mean) * alpha + mean, 0, 255)


    # 채도 변화
    hsv = cv2.cvtColor(img.astype(np.uint8), cv2.COLOR_BGR2HSV).astype(np.float32)
    s_factor = np.random.uniform(1 - saturation, 1 + saturation)
    hsv[:, :, 1] = np.clip(hsv[:, :, 1] * s_factor, 0, 255)
    img = cv2.cvtColor(hsv.astype(np.uint8), cv2.COLOR_HSV2BGR)


    return img.astype(np.uint8)




def demo_augmentation():
    """Augmentation 데모"""
    # 더미 이미지 생성
    image = np.random.randint(100, 200, (375, 1242, 3), dtype=np.uint8)


    # 더미 레이블
    labels = [{
        'type': 'Car',
        'bbox_2d': [300, 150, 500, 300],
        'location': [2.0, 1.65, 15.0],
        'dimensions': [4.5, 1.8, 1.5],
        'rotation_y': 0.3,
        'alpha': 0.2,
    }]


    # 더미 캘리브레이션
    P2 = np.array([
        [721.54, 0, 609.56, 44.85],
        [0, 721.54, 172.85, 0.22],
        [0, 0, 1, 0.003],
    ])


    print("=== 3D Augmentation 데모 ===")
    print(f"원본 레이블:")
    print(f"location: {labels[0]['location']}")
    print(f"rotation_y: {labels[0]['rotation_y']:.3f}")


    flipped_img, flipped_labels, flipped_P2 = random_flip_3d(image, labels, P2)


    print(f"\n반전 후 레이블:")
    print(f"location: {flipped_labels[0]['location']}")
    print(f"rotation_y: {flipped_labels[0]['rotation_y']:.3f}")


    print(f"\n핵심:")
    print(f"x: {labels[0]['location'][0]} -> {flipped_labels[0]['location'][0]}")
    print(f"ry: {labels[0]['rotation_y']:.3f} -> {flipped_labels[0]['rotation_y']:.3f}")
    print(f"좌우 반전 시 x와 ry 모두 부호가 바뀌었음을 확인!")




if __name__ == '__main__':
    demo_augmentation()
```


---


## Step 4: 3D NMS 및 Depth Loss 실험


### 4.1 3D NMS 비교


```python
# nms_comparison.py
"""
2D NMS vs 3D NMS 비교 실험
3D NMS가 깊이가 다른 객체를 어떻게 보존하는지 시각화합니다.
"""
import numpy as np
import matplotlib.pyplot as plt




def nms_2d(detections, iou_threshold=0.5):
    """2D NMS 구현"""
    if len(detections) == 0:
        return []


    boxes = np.array([d['bbox_2d'] for d in detections])
    scores = np.array([d['score'] for d in detections])


    x1, y1, x2, y2 = boxes[:, 0], boxes[:, 1], boxes[:, 2], boxes[:, 3]
    areas = (x2 - x1) * (y2 - y1)


    order = scores.argsort()[::-1]
    keep = []


    while order.size > 0:
        i = order[0]
        keep.append(i)


        xx1 = np.maximum(x1[i], x1[order[1:]])
        yy1 = np.maximum(y1[i], y1[order[1:]])
        xx2 = np.minimum(x2[i], x2[order[1:]])
        yy2 = np.minimum(y2[i], y2[order[1:]])


        w = np.maximum(0, xx2 - xx1)
        h = np.maximum(0, yy2 - yy1)
        inter = w * h


        iou = inter / (areas[i] + areas[order[1:]] - inter)
        inds = np.where(iou <= iou_threshold)[0]
        order = order[inds + 1]


    return [detections[i] for i in keep]




def nms_bev(detections, iou_threshold=0.25):
    """BEV(Bird's Eye View) NMS 구현"""
    if len(detections) == 0:
        return []


    scores = np.array([d['score'] for d in detections])
    order = scores.argsort()[::-1]
    keep = []


    while order.size > 0:
        i = order[0]
        keep.append(i)


        remaining = order[1:]
        inds_to_keep = []


        for j_idx, j in enumerate(remaining):
            # BEV IoU 계산 (x-z 평면)
            d1 = detections[i]
            d2 = detections[j]


            # BEV 겹침 계산
            x_overlap = max(0, min(d1['location'][0] + d1['dimensions'][1]/2,
                                   d2['location'][0] + d2['dimensions'][1]/2)
                           - max(d1['location'][0] - d1['dimensions'][1]/2,
                                 d2['location'][0] - d2['dimensions'][1]/2))
            z_overlap = max(0, min(d1['location'][2] + d1['dimensions'][0]/2,
                                   d2['location'][2] + d2['dimensions'][0]/2)
                           - max(d1['location'][2] - d1['dimensions'][0]/2,
                                 d2['location'][2] - d2['dimensions'][0]/2))


            bev_inter = x_overlap * z_overlap
            bev_area1 = d1['dimensions'][0] * d1['dimensions'][1]
            bev_area2 = d2['dimensions'][0] * d2['dimensions'][1]
            bev_union = bev_area1 + bev_area2 - bev_inter


            bev_iou = bev_inter / bev_union if bev_union > 0 else 0


            if bev_iou <= iou_threshold:
                inds_to_keep.append(j_idx)


        order = remaining[inds_to_keep]


    return [detections[i] for i in keep]




def compare_nms():
    """2D NMS vs BEV NMS 비교 시각화"""
    # 시나리오: 다른 깊이에 있는 두 차량 (이미지에서는 겹침)
    detections = [
        {
            'bbox_2d': [300, 200, 500, 350],
            'location': [1.0, 1.65, 10.0],
            'dimensions': [4.5, 1.8, 1.5],
            'score': 0.92,
            'label': 'Car A (z=10m)',
        },
        {
            'bbox_2d': [310, 210, 490, 340], # 2D에서 많이 겹침
            'location': [0.5, 1.65, 25.0], # 하지만 깊이가 다름!
            'dimensions': [4.5, 1.8, 1.5],
            'score': 0.85,
            'label': 'Car B (z=25m)',
        },
        {
            'bbox_2d': [600, 250, 750, 380],
            'location': [-3.0, 1.65, 15.0],
            'dimensions': [4.5, 1.8, 1.5],
            'score': 0.78,
            'label': 'Car C (z=15m)',
        },
    ]


    result_2d = nms_2d(detections, iou_threshold=0.5)
    result_bev = nms_bev(detections, iou_threshold=0.25)


    print("=== NMS 비교 결과 ===\n")
    print(f"입력 검출 수: {len(detections)}")
    print(f"2D NMS 후: {len(result_2d)}개 유지")
    for d in result_2d:
        print(f"- {d['label']} (score={d['score']:.2f})")


    print(f"\nBEV NMS 후: {len(result_bev)}개 유지")
    for d in result_bev:
        print(f"- {d['label']} (score={d['score']:.2f})")


    # 시각화
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))


    # 원본
    ax = axes[0]
    ax.set_xlim(0, 1000)
    ax.set_ylim(500, 0)
    ax.set_title(f'원본 ({len(detections)}개)', fontsize=13, fontweight='bold')
    colors = ['green', 'blue', 'red']
    for d, c in zip(detections, colors):
        x1, y1, x2, y2 = d['bbox_2d']
        rect = plt.Rectangle((x1, y1), x2-x1, y2-y1, fill=False,
                             edgecolor=c, linewidth=2)
        ax.add_patch(rect)
        ax.text(x1, y1-5, d['label'], color=c, fontsize=9, fontweight='bold')


    # 2D NMS
    ax = axes[1]
    ax.set_xlim(0, 1000)
    ax.set_ylim(500, 0)
    ax.set_title(f'2D NMS 후 ({len(result_2d)}개)', fontsize=13, fontweight='bold')
    for d in result_2d:
        idx = detections.index(d)
        x1, y1, x2, y2 = d['bbox_2d']
        rect = plt.Rectangle((x1, y1), x2-x1, y2-y1, fill=False,
                             edgecolor=colors[idx], linewidth=2)
        ax.add_patch(rect)
        ax.text(x1, y1-5, d['label'], color=colors[idx], fontsize=9, fontweight='bold')


    # BEV NMS
    ax = axes[2]
    ax.set_xlim(0, 1000)
    ax.set_ylim(500, 0)
    ax.set_title(f'BEV NMS 후 ({len(result_bev)}개)', fontsize=13, fontweight='bold')
    for d in result_bev:
        idx = detections.index(d)
        x1, y1, x2, y2 = d['bbox_2d']
        rect = plt.Rectangle((x1, y1), x2-x1, y2-y1, fill=False,
                             edgecolor=colors[idx], linewidth=2)
        ax.add_patch(rect)
        ax.text(x1, y1-5, d['label'], color=colors[idx], fontsize=9, fontweight='bold')


    plt.suptitle('2D NMS vs BEV NMS: 깊이가 다른 객체 보존', fontsize=14, fontweight='bold')
    plt.tight_layout()
    plt.savefig('nms_comparison.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("\n 저장: nms_comparison.png")




if __name__ == '__main__':
    compare_nms()
```


### 4.2 Ablation Study 결과 정리


```python
# ablation_study.py
"""
개선 기법별 성능 기여도를 정리하고 시각화합니다.
"""
import matplotlib.pyplot as plt
import numpy as np




def plot_ablation_study():
    """Ablation Study 결과 시각화"""


    experiments = [
        ('Baseline', 13.0),
        ('+ Multi-scale', 14.5),
        ('+ Augmentation', 15.5),
        ('+ 3D NMS', 16.0),
        ('+ Depth Loss', 17.0),
    ]


    names = [e[0] for e in experiments]
    values = [e[1] for e in experiments]


    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))


    # 누적 성능 그래프
    colors = ['#3498db', '#2ecc71', '#f39c12', '#e74c3c', '#9b59b6']
    bars = ax1.bar(range(len(names)), values, color=colors, alpha=0.8, edgecolor='black')
    ax1.axhline(y=15, color='red', linestyle='--', alpha=0.7, label='목표: 15%')
    ax1.set_xticks(range(len(names)))
    ax1.set_xticklabels(names, rotation=30, ha='right')
    ax1.set_ylabel('Car Moderate AP3D (%)')
    ax1.set_title('Ablation Study: 누적 성능', fontsize=13, fontweight='bold')
    ax1.legend()


    for bar, val in zip(bars, values):
        ax1.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.2,
                f'{val:.1f}%', ha='center', fontsize=10, fontweight='bold')


    # 각 기법의 기여도
    contributions = [0] + [values[i] - values[i-1] for i in range(1, len(values))]
    contrib_names = ['Baseline'] + [n.replace('+ ', '') for n in names[1:]]


    bars2 = ax2.barh(range(len(contrib_names)), contributions,
                     color=colors, alpha=0.8, edgecolor='black')
    ax2.set_yticks(range(len(contrib_names)))
    ax2.set_yticklabels(contrib_names)
    ax2.set_xlabel('AP3D 향상 (%p)')
    ax2.set_title('각 기법의 성능 기여도', fontsize=13, fontweight='bold')


    for bar, val in zip(bars2, contributions):
        if val > 0:
            ax2.text(bar.get_width() + 0.1, bar.get_y() + bar.get_height()/2,
                    f'+{val:.1f}%p', va='center', fontsize=10, fontweight='bold')


    plt.tight_layout()
    plt.savefig('ablation_study.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("저장: ablation_study.png")




if __name__ == '__main__':
    plot_ablation_study()
```


---


## 체크리스트


### 오류 분석
- [ ] 예측 결과를 TP / Depth Error / Orient Error / FP / FN으로 분류
- [ ] Depth 오차 분포 히스토그램 시각화
- [ ] Orientation 오차 분포 확인
- [ ] 거리별 AP3D 분석


### 성능 개선
- [ ] Random Flip Augmentation (3D 일관성 유지) 적용
- [ ] Color Jitter Augmentation 적용
- [ ] 3D NMS vs 2D NMS 비교 실험
- [ ] NMS threshold 튜닝


### Ablation Study
- [ ] Baseline 성능 기록
- [ ] 각 개선 기법별 성능 기록
- [ ] 기여도 표 작성
- [ ] 시각화 (그래프) 저장


---


**다음**: [Week 7 - nuScenes 데이터셋](../week7/PRACTICE.md)
