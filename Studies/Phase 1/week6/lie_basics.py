"""
Phase 1 - Week 6: Lie 군/대수 기초
==================================
SO(3), SE(3)와 exp/log 매핑 실습

학습 목표:
1. Over-parameterized 문제 이해
2. Skew-symmetric 행렬 변환 (skew, vee)
3. exp/log 매핑 구현 및 검증
4. 최적화에서의 활용 이해
5. SE(3) exp 매핑 구현

실행 시간: 약 1분
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

np.set_printoptions(precision=4, suppress=True)

print("=" * 70)
print("            Phase 1 - Week 6: Lie 군/대수 기초")
print("=" * 70)
print("\n💡 이 실습은 README.md의 내용을 코드로 확인합니다")
print("   각 섹션을 천천히 읽으며 이해하세요!\n")

# ============================================================
# Part 1: Over-parameterized 문제 시연
# ============================================================
print("\n" + "=" * 70)
print("Part 1: Why Lie Groups/Algebras? (Over-parameterization 문제)")
print("=" * 70)

print("""
🤔 문제 상황: 회전을 어떻게 표현하고 업데이트할까?

회전 표현의 파라미터 수 vs 실제 자유도:

┌──────────────┬──────────┬──────────┬──────────┬──────────┐
│ 표현 방법     │ 파라미터 │ 실제자유도│ 제약조건 │ 평가     │
├──────────────┼──────────┼──────────┼──────────┼──────────┤
│ 회전 행렬 R  │    9     │    3     │ RᵀR=I    │ ❌ 비효율│
│ 쿼터니언 q   │    4     │    3     │ ‖q‖=1    │ ⚠️ 제약  │
│ Lie 대수 ω   │    3     │    3     │ 없음!    │ ✅ 이상적│
└──────────────┴──────────┴──────────┴──────────┴──────────┘

왜 자유도가 3개?
- 3D 회전 = 어떤 축 주위로 얼마나 회전? 
  → 축 방향(2 DOF) + 회전 각도(1 DOF) = 3 DOF
""")

# 잘못된 방법 시연
print("\n" + "-" * 70)
print("❌ 잘못된 방법: 회전 행렬에 단순 덧셈")
print("-" * 70)

R_identity = np.eye(3)
delta_R = np.array([[0.1, 0, 0], 
                     [0, 0.1, 0], 
                     [0, 0, 0.1]])

R_wrong = R_identity + delta_R

print(f"\n초기 R (단위 행렬):")
print(R_identity)
print(f"\n작은 변화 ΔR:")
print(delta_R)
print(f"\nR_wrong = R + ΔR:")
print(R_wrong)

# 회전 행렬 조건 검증
RtR = R_wrong.T @ R_wrong
det_R = np.linalg.det(R_wrong)

print(f"\n검증: RᵀR =")
print(RtR)
print(f"\nRᵀR = I? {np.allclose(RtR, np.eye(3))}  ❌")
print(f"det(R) = {det_R:.4f}  (1이어야 함)  ❌")
print("\n⚠️ 결과가 회전 행렬이 아닙니다!")

print("""
문제점:
- R + ΔR은 회전 행렬의 조건(RᵀR=I, det=1)을 만족하지 않음
- 포인트를 변환하면 크기가 변하거나 찌그러짐
- 매 스텝마다 투영(projection) 필요 → 비효율적
""")

# ============================================================
# Part 2: Skew-symmetric 행렬
# ============================================================
print("\n" + "=" * 70)
print("Part 2: Skew-symmetric Matrix (반대칭 행렬)")
print("=" * 70)

print("""
📐 so(3)의 두 가지 표현:
1. 벡터 형태: ω = [ω₁, ω₂, ω₃]ᵀ ∈ ℝ³
2. 행렬 형태: ω^ (hat) ∈ ℝ³ˣ³ (반대칭 행렬)

변환:
- hat (^): 벡터 → 반대칭 행렬
- vee (∨): 반대칭 행렬 → 벡터
""")

def skew(w):
    """
    벡터를 반대칭 행렬로 변환 (hat 연산자 ^)
    
    Args:
        w: 3D 벡터 [w1, w2, w3]
    
    Returns:
        3x3 반대칭 행렬:
        [  0  -w3   w2]
        [ w3    0  -w1]
        [-w2   w1    0]
    """
    return np.array([
        [    0, -w[2],  w[1]],
        [ w[2],     0, -w[0]],
        [-w[1],  w[0],     0]
    ])

def vee(W):
    """
    반대칭 행렬을 벡터로 변환 (vee 연산자 ∨)
    
    Args:
        W: 3x3 반대칭 행렬
    
    Returns:
        3D 벡터 [w1, w2, w3]
    """
    return np.array([W[2, 1], W[0, 2], W[1, 0]])

# 테스트
w = np.array([1.0, 2.0, 3.0])
W = skew(w)

print(f"\n벡터 ω = {w}")
print(f"\nhat(ω) = ω^ =")
print(W)
print(f"\nvee(ω^) = {vee(W)}")
print(f"원래 벡터 복원 성공? {np.allclose(w, vee(W))}  ✅")

print(f"\n반대칭 성질 확인:")
print(f"Wᵀ = -W? {np.allclose(W.T, -W)}  ✅")

# 외적과의 관계
p = np.array([1, 0, 0])
cross_product = np.cross(w, p)
matrix_product = W @ p

print(f"\n💡 중요한 성질: ω^ @ p = ω × p (외적)")
print(f"ω × p = {cross_product}")
print(f"ω^ @ p = {matrix_product}")
print(f"같음? {np.allclose(cross_product, matrix_product)}  ✅")

# ============================================================
# Part 3: Rodrigues 공식 (exp 매핑)
# ============================================================
print("\n" + "=" * 70)
print("Part 3: Exponential Map - Rodrigues Formula")
print("=" * 70)

print("""
🎯 목표: Lie 대수(벡터) → Lie 군(회전 행렬)

Rodrigues 공식:
    R = exp(ω^) = I + sin(θ)K + (1-cos(θ))K²

    여기서:
    - θ = ‖ω‖ (회전 각도, radian)
    - k = ω/θ (단위 회전축)
    - K = skew(k) (단위 축의 반대칭 행렬)

물리적 의미:
    ω = θ·k = (회전 각도) × (회전축 방향)
""")

def exp_so3(omega):
    """
    SO(3)의 exp 매핑: so(3) → SO(3)
    Rodrigues 공식 구현
    
    Args:
        omega: 3D 벡터 (회전축 방향 × 각도)
    
    Returns:
        R: 3×3 회전 행렬
    """
    theta = np.linalg.norm(omega)  # 회전 각도
    
    # 특수 경우: 회전 없음
    if theta < 1e-10:
        return np.eye(3)
    
    # 단위 회전축
    axis = omega / theta
    K = skew(axis)
    
    # Rodrigues 공식
    # R = I + sin(θ)K + (1-cos(θ))K²
    R = np.eye(3) + np.sin(theta) * K + (1 - np.cos(theta)) * (K @ K)
    
    return R

print("\n" + "-" * 70)
print("예시 1: Z축 90도 회전")
print("-" * 70)

omega_z90 = np.array([0, 0, np.pi/2])  # Z축, π/2 radian
R_z90 = exp_so3(omega_z90)

print(f"\nω = [0, 0, π/2]")
print(f"  → Z축 주위로 90도(π/2) 회전")
print(f"\nexp(ω) = R:")
print(R_z90)

# 검증
print(f"\n검증:")
print(f"RᵀR = I? {np.allclose(R_z90.T @ R_z90, np.eye(3))}  ✅")
print(f"det(R) = {np.linalg.det(R_z90):.4f}  ✅")

# 포인트 회전 테스트
p_before = np.array([1, 0, 0])  # X축 방향 포인트
p_after = R_z90 @ p_before

print(f"\n포인트 변환:")
print(f"변환 전: {p_before}  (X축 방향)")
print(f"변환 후: {p_after}  (Y축 방향)")
print(f"→ Z축 주위로 90도 회전하면 X → Y  ✅")

print("\n" + "-" * 70)
print("예시 2: 임의 축 회전")
print("-" * 70)

# (1,1,1) 방향 축 주위로 60도 회전
axis = np.array([1, 1, 1]) / np.sqrt(3)  # 단위 벡터
angle = np.pi / 3  # 60도
omega_arbitrary = axis * angle

R_arbitrary = exp_so3(omega_arbitrary)

print(f"\n회전축: {axis} (정규화된)")
print(f"각도: {np.degrees(angle):.1f}°")
print(f"\nω = axis × angle = {omega_arbitrary}")
print(f"\nR =")
print(R_arbitrary)
print(f"\n여전히 유효한 회전? {np.allclose(R_arbitrary.T @ R_arbitrary, np.eye(3))}  ✅")

# ============================================================
# Part 4: Log 매핑 (SO(3) → so(3))
# ============================================================
print("\n" + "=" * 70)
print("Part 4: Logarithmic Map (Inverse of exp)")
print("=" * 70)

print("""
🎯 목표: Lie 군(회전 행렬) → Lie 대수(벡터)

공식:
    θ = arccos((tr(R) - 1) / 2)
    ω^ = (R - Rᵀ) / (2sin(θ)) · θ
    ω = vee(ω^)

유도:
    - Rodrigues 공식에서 tr(R) = 1 + 2cos(θ)
    - R - Rᵀ = 2sin(θ)ω^  (반대칭 부분)
""")

def log_so3(R):
    """
    SO(3)의 log 매핑: SO(3) → so(3)
    
    Args:
        R: 3×3 회전 행렬
    
    Returns:
        omega: 3D 벡터 (회전축 × 각도)
    """
    # 회전 각도 계산 (tr(R) = 1 + 2cos(θ) 공식 사용)
    trace = np.trace(R)  # 대각합: R[0,0] + R[1,1] + R[2,2]로 회전 각도 θ를 추출
    cos_theta = (trace - 1) / 2  # tr(R) = 1 + 2cos(θ) → cos(θ) 계산
    cos_theta = np.clip(cos_theta, -1, 1)  # 수치 안정성
    theta = np.arccos(cos_theta)
    
    # 특수 경우: 회전 없음
    if theta < 1e-10:
        return np.zeros(3)
    
    # 회전축 계산
    # R - Rᵀ = 2sin(θ)ω^
    omega_hat = (R - R.T) * theta / (2 * np.sin(theta))
    
    # 반대칭 행렬 → 벡터
    omega = vee(omega_hat)
    
    return omega

print("\n" + "-" * 70)
print("exp/log 왕복 테스트")
print("-" * 70)

# 임의의 회전 벡터
omega_original = np.array([0.3, -0.5, 0.8])

print(f"\n1. 원본 ω: {omega_original}")

# exp 매핑
R_temp = exp_so3(omega_original)
print(f"\n2. exp(ω) = R:")
print(R_temp)

# log 매핑
omega_recovered = log_so3(R_temp)
print(f"\n3. log(R) = ω': {omega_recovered}")

# 비교
print(f"\n4. ω와 ω' 비교:")
print(f"   원본:  {omega_original}")
print(f"   복원:  {omega_recovered}")
print(f"   일치? {np.allclose(omega_original, omega_recovered)}  ✅")

print("""
💡 결론: exp와 log는 역함수 관계!
   log(exp(ω)) = ω
   exp(log(R)) = R
""")

# ============================================================
# Part 5: 여러 회전 각도 테스트
# ============================================================
print("\n" + "=" * 70)
print("Part 5: 다양한 회전 테스트")
print("=" * 70)

test_cases = [
    ("X축 30°", np.array([np.pi/6, 0, 0])),
    ("Y축 45°", np.array([0, np.pi/4, 0])),
    ("Z축 90°", np.array([0, 0, np.pi/2])),
    ("임의 120°", np.array([1, 1, 1]) / np.sqrt(3) * (2*np.pi/3)),
]

print("\n회전 벡터 → 회전 행렬 → 회전 벡터 왕복 테스트:")
print("-" * 70)

for name, omega in test_cases:
    R = exp_so3(omega)
    omega_back = log_so3(R)
    angle_deg = np.degrees(np.linalg.norm(omega))
    
    success = np.allclose(omega, omega_back)
    status = "✅" if success else "❌"
    
    print(f"{name:12} | 각도: {angle_deg:6.2f}° | 왕복 성공: {status}")

# ============================================================
# Part 6: 최적화에서의 활용
# ============================================================
print("\n" + "=" * 70)
print("Part 6: Lie 대수를 이용한 회전 업데이트")
print("=" * 70)

print("""
🎯 핵심 아이디어:
    1. Lie 대수에서 업데이트 계산 (3개 파라미터만!)
    2. exp 매핑으로 증분 회전 생성
    3. 증분 회전을 현재 회전에 합성
    
업데이트 공식:
    R_new = exp(Δω) @ R
    
    여기서:
    - Δω ∈ ℝ³: Lie 대수에서의 작은 변화 (제약 없음!)
    - exp(Δω): 증분 회전 행렬 (항상 유효한 SO(3))
    - @ : 행렬 곱 (회전 합성)
""")

def rotation_z(theta):
    """Z축 회전 행렬 생성"""
    c, s = np.cos(theta), np.sin(theta)
    return np.array([[c, -s, 0], 
                     [s,  c, 0], 
                     [0,  0, 1]])

print("\n" + "-" * 70)
print("시나리오: 로봇이 Z축 주위로 30도 회전한 상태에서 작은 조정")
print("-" * 70)

# 현재 상태
R_current = rotation_z(np.radians(30))
print(f"\n초기 회전 R (Z축 30°):")
print(R_current)

# Lie 대수에서 작은 업데이트 (예: gradient descent의 한 스텝)
delta_omega = np.array([0.01, 0.02, 0.05])  # 3개 파라미터만!
print(f"\n업데이트 Δω = {delta_omega}")
print(f"크기: {np.linalg.norm(delta_omega):.4f} radian ({np.degrees(np.linalg.norm(delta_omega)):.2f}°)")

# exp 매핑으로 증분 회전 생성
delta_R = exp_so3(delta_omega)
print(f"\nexp(Δω) = ΔR:")
print(delta_R)

# 올바른 업데이트
R_new = delta_R @ R_current
print(f"\nR_new = exp(Δω) @ R:")
print(R_new)

# 검증
print(f"\n검증:")
print(f"R_new는 여전히 회전 행렬? {np.allclose(R_new.T @ R_new, np.eye(3))}  ✅")
print(f"det(R_new) = {np.linalg.det(R_new):.4f}  ✅")

print(f"""
💡 핵심 장점:
   ✅ Δω는 제약 없는 3차원 벡터 (일반 gradient descent 가능)
   ✅ exp(Δω)는 자동으로 유효한 회전 행렬
   ✅ 별도의 투영/정규화 불필요
   ✅ 수치적으로 안정적
""")

# ============================================================
# Part 7: SE(3) - 회전 + 이동
# ============================================================
print("\n" + "=" * 70)
print("Part 7: SE(3) Exponential Map")
print("=" * 70)

print("""
📐 SE(3): 강체 변환 (회전 + 이동)

se(3) 벡터:
    ξ = [ρ₁, ρ₂, ρ₃, φ₁, φ₂, φ₃]ᵀ ∈ ℝ⁶
    
    - ρ ∈ ℝ³: 평행이동 관련 (주의: t가 아님!)
    - φ ∈ ℝ³: 회전 (so(3)와 같음)

exp 매핑:
    T = exp(ξ) = [R | t]  (4×4 행렬)
                 [0 | 1]
    
    여기서:
    - R = exp(φ)  (SO(3))
    - t = J·ρ     (Jacobian 필요!)
""")

def exp_se3(xi):
    """
    SE(3)의 exp 매핑: se(3) → SE(3)
    
    Args:
        xi: 6D 벡터 [ρ(3), φ(3)]
    
    Returns:
        T: 4×4 변환 행렬
    """
    rho = xi[:3]  # ρ (rho, 그리스 문자): 평행이동 파라미터 (실제 t는 J @ rho로 계산)
    phi = xi[3:]  # φ (phi, 그리스 문자): 회전 파라미터 (R = exp(phi))
    
    # 1. 회전 부분
    R = exp_so3(phi)
    
    # 2. Jacobian 계산
    theta = np.linalg.norm(phi)
    
    if theta < 1e-10:
        # 작은 각도: J ≈ I
        J = np.eye(3)
    else:
        axis = phi / theta
        K = skew(axis)
        
        # J = I + ((1-cos(θ))/θ)K + ((θ-sin(θ))/θ)K²
        J = np.eye(3) + \
            ((1 - np.cos(theta)) / theta) * K + \
            ((theta - np.sin(theta)) / theta) * (K @ K)
    
    # 3. 실제 평행이동
    t = J @ rho
    
    # 4. 4×4 변환 행렬 조립
    T = np.eye(4)
    T[:3, :3] = R
    T[:3, 3] = t
    
    return T

print("\n" + "-" * 70)
print("예시: SE(3) 변환 생성")
print("-" * 70)

# se(3) 벡터
xi = np.array([
    0.1, 0.2, 0.3,  # ρ (이동 관련)
    0.0, 0.0, 0.5   # φ (Z축 약 28.6도 회전)
])

T = exp_se3(xi)

print(f"\nξ = {xi}")
print(f"  ρ (평행이동 관련): {xi[:3]}")
print(f"  φ (회전):          {xi[3:]}")
print(f"\nexp(ξ) = T:")
print(T)

# 포인트 변환 테스트
p_homogeneous = np.array([1, 0, 0, 1])  # 동차좌표
p_transformed = T @ p_homogeneous

print(f"\n포인트 변환:")
print(f"p 원본 (동차):  {p_homogeneous}")
print(f"T @ p:          {p_transformed}")
print(f"→ 회전 후 이동 적용됨  ✅")

# ============================================================
# Part 8: 시각화 (선택)
# ============================================================
print("\n" + "=" * 70)
print("Part 8: 회전 시각화")
print("=" * 70)

def plot_frame(ax, R, t=np.zeros(3), label="", scale=1.0):
    """좌표계 프레임 그리기"""
    # 원점
    origin = t
    
    # 축 벡터
    x_axis = origin + scale * R @ np.array([1, 0, 0])
    y_axis = origin + scale * R @ np.array([0, 1, 0])
    z_axis = origin + scale * R @ np.array([0, 0, 1])
    
    # 그리기
    ax.plot([origin[0], x_axis[0]], [origin[1], x_axis[1]], [origin[2], x_axis[2]], 'r-', linewidth=2)
    ax.plot([origin[0], y_axis[0]], [origin[1], y_axis[1]], [origin[2], y_axis[2]], 'g-', linewidth=2)
    ax.plot([origin[0], z_axis[0]], [origin[1], z_axis[1]], [origin[2], z_axis[2]], 'b-', linewidth=2)
    
    if label:
        ax.text(origin[0], origin[1], origin[2], label, fontsize=10)

# Z축 90도 회전 시각화
fig = plt.figure(figsize=(10, 5))

# Original frame
ax1 = fig.add_subplot(121, projection='3d')
ax1.set_title('Original Frame')
plot_frame(ax1, np.eye(3), label='Original')
ax1.set_xlabel('X'); ax1.set_ylabel('Y'); ax1.set_zlabel('Z')
ax1.set_xlim([-1.5, 1.5]); ax1.set_ylim([-1.5, 1.5]); ax1.set_zlim([-1.5, 1.5])

# Rotated frame
ax2 = fig.add_subplot(122, projection='3d')
ax2.set_title('Z-axis 90° Rotation')
plot_frame(ax2, np.eye(3), label='Original', scale=0.7)
plot_frame(ax2, R_z90, label='Rotated', scale=1.0)
ax2.set_xlabel('X'); ax2.set_ylabel('Y'); ax2.set_zlabel('Z')
ax2.set_xlim([-1.5, 1.5]); ax2.set_ylim([-1.5, 1.5]); ax2.set_zlim([-1.5, 1.5])

plt.tight_layout()
plt.savefig('rotation_visualization.png', dpi=150)
print("\nVisualization saved: rotation_visualization.png")
print("→ Red(X-axis), Green(Y-axis), Blue(Z-axis)")

# ============================================================
# 종합 정리
# ============================================================
print("\n" + "=" * 70)
print("📚 Week 6 종합 정리")
print("=" * 70)

print("""
✅ Part 1: Over-parameterized 문제
   - 회전 행렬 9개 vs 자유도 3개
   - 단순 덧셈(R+ΔR)은 회전 행렬 조건 위반
   - 해결: Lie 대수 (3개 파라미터)

✅ Part 2: Skew-symmetric 행렬
   - ω ∈ ℝ³ ↔ ω^ ∈ ℝ³ˣ³
   - skew(w): 벡터 → 반대칭 행렬
   - vee(W): 반대칭 행렬 → 벡터
   - 성질: ω^ @ p = ω × p (외적)

✅ Part 3: Rodrigues 공식 (exp 매핑)
   - so(3) → SO(3)
   - R = I + sin(θ)K + (1-cos(θ))K²
   - 항상 유효한 회전 행렬 생성

✅ Part 4: Log 매핑
   - SO(3) → so(3)
   - exp의 역함수
   - log(exp(ω)) = ω

✅ Part 5: 최적화 활용
   - R_new = exp(Δω) @ R
   - Δω는 제약 없는 3D 벡터
   - 자동으로 유효한 회전 보장

✅ Part 6: SE(3) exp 매핑
   - se(3) → SE(3)
   - 6 DOF (회전 3 + 이동 3)
   - Jacobian 필요 (ρ → t)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
💡 핵심 메시지:

Lie 대수를 사용하면:
  1. 파라미터 수 = 실제 자유도 (효율적)
  2. 제약 조건 처리 불필요 (간단)
  3. 항상 유효한 회전/변환 (안전)
  4. 일반적인 최적화 알고리즘 사용 가능 (범용성)

현대 SLAM의 필수 도구!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

🎯 다음 단계:
   1. lie_quiz.py로 개념 확인
   2. README.md 재학습
   3. Week 7 (최소자승법) 준비

📌 이 내용은 이후 Phase 에서 다시 복습합니다!
""")

print("\n" + "=" * 70)
print("lie_basics.py 실습 완료! 🎉")
print("=" * 70)
