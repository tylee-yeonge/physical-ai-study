"""
Phase 2 - Week 1: 핀홀 카메라 모델 기초
======================================
3D → 2D 투영 구현 및 시각화

학습 목표:
1. 내부/외부 파라미터 이해
2. 3D → 2D 투영 구현
3. 시야각(FOV) 계산
4. 재투영 오차 계산

실행 시간: 약 1분
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

np.set_printoptions(precision=4, suppress=True)

print("=" * 70)
print("        Phase 2 - Week 1: 핀홀 카메라 모델 기초")
print("=" * 70)
print("\n💡 이 실습에서는 3D 점을 2D 이미지로 투영하는 과정을 배웁니다.\n")

# ============================================================
# Part 1: 카메라 파라미터 정의
# ============================================================
print("\n" + "=" * 70)
print("Part 1: 카메라 파라미터 정의")
print("=" * 70)

print("""
📷 카메라 파라미터는 두 종류로 나뉩니다:

1. 내부 파라미터 (Intrinsic) - K 행렬
   - fx, fy: 초점 거리 (픽셀 단위)
   - cx, cy: 주점 (이미지 중심)
   - 카메라 고유값, 캘리브레이션으로 측정

2. 외부 파라미터 (Extrinsic) - [R|t]
   - R: 3x3 회전 행렬 (카메라 방향)
   - t: 3x1 이동 벡터 (카메라 위치)
   - 매 프레임마다 변화
""")

# 내부 파라미터 (Intrinsic Matrix K)
# 일반적인 카메라 값 사용
image_width = 640
image_height = 480
fx = 500.0  # 초점 거리 (픽셀)
fy = 500.0  # 대부분 fx ≈ fy
cx = image_width / 2   # 320
cy = image_height / 2  # 240

K = np.array([
    [fx,  0, cx],
    [ 0, fy, cy],
    [ 0,  0,  1]
])

print("내부 파라미터 K (Intrinsic Matrix):")
print(K)
print(f"\n  fx = {fx} (X축 초점 거리)")
print(f"  fy = {fy} (Y축 초점 거리)")
print(f"  cx = {cx} (주점 X)")
print(f"  cy = {cy} (주점 Y)")
print(f"  이미지 크기: {image_width} x {image_height}")

# 외부 파라미터 (Extrinsic [R|t])
# 카메라가 원점에서 Z축 방향을 바라보고 있다고 가정
R = np.eye(3)  # 회전 없음
t = np.array([[0], [0], [0]])  # 이동 없음 (카메라가 원점에 있음)

print("\n외부 파라미터 [R|t] (Extrinsic):")
print(f"R (회전 행렬):\n{R}")
print(f"t (이동 벡터): {t.flatten()}")
print("→ 카메라가 월드 원점에서 Z축 방향을 바라봄")

# ============================================================
# Part 2: 투영 함수 구현
# ============================================================
print("\n" + "=" * 70)
print("Part 2: 3D → 2D 투영 함수")
print("=" * 70)

print("""
투영 과정 3단계:

1️⃣ 월드 → 카메라: Pc = R · Pw + t
2️⃣ 카메라 → 정규화: (x', y') = (Xc/Zc, Yc/Zc)
3️⃣ 정규화 → 픽셀: (u, v) = (fx·x'+cx, fy·y'+cy)
""")

def project_point(P_world, R, t, K):
    """
    3D 월드 점을 2D 픽셀로 투영
    
    Args:
        P_world: (3,) array - 3D 점 [X, Y, Z]
        R: (3, 3) array - 회전 행렬
        t: (3, 1) array - 이동 벡터
        K: (3, 3) array - 내부 파라미터 행렬
    
    Returns:
        pixel: (2,) array - [u, v] 픽셀 좌표
        P_camera: (3,) array - 카메라 좌표계의 3D 점
    """
    P_world = np.array(P_world).flatten()
    
    # Step 1: 월드 → 카메라
    P_camera = R @ P_world + t.flatten()
    
    # Step 2: 카메라 → 정규화 이미지 (원근 투영)
    Zc = P_camera[2]
    if Zc <= 0:
        # 점이 카메라 뒤에 있음 - 투영 불가
        return None, P_camera
    
    x_normalized = P_camera[0] / Zc
    y_normalized = P_camera[1] / Zc
    
    # Step 3: 정규화 → 픽셀
    fx, fy = K[0, 0], K[1, 1]
    cx, cy = K[0, 2], K[1, 2]
    
    u = fx * x_normalized + cx
    v = fy * y_normalized + cy
    
    return np.array([u, v]), P_camera

def project_points(points_world, R, t, K):
    """여러 3D 점을 투영"""
    pixels = []
    points_camera = []
    
    for P in points_world:
        pixel, P_cam = project_point(P, R, t, K)
        if pixel is not None:
            pixels.append(pixel)
            points_camera.append(P_cam)
    
    return np.array(pixels), np.array(points_camera)

# 테스트: 단일 점 투영
P_test = np.array([1.0, 0.5, 5.0])  # 카메라 앞 5m, 오른쪽 1m, 위 0.5m
pixel, P_cam = project_point(P_test, R, t, K)

print(f"\n예시: 3D 점 투영")
print(f"  3D 월드 점: {P_test}")
print(f"  3D 카메라 점: {P_cam}")
print(f"  정규화 좌표: ({P_cam[0]/P_cam[2]:.4f}, {P_cam[1]/P_cam[2]:.4f})")
print(f"  2D 픽셀: ({pixel[0]:.1f}, {pixel[1]:.1f})")
print(f"  이미지 내부? {0 <= pixel[0] < image_width and 0 <= pixel[1] < image_height}")

# ============================================================
# Part 3: 3D 정육면체 투영
# ============================================================
print("\n" + "=" * 70)
print("Part 3: 3D 정육면체 투영")
print("=" * 70)

# 카메라 앞 5m에 1m 정육면체 생성
cube_center = np.array([0, 0, 5])
cube_size = 1.0

# 정육면체 꼭짓점
cube_vertices = np.array([
    [-0.5, -0.5, -0.5],
    [ 0.5, -0.5, -0.5],
    [ 0.5,  0.5, -0.5],
    [-0.5,  0.5, -0.5],
    [-0.5, -0.5,  0.5],
    [ 0.5, -0.5,  0.5],
    [ 0.5,  0.5,  0.5],
    [-0.5,  0.5,  0.5],
]) * cube_size + cube_center

# 정육면체 모서리 (edge 연결)
cube_edges = [
    (0, 1), (1, 2), (2, 3), (3, 0),  # 앞면
    (4, 5), (5, 6), (6, 7), (7, 4),  # 뒷면
    (0, 4), (1, 5), (2, 6), (3, 7)   # 연결
]

# 투영
pixels_cube, _ = project_points(cube_vertices, R, t, K)

print(f"정육면체 중심: {cube_center}")
print(f"정육면체 크기: {cube_size}m")
print(f"꼭짓점 개수: {len(cube_vertices)}")
print(f"\n투영된 픽셀 좌표:")
for i, (p3d, p2d) in enumerate(zip(cube_vertices, pixels_cube)):
    print(f"  점 {i}: {p3d} → ({p2d[0]:.1f}, {p2d[1]:.1f})")

# Plot projected pixel coordinates
fig_proj = plt.figure(figsize=(8, 6))
ax_proj = fig_proj.add_subplot(111)
ax_proj.set_title('Projected Cube Vertices (Pixel Coordinates)', fontsize=14, pad=20)
ax_proj.set_xlim([0, image_width])
ax_proj.set_ylim([image_height, 0])  # Flip Y-axis (image coordinates)
ax_proj.set_xlabel('u (pixels)', fontsize=12)
ax_proj.set_ylabel('v (pixels)', fontsize=12)
ax_proj.set_aspect('equal')
ax_proj.grid(True, alpha=0.3)

# Image boundaries
ax_proj.axhline(y=0, color='gray', linestyle='-', linewidth=1.5)
ax_proj.axhline(y=image_height, color='gray', linestyle='-', linewidth=1.5)
ax_proj.axvline(x=0, color='gray', linestyle='-', linewidth=1.5)
ax_proj.axvline(x=image_width, color='gray', linestyle='-', linewidth=1.5)

# Principal point
ax_proj.scatter([cx], [cy], c='red', s=150, marker='+', linewidths=3, 
               label='Principal Point', zorder=5)

# Projected points
ax_proj.scatter(pixels_cube[:, 0], pixels_cube[:, 1], c='blue', s=100, 
               alpha=0.7, edgecolors='navy', linewidths=2, zorder=4)

# Add point labels
for i, pixel in enumerate(pixels_cube):
    ax_proj.annotate(f'{i}', (pixel[0], pixel[1]), 
                    xytext=(5, 5), textcoords='offset points',
                    fontsize=10, color='darkblue', fontweight='bold')

# Draw cube edges
for edge in cube_edges:
    p1, p2 = pixels_cube[edge[0]], pixels_cube[edge[1]]
    ax_proj.plot([p1[0], p2[0]], [p1[1], p2[1]], 'b-', 
                linewidth=1.5, alpha=0.5, zorder=3)

ax_proj.legend(loc='upper right', fontsize=11)
plt.tight_layout()
plt.savefig('projected_pixels.png', dpi=150, bbox_inches='tight')
print("\nProjected pixel coordinates plot saved: projected_pixels.png")
plt.close(fig_proj)

# ============================================================
# Part 4: 시각화
# ============================================================
print("\n" + "=" * 70)
print("Part 4: 시각화")
print("=" * 70)

fig = plt.figure(figsize=(14, 5))

# 1. 3D 뷰
ax1 = fig.add_subplot(131, projection='3d')
ax1.set_title('3D World View')

# 정육면체 그리기
for edge in cube_edges:
    points = cube_vertices[[edge[0], edge[1]]]
    ax1.plot3D(points[:, 0], points[:, 1], points[:, 2], 'b-', linewidth=2)

# 카메라 위치 (원점)
ax1.scatter([0], [0], [0], c='red', s=100, marker='^', label='Camera')

# 카메라 시야 표시 (피라미드)
fov_scale = 2.0
corners_2d = [[-1, -1], [1, -1], [1, 1], [-1, 1]]
for corner in corners_2d:
    ax1.plot3D([0, corner[0]*fov_scale], [0, corner[1]*fov_scale], 
               [0, fov_scale], 'r--', alpha=0.3)

ax1.set_xlabel('X'); ax1.set_ylabel('Y'); ax1.set_zlabel('Z')
ax1.set_xlim([-3, 3]); ax1.set_ylim([-3, 3]); ax1.set_zlim([0, 7])
ax1.legend()

# 2. 투영된 이미지
ax2 = fig.add_subplot(132)
ax2.set_title('Projected Image')
ax2.set_xlim([0, image_width])
ax2.set_ylim([image_height, 0])  # Y축 뒤집기 (이미지 좌표)
ax2.set_xlabel('u (pixels)'); ax2.set_ylabel('v (pixels)')
ax2.set_aspect('equal')
ax2.grid(True, alpha=0.3)

# 투영된 정육면체 그리기
for edge in cube_edges:
    p1, p2 = pixels_cube[edge[0]], pixels_cube[edge[1]]
    ax2.plot([p1[0], p2[0]], [p1[1], p2[1]], 'b-', linewidth=2)

ax2.scatter(pixels_cube[:, 0], pixels_cube[:, 1], c='blue', s=50)

# 이미지 경계
ax2.axhline(y=0, color='gray', linestyle='-', linewidth=1)
ax2.axhline(y=image_height, color='gray', linestyle='-', linewidth=1)
ax2.axvline(x=0, color='gray', linestyle='-', linewidth=1)
ax2.axvline(x=image_width, color='gray', linestyle='-', linewidth=1)

# 주점 표시
ax2.scatter([cx], [cy], c='red', s=100, marker='+', linewidths=2, label='Principal Point')
ax2.legend()

# 3. 다른 거리에서의 투영 비교
ax3 = fig.add_subplot(133)
ax3.set_title('Effect of Distance')
ax3.set_xlim([0, image_width])
ax3.set_ylim([image_height, 0])
ax3.set_xlabel('u (pixels)'); ax3.set_ylabel('v (pixels)')
ax3.set_aspect('equal')
ax3.grid(True, alpha=0.3)

distances = [3, 5, 10]
colors = ['red', 'blue', 'green']

for dist, color in zip(distances, colors):
    cube_at_dist = cube_vertices - cube_center + np.array([0, 0, dist])
    pixels_at_dist, _ = project_points(cube_at_dist, R, t, K)
    
    for edge in cube_edges:
        p1, p2 = pixels_at_dist[edge[0]], pixels_at_dist[edge[1]]
        ax3.plot([p1[0], p2[0]], [p1[1], p2[1]], color=color, linewidth=1.5)
    
    ax3.scatter([], [], c=color, label=f'Distance: {dist}m')

ax3.legend()

plt.tight_layout()
plt.savefig('projection_visualization.png', dpi=150)
print("\n시각화 저장: projection_visualization.png")
print("→ 멀리 있을수록 이미지에서 작게 보임 (원근 효과)")

# ============================================================
# Part 5: 시야각 (Field of View) 계산
# ============================================================
print("\n" + "=" * 70)
print("Part 5: 시야각 (FOV) 계산")
print("=" * 70)

print("""
📐 시야각 공식:

    FOV_x = 2 × arctan(width / (2 × fx))
    FOV_y = 2 × arctan(height / (2 × fy))
    
직관적 이해:
- fx 클수록 → FOV 작음 (망원, 줌 인)
- fx 작을수록 → FOV 큼 (광각, 줌 아웃)
""")

def calculate_fov(K, image_size):
    """시야각 계산"""
    fx, fy = K[0, 0], K[1, 1]
    width, height = image_size
    
    fov_x = 2 * np.arctan(width / (2 * fx))
    fov_y = 2 * np.arctan(height / (2 * fy))
    
    return np.degrees(fov_x), np.degrees(fov_y)

fov_x, fov_y = calculate_fov(K, (image_width, image_height))

print(f"\n현재 카메라 설정:")
print(f"  fx = {fx}, 이미지 너비 = {image_width}")
print(f"  fy = {fy}, 이미지 높이 = {image_height}")
print(f"\n시야각:")
print(f"  수평 FOV: {fov_x:.1f}°")
print(f"  수직 FOV: {fov_y:.1f}°")

# 다양한 초점 거리 비교
print("\n다양한 초점 거리와 시야각:")
print(f"{'fx':>8} | {'FOV_x':>10} | 특성")
print("-" * 35)

for fx_test in [300, 500, 800, 1200]:
    K_test = np.array([[fx_test, 0, cx], [0, fx_test, cy], [0, 0, 1]])
    fov_test, _ = calculate_fov(K_test, (image_width, image_height))
    
    if fx_test < 400:
        desc = "광각"
    elif fx_test < 700:
        desc = "표준"
    else:
        desc = "망원"
    
    print(f"{fx_test:>8} | {fov_test:>9.1f}° | {desc}")

# ============================================================
# Part 6: 재투영 오차
# ============================================================
print("\n" + "=" * 70)
print("Part 6: 재투영 오차 (Reprojection Error)")
print("=" * 70)

print("""
🎯 재투영 오차란?

실제 관측된 2D 점과 3D 점을 투영한 위치의 차이입니다.

    error = || projected_2d - observed_2d ||

Bundle Adjustment에서 최소화하는 핵심 비용 함수!
""")

def reprojection_error(P_3d, observed_2d, R, t, K):
    """
    재투영 오차 계산
    
    Args:
        P_3d: 3D 점
        observed_2d: 실제 관측된 2D 점
        R, t, K: 카메라 파라미터
    
    Returns:
        error: 유클리드 거리 (픽셀)
    """
    projected, _ = project_point(P_3d, R, t, K)
    if projected is None:
        return np.inf
    
    error = np.linalg.norm(projected - observed_2d)
    return error

# 테스트: 노이즈가 있는 관측
np.random.seed(42)

P_true = np.array([1.0, 0.5, 5.0])
pixel_true, _ = project_point(P_true, R, t, K)

# 노이즈 추가
noise = np.random.randn(2) * 2  # 2픽셀 표준편차
pixel_observed = pixel_true + noise

# 재투영 오차
error = reprojection_error(P_true, pixel_observed, R, t, K)

print(f"\n예시:")
print(f"  3D 점: {P_true}")
print(f"  실제 투영: ({pixel_true[0]:.2f}, {pixel_true[1]:.2f})")
print(f"  관측 (노이즈 포함): ({pixel_observed[0]:.2f}, {pixel_observed[1]:.2f})")
print(f"  재투영 오차: {error:.2f} 픽셀")

# 여러 점의 평균 재투영 오차
errors = []
for P in cube_vertices:
    proj, _ = project_point(P, R, t, K)
    obs = proj + np.random.randn(2) * 1.5
    err = reprojection_error(P, obs, R, t, K)
    errors.append(err)

print(f"\n정육면체 8개 점의 재투영 오차:")
print(f"  평균: {np.mean(errors):.2f} 픽셀")
print(f"  최대: {np.max(errors):.2f} 픽셀")
print(f"  최소: {np.min(errors):.2f} 픽셀")

print("""
💡 좋은 재투영 오차 기준:
   < 0.5 픽셀: 매우 좋음
   < 1.0 픽셀: 좋음
   < 2.0 픽셀: 보통
   > 3.0 픽셀: 문제 있음 (캘리브레이션 확인 필요)
""")

# ============================================================
# 정리
# ============================================================
print("\n" + "=" * 70)
print("📚 Week 1 정리")
print("=" * 70)

print("""
✅ Part 1: 카메라 파라미터
   - 내부 파라미터 K (fx, fy, cx, cy) - 카메라 고유
   - 외부 파라미터 [R|t] - 카메라 포즈

✅ Part 2: 투영 함수
   - 월드 → 카메라 → 정규화 → 픽셀
   - 핵심: Z로 나누기 (원근 효과)

✅ Part 3-4: 3D 객체 투영
   - 정육면체 → 이미지
   - 거리에 따른 크기 변화

✅ Part 5: 시야각 (FOV)
   - FOV = 2 × arctan(size / 2f)
   - 초점 거리와 반비례

✅ Part 6: 재투영 오차
   - SLAM 최적화의 핵심 비용 함수
   - 목표: 1픽셀 이하

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

🎯 핵심 메시지:
   3D 점을 2D로 투영하는 것은
   K (내부) × [R|t] (외부) × P (3D점) 의 행렬 연산!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

🎯 다음: pinhole_quiz.py → Week 2: 왜곡과 캘리브레이션
""")

print("\n" + "=" * 70)
print("pinhole_basics.py 실행 완료! 🎉")
print("=" * 70)
