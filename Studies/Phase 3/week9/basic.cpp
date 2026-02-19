#include "schur_basic.h"
#include <iostream>
#include <random>
#include <chrono>

// ====================
// SchurBasic 구현
// ====================

HessianBlocks SchurBasic::build_hessian(int num_cameras, int num_points,
                                        const std::vector<std::pair<int, int>>& observations)
{
    // 카메라 6 DoF, 점 3 DoF (간략화)
    int cam_dim = num_cameras * 6;
    int pt_dim = num_points * 3;
    int total = cam_dim + pt_dim;

    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(total, total);
    Eigen::VectorXd b = Eigen::VectorXd::Zero(total);

    std::mt19937 gen(42);
    std::normal_distribution<> noise(0, 1);

    // 각 관측에 대해 자코비안 J를 시뮬레이션하고 H += J^T * J
    for (const auto& [cam_idx, pt_idx] : observations)
    {
        // 2x9 자코비안 시뮬레이션 (2D 관측, 6 DoF 카메라 + 3 DoF 점)
        Eigen::MatrixXd J = Eigen::MatrixXd::Zero(2, total);

        int cam_start = cam_idx * 6;
        int pt_start = cam_dim + pt_idx * 3;

        // 카메라 부분 자코비안
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                J(i, cam_start + j) = noise(gen);
            }
        }
        // 점 부분 자코비안
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                J(i, pt_start + j) = noise(gen);
            }
        }

        // 잔차 시뮬레이션
        Eigen::Vector2d r(noise(gen) * 0.1, noise(gen) * 0.1);

        H += J.transpose() * J;
        b += J.transpose() * r;
    }

    // 대각 정규화 (LM damping)
    for (int i = 0; i < total; i++)
    {
        H(i, i) += 1e-3;
    }

    return extract_blocks(H, b, cam_dim, pt_dim);
}

HessianBlocks SchurBasic::extract_blocks(const Eigen::MatrixXd& H, const Eigen::VectorXd& b,
                                         int cam_dim, int pt_dim)
{
    HessianBlocks blocks;
    blocks.Hcc = H.block(0, 0, cam_dim, cam_dim);
    blocks.Hcp = H.block(0, cam_dim, cam_dim, pt_dim);
    blocks.Hpp = H.block(cam_dim, cam_dim, pt_dim, pt_dim);
    blocks.bc = b.segment(0, cam_dim);
    blocks.bp = b.segment(cam_dim, pt_dim);
    return blocks;
}

void SchurBasic::schur_complement(const HessianBlocks& blocks,
                                  Eigen::MatrixXd& H_reduced, Eigen::VectorXd& b_reduced)
{
    // Hpp^{-1} — 블록 대각이므로 역행렬 쉬움
    Eigen::MatrixXd Hpp_inv = blocks.Hpp.inverse();

    // Schur Complement
    H_reduced = blocks.Hcc - blocks.Hcp * Hpp_inv * blocks.Hcp.transpose();
    b_reduced = blocks.bc - blocks.Hcp * Hpp_inv * blocks.bp;
}

Eigen::VectorXd SchurBasic::solve_reduced(const Eigen::MatrixXd& H_reduced,
                                          const Eigen::VectorXd& b_reduced)
{
    return H_reduced.ldlt().solve(b_reduced);
}

Eigen::VectorXd SchurBasic::back_substitute(const HessianBlocks& blocks,
                                            const Eigen::VectorXd& delta_c)
{
    Eigen::MatrixXd Hpp_inv = blocks.Hpp.inverse();
    return Hpp_inv * (blocks.bp - blocks.Hcp.transpose() * delta_c);
}

// ====================
// 교육 블록
// ====================

// 💡 quiz_easy Q1: 자코비안이 희소한 이유
// 💡 quiz_easy Q4: Hpp가 블록 대각인 이유
void demoSparseStructure()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 BA Hessian의 희소 구조" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "핵심: 각 관측은 1개 카메라 + 1개 점에만 연결!\n" << std::endl;

    std::cout << "Hessian 구조:" << std::endl;
    std::cout << "  H = [Hcc  Hcp]" << std::endl;
    std::cout << "      [Hpc  Hpp]\n" << std::endl;

    std::cout << "  Hcc: 카메라-카메라 (작고 밀집)" << std::endl;
    std::cout << "  Hcp: 카메라-점 (희소)" << std::endl;
    std::cout << "  Hpp: 점-점 (블록 대각!)\n" << std::endl;

    std::cout << "💡 Hpp가 블록 대각인 이유:" << std::endl;
    std::cout << "   점 i와 점 j는 서로 직접 연결 없음" << std::endl;
    std::cout << "   → Hpp의 (i,j) 블록 = 0 (i≠j)" << std::endl;

    // 수치 시연
    int num_cam = 3, num_pt = 5;
    std::vector<std::pair<int, int>> obs = {
        {0, 0}, {0, 1}, {0, 2},
        {1, 1}, {1, 2}, {1, 3},
        {2, 2}, {2, 3}, {2, 4}
    };

    auto blocks = SchurBasic::build_hessian(num_cam, num_pt, obs);

    std::cout << "\n예시: 3 카메라, 5 점, 9 관측" << std::endl;
    std::cout << "  Hcc 크기: " << blocks.Hcc.rows() << "x" << blocks.Hcc.cols()
              << " (18x18)" << std::endl;
    std::cout << "  Hpp 크기: " << blocks.Hpp.rows() << "x" << blocks.Hpp.cols()
              << " (15x15)" << std::endl;

    // Hpp 대각 구조 확인
    double off_diag_norm = 0;
    for (int i = 0; i < num_pt; i++)
    {
        for (int j = 0; j < num_pt; j++)
        {
            if (i != j)
            {
                off_diag_norm += blocks.Hpp.block(i * 3, j * 3, 3, 3).norm();
            }
        }
    }
    std::cout << "  Hpp 비대각 블록 norm: " << off_diag_norm
              << (off_diag_norm < 1e-6 ? " ≈ 0 (블록 대각!)" : " > 0") << std::endl;
}

// 💡 quiz_easy Q2: Schur에서 먼저 소거하는 변수
// 💡 quiz_medium Q1: Schur가 계산량을 줄이는 원리
void demoSchurComplement()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Schur Complement 시연" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "전략: 점 변수(많음)를 먼저 소거!\n" << std::endl;
    std::cout << "  원래: (18+15)x(18+15) = 33x33 풀기" << std::endl;
    std::cout << "  Schur: 18x18 풀기 → 역대입으로 15개 복구\n" << std::endl;

    int num_cam = 3, num_pt = 5;
    std::vector<std::pair<int, int>> obs = {
        {0, 0}, {0, 1}, {0, 2},
        {1, 1}, {1, 2}, {1, 3},
        {2, 2}, {2, 3}, {2, 4}
    };

    auto blocks = SchurBasic::build_hessian(num_cam, num_pt, obs);

    // 전체 시스템 직접 풀기
    int total = num_cam * 6 + num_pt * 3;
    Eigen::MatrixXd H_full(total, total);
    H_full << blocks.Hcc, blocks.Hcp,
              blocks.Hcp.transpose(), blocks.Hpp;
    Eigen::VectorXd b_full(total);
    b_full << blocks.bc, blocks.bp;

    auto t1 = std::chrono::high_resolution_clock::now();
    Eigen::VectorXd x_direct = H_full.ldlt().solve(b_full);
    auto t2 = std::chrono::high_resolution_clock::now();

    // Schur Complement로 풀기
    Eigen::MatrixXd H_reduced;
    Eigen::VectorXd b_reduced;
    auto t3 = std::chrono::high_resolution_clock::now();
    SchurBasic::schur_complement(blocks, H_reduced, b_reduced);
    Eigen::VectorXd delta_c = SchurBasic::solve_reduced(H_reduced, b_reduced);
    Eigen::VectorXd delta_p = SchurBasic::back_substitute(blocks, delta_c);
    auto t4 = std::chrono::high_resolution_clock::now();

    // 결과 비교
    Eigen::VectorXd x_schur(total);
    x_schur << delta_c, delta_p;

    double diff = (x_direct - x_schur).norm();
    std::cout << "결과 비교:" << std::endl;
    std::cout << "  직접 풀기: " << x_direct.head(6).transpose() << " ..." << std::endl;
    std::cout << "  Schur:     " << delta_c.head(6).transpose() << " ..." << std::endl;
    std::cout << "  차이 norm: " << diff << (diff < 1e-8 ? " ✅ 동일!" : " ❌") << std::endl;

    std::cout << "\n💡 실제 BA에서의 효과:" << std::endl;
    std::cout << "  100 카메라 + 10000 점:" << std::endl;
    std::cout << "  직접: (600+30000)² = 30600² ≈ 9.4억 원소" << std::endl;
    std::cout << "  Schur: 600² = 36만 원소 (2600배 감소!)" << std::endl;
}

// 💡 quiz_easy Q3: Local BA vs Global BA
// 💡 quiz_medium Q2: Local BA 장단점
void demoLocalVsGlobal()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Local BA vs Global BA" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Local BA:" << std::endl;
    std::cout << "  범위: Covisible Keyframes만" << std::endl;
    std::cout << "  장점: 실시간 가능 (10-50ms)" << std::endl;
    std::cout << "  단점: 드리프트 누적\n" << std::endl;

    std::cout << "Global BA:" << std::endl;
    std::cout << "  범위: 전체 맵" << std::endl;
    std::cout << "  장점: 전역 최적" << std::endl;
    std::cout << "  단점: 느림 (Loop Closure 후만)\n" << std::endl;

    std::cout << "💡 ORB-SLAM: Local BA (실시간) + Global BA (Loop 후)" << std::endl;
    std::cout << "💡 VINS: Sliding Window BA = Local BA의 변형" << std::endl;
}

// 💡 quiz_easy Q5: g2o setMarginalized
// 💡 quiz_easy Q6: Gauge freedom
void demoGaugeAndMarginalize()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Gauge Freedom & Marginalization" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Gauge Freedom:" << std::endl;
    std::cout << "  BA는 7 DoF 자유도 (3 회전 + 3 이동 + 1 스케일)" << std::endl;
    std::cout << "  → 첫 KF를 setFixed(true)로 고정\n" << std::endl;

    std::cout << "Marginalization (g2o):" << std::endl;
    std::cout << "  vertex_point->setMarginalized(true)" << std::endl;
    std::cout << "  → 내부적으로 Schur Complement 적용!" << std::endl;
    std::cout << "  → 점 변수를 먼저 소거 → 카메라만 풀기" << std::endl;
}

// ====================
// Main
// ====================

#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 9: Schur Complement" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 교육 블록
    demoSparseStructure();
    demoSchurComplement();
    demoLocalVsGlobal();
    demoGaugeAndMarginalize();

    // 다음 단계 안내
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📌 다음 단계:" << std::endl;
    std::cout << "  1. quiz_easy    → 희소 구조, Schur, Local/Global BA" << std::endl;
    std::cout << "  2. quiz_medium  → Schur 계산량 분석, VINS" << std::endl;
    std::cout << "  3. my_basic.cpp → 직접 구현 (5 Step)" << std::endl;
    std::cout << "  4. Week 10      → g2o 심화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
