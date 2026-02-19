/**
 * Phase 3 Week 9 - Schur Complement 직접 구현
 *
 * schur_basic.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬──────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                 │ 난이도  │ 검증 방법     │
 * ├──────┼──────────────────────┼────────┼──────────────┤
 * │  1   │ build_hessian        │ 어려움  │ ./my_basic   │
 * │  2   │ extract_blocks       │ 쉬움   │ ./my_basic   │
 * │  3   │ schur_complement     │ 핵심   │ ./my_basic   │
 * │  4   │ solve_reduced        │ 쉬움   │ ./my_basic   │
 * │  5   │ back_substitute      │ 핵심   │ ./my_basic   │
 * └──────┴──────────────────────┴────────┴──────────────┘
 */
#include "schur_basic.h"
#include <iostream>
#include <random>

// [Step 1] build_hessian — 관측에서 Hessian 구성
// 힌트:
//   1. 각 관측마다 자코비안 J (2x(cam_dim+pt_dim)) 생성
//   2. J는 해당 카메라와 점 위치에만 비영 (나머지 0)
//   3. H += J^T * J,  b += J^T * r
//   4. 대각 정규화 추가 (LM damping)
// 참고: basic.cpp demoSparseStructure()
HessianBlocks SchurBasic::build_hessian(int num_cameras, int num_points,
                                        const std::vector<std::pair<int, int>>& observations)
{
    int cam_dim = num_cameras * 6;
    int pt_dim = num_points * 3;
    int total = cam_dim + pt_dim;

    // TODO:
    // 1. H = Zero(total, total), b = Zero(total)
    // 2. 각 관측(cam_idx, pt_idx)에 대해:
    //    - J = Zero(2, total)
    //    - J의 cam_start ~ cam_start+5 열과 pt_start ~ pt_start+2 열에 랜덤 값
    //    - H += J^T * J, b += J^T * r
    // 3. 대각 정규화: H(i,i) += 1e-3
    // 4. extract_blocks로 반환

    Eigen::MatrixXd H = Eigen::MatrixXd::Identity(total, total);
    Eigen::VectorXd b = Eigen::VectorXd::Zero(total);
    return extract_blocks(H, b, cam_dim, pt_dim);
}

// [Step 2] extract_blocks — 전체 Hessian에서 블록 추출
// 힌트: Eigen의 .block() 사용
// 기대값: Hcc(cam_dim x cam_dim), Hpp(pt_dim x pt_dim) 등
HessianBlocks SchurBasic::extract_blocks(const Eigen::MatrixXd& H, const Eigen::VectorXd& b,
                                         int cam_dim, int pt_dim)
{
    HessianBlocks blocks;
    // TODO:
    // blocks.Hcc = H.block(0, 0, cam_dim, cam_dim)
    // blocks.Hcp = H.block(0, cam_dim, cam_dim, pt_dim)
    // blocks.Hpp = H.block(cam_dim, cam_dim, pt_dim, pt_dim)
    // blocks.bc = b.segment(0, cam_dim)
    // blocks.bp = b.segment(cam_dim, pt_dim)
    blocks.Hcc = Eigen::MatrixXd::Identity(cam_dim, cam_dim);
    blocks.Hcp = Eigen::MatrixXd::Zero(cam_dim, pt_dim);
    blocks.Hpp = Eigen::MatrixXd::Identity(pt_dim, pt_dim);
    blocks.bc = Eigen::VectorXd::Zero(cam_dim);
    blocks.bp = Eigen::VectorXd::Zero(pt_dim);
    return blocks;
}

// [Step 3] schur_complement — Schur Complement 계산 (핵심!)
// 힌트:
//   H_reduced = Hcc - Hcp * Hpp^{-1} * Hpc
//   b_reduced = bc  - Hcp * Hpp^{-1} * bp
//   (Hpc = Hcp^T)
// 참고: basic.cpp demoSchurComplement()
// 기대값: 직접 풀기와 동일한 결과
void SchurBasic::schur_complement(const HessianBlocks& blocks,
                                  Eigen::MatrixXd& H_reduced, Eigen::VectorXd& b_reduced)
{
    // TODO:
    // 1. Hpp_inv = blocks.Hpp.inverse()
    // 2. H_reduced = blocks.Hcc - blocks.Hcp * Hpp_inv * blocks.Hcp.transpose()
    // 3. b_reduced = blocks.bc - blocks.Hcp * Hpp_inv * blocks.bp
    H_reduced = blocks.Hcc;
    b_reduced = blocks.bc;
}

// [Step 4] solve_reduced — 축소된 시스템 풀기
// 힌트: LDLT 분해 사용 (대칭 양정치)
// 기대값: delta_c = H_reduced^{-1} * b_reduced
Eigen::VectorXd SchurBasic::solve_reduced(const Eigen::MatrixXd& H_reduced,
                                          const Eigen::VectorXd& b_reduced)
{
    // TODO: H_reduced.ldlt().solve(b_reduced)
    return Eigen::VectorXd::Zero(b_reduced.size());
}

// [Step 5] back_substitute — 역대입으로 점 변수 복구 (핵심!)
// 힌트: delta_p = Hpp^{-1} * (bp - Hpc * delta_c)
//       Hpc = Hcp^T
// 참고: basic.cpp demoSchurComplement()
Eigen::VectorXd SchurBasic::back_substitute(const HessianBlocks& blocks,
                                            const Eigen::VectorXd& delta_c)
{
    // TODO:
    // 1. Hpp_inv = blocks.Hpp.inverse()
    // 2. delta_p = Hpp_inv * (blocks.bp - blocks.Hcp.transpose() * delta_c)
    return Eigen::VectorXd::Zero(blocks.bp.size());
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] Schur Complement 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    int num_cam = 3, num_pt = 5;
    std::vector<std::pair<int, int>> obs = {
        {0, 0}, {0, 1}, {0, 2},
        {1, 1}, {1, 2}, {1, 3},
        {2, 2}, {2, 3}, {2, 4}
    };

    // ── Step 1 테스트: build_hessian ──
    std::cout << "[Step 1] build_hessian" << std::endl;
    auto blocks = SchurBasic::build_hessian(num_cam, num_pt, obs);
    bool step1 = (blocks.Hcc.rows() == 18 && blocks.Hpp.rows() == 15);
    std::cout << "  Hcc: " << blocks.Hcc.rows() << "x" << blocks.Hcc.cols()
              << (blocks.Hcc.rows() == 18 ? " ✅" : " ❌") << std::endl;
    std::cout << "  Hpp: " << blocks.Hpp.rows() << "x" << blocks.Hpp.cols()
              << (blocks.Hpp.rows() == 15 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 2 테스트: extract_blocks ──
    std::cout << "[Step 2] extract_blocks" << std::endl;
    bool step2 = (blocks.Hcp.rows() == 18 && blocks.Hcp.cols() == 15);
    std::cout << "  Hcp: " << blocks.Hcp.rows() << "x" << blocks.Hcp.cols()
              << (step2 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 3 테스트: schur_complement ──
    std::cout << "[Step 3] schur_complement" << std::endl;
    Eigen::MatrixXd H_reduced;
    Eigen::VectorXd b_reduced;
    SchurBasic::schur_complement(blocks, H_reduced, b_reduced);
    bool step3 = (H_reduced.rows() == 18);
    std::cout << "  H_reduced: " << H_reduced.rows() << "x" << H_reduced.cols()
              << (step3 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 4 테스트: solve_reduced ──
    std::cout << "[Step 4] solve_reduced" << std::endl;
    Eigen::VectorXd delta_c = SchurBasic::solve_reduced(H_reduced, b_reduced);
    bool step4 = (delta_c.size() == 18);
    std::cout << "  delta_c 크기: " << delta_c.size()
              << (step4 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 5 테스트: back_substitute ──
    std::cout << "[Step 5] back_substitute" << std::endl;
    Eigen::VectorXd delta_p = SchurBasic::back_substitute(blocks, delta_c);
    bool step5 = (delta_p.size() == 15);
    std::cout << "  delta_p 크기: " << delta_p.size()
              << (step5 ? " ✅" : " ❌") << "\n" << std::endl;

    // 직접 풀기와 비교
    int total = num_cam * 6 + num_pt * 3;
    Eigen::MatrixXd H_full(total, total);
    H_full << blocks.Hcc, blocks.Hcp,
              blocks.Hcp.transpose(), blocks.Hpp;
    Eigen::VectorXd b_full(total);
    b_full << blocks.bc, blocks.bp;
    Eigen::VectorXd x_direct = H_full.ldlt().solve(b_full);

    Eigen::VectorXd x_schur(total);
    x_schur << delta_c, delta_p;
    double diff = (x_direct - x_schur).norm();
    bool match = (diff < 1e-8);
    std::cout << "직접 풀기 vs Schur 차이: " << diff
              << (match ? " ✅ 동일!" : " ❌") << std::endl;

    // ── 종합 결과 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step1 " << (step1 ? "✅" : "❌")
              << " Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌")
              << " Step5 " << (step5 ? "✅" : "❌")
              << " 비교 " << (match ? "✅" : "❌") << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
