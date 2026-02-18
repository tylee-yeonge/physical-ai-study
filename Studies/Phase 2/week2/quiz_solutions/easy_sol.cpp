/**
 * Phase 2 Week 2 - 기초 퀴즈 정답
 *
 * 이 파일은 quiz_easy.cpp의 완성된 정답 코드입니다.
 * 먼저 스스로 풀어본 후 참고하세요!
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

void problem1_camera_matrix()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 카메라 행렬 K 분석 [정답]" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    std::cout << "주어진 카메라 행렬 K:" << std::endl;
    std::cout << K << "\n" << std::endl;

    // ✅ 정답: K 행렬에서 값 추출
    double fx = K.at<double>(0, 0);  // 600.0
    double fy = K.at<double>(1, 1);  // 600.0
    double cx = K.at<double>(0, 2);  // 400.0
    double cy = K.at<double>(1, 2);  // 300.0

    std::cout << "📊 추출한 값:" << std::endl;
    std::cout << "   fx (X방향 초점거리): " << fx << " 픽셀" << std::endl;
    std::cout << "   fy (Y방향 초점거리): " << fy << " 픽셀" << std::endl;
    std::cout << "   cx (주점 X좌표): " << cx << " 픽셀" << std::endl;
    std::cout << "   cy (주점 Y좌표): " << cy << " 픽셀" << std::endl;

    std::cout << "\n💡 설명:" << std::endl;
    std::cout << "   - K(0,0) = fx: X방향 초점 거리" << std::endl;
    std::cout << "   - K(1,1) = fy: Y방향 초점 거리" << std::endl;
    std::cout << "   - K(0,2) = cx: 주점의 X 좌표 (이미지 중심)" << std::endl;
    std::cout << "   - K(1,2) = cy: 주점의 Y 좌표" << std::endl;
    std::cout << "\n✅ 정답입니다!" << std::endl;
}

void problem2_distortion_type()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 왜곡 유형 판별 [정답]" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.28, 0.07, 0.0, 0.0, 0.0);

    std::cout << "주어진 왜곡 계수: " << dist << std::endl;

    double k1 = dist.at<double>(0);  // -0.28
    double k2 = dist.at<double>(1);  // 0.07

    std::cout << "   k1 = " << k1 << std::endl;
    std::cout << "   k2 = " << k2 << "\n" << std::endl;

    // ✅ 정답: k1의 부호로 왜곡 유형 판별
    std::string distortion_type;
    if (k1 < 0)
    {
        distortion_type = "BARREL";
    }
    else if (k1 > 0)
    {
        distortion_type = "PINCUSHION";
    }
    else
    {
        distortion_type = "NONE";
    }

    std::cout << "📋 판별 결과: " << distortion_type << " 왜곡" << std::endl;

    std::cout << "\n💡 설명:" << std::endl;
    std::cout << "   - k1 = -0.28 < 0 이므로 배럴 왜곡" << std::endl;
    std::cout << "   - 배럴 왜곡: 이미지가 바깥으로 볼록하게 팽창" << std::endl;
    std::cout << "   - 주로 광각 렌즈에서 발생 (어안 렌즈 효과)" << std::endl;
    std::cout << "\n✅ 정답입니다!" << std::endl;
}

void problem3_rms_evaluation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: RMS 재투영 오차 평가 [정답]" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::vector<double> rms_values = {0.35, 0.85, 1.25};
    std::vector<std::string> scenarios = {"시나리오 A", "시나리오 B", "시나리오 C"};

    for (size_t i = 0; i < rms_values.size(); i++)
    {
        double rms = rms_values[i];
        std::cout << scenarios[i] << ": RMS = " << rms << " 픽셀" << std::endl;

        // ✅ 정답: RMS 값에 따라 품질 평가
        std::string quality;
        if (rms < 0.5)
        {
            quality = "EXCELLENT";
        }
        else if (rms < 1.0)
        {
            quality = "GOOD";
        }
        else
        {
            quality = "POOR";
        }

        std::cout << "   → 품질: " << quality << "\n" << std::endl;
    }

    std::cout << "💡 설명:" << std::endl;
    std::cout << "   - 시나리오 A (0.35): 우수 - 대부분의 응용에 적합" << std::endl;
    std::cout << "   - 시나리오 B (0.85): 양호 - 일반적인 SLAM 가능" << std::endl;
    std::cout << "   - 시나리오 C (1.25): 불량 - 재캘리브레이션 필요" << std::endl;
    std::cout << "\n✅ 정답입니다!" << std::endl;
}

void problem4_3d_2d_correspondence()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 3D-2D 대응점 생성 [정답]" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    int board_width = 9;
    int board_height = 6;
    float square_size = 30.0f;

    std::cout << "체커보드 설정:" << std::endl;
    std::cout << "   - 내부 코너: " << board_width << " × " << board_height << std::endl;
    std::cout << "   - 한 칸 크기: " << square_size << " mm\n" << std::endl;

    // ✅ 정답: 첫 번째 행의 3D 점 생성
    std::vector<cv::Point3f> first_row;
    for (int j = 0; j < board_width; j++)
    {
        first_row.push_back(cv::Point3f(j * square_size, 0.0f, 0.0f));
    }

    std::cout << "첫 번째 행의 3D 좌표:" << std::endl;
    for (size_t i = 0; i < first_row.size(); i++)
    {
        std::cout << "   점 " << i << ": (" << first_row[i].x << ", " << first_row[i].y << ", "
                  << first_row[i].z << ") mm" << std::endl;
    }

    std::cout << "\n💡 설명:" << std::endl;
    std::cout << "   - X 좌표: i × square_size (0, 30, 60, ..., 240)" << std::endl;
    std::cout << "   - Y 좌표: 0 (첫 번째 행)" << std::endl;
    std::cout << "   - Z 좌표: 0 (평면)" << std::endl;
    std::cout << "\n✅ 정답입니다!" << std::endl;
}

/**
 * @brief 문제 5 정답: 왜곡 크기 분포 분석
 *
 * 정규화 좌표에서 r^2를 구하고, 방사 왜곡 변위를 계산하여
 * 중심과 모서리의 왜곡 차이를 확인합니다.
 */
void problem5_distortion_magnitude()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 왜곡 크기 분포 분석 [정답]" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    double k1 = -0.3, k2 = 0.1;

    struct GridPoint
    {
        double u, v;
        std::string location;
    };

    std::vector<GridPoint> grid_points = {
        {320, 240, "중심"},
        {160, 120, "좌상 1/4"},
        {480, 360, "우하 1/4"},
        {0, 0, "좌상 모서리"},
        {640, 480, "우하 모서리"},
        {320, 0, "상단 중앙"},
        {0, 240, "좌측 중앙"},
    };

    std::cout << "  위치              |  (u, v)       | r^2    | 변위(px)" << std::endl;
    std::cout << std::string(65, '-') << std::endl;

    for (const auto& pt : grid_points)
    {
        // 정규화 좌표
        double x = (pt.u - cx) / fx;
        double y = (pt.v - cy) / fy;

        // r^2
        double r2 = x * x + y * y;

        // 방사 왜곡 계수
        double radial = 1.0 + k1 * r2 + k2 * r2 * r2;

        // 왜곡된 정규화 좌표
        double x_dist = x * radial;
        double y_dist = y * radial;

        // 왜곡된 픽셀 좌표
        double u_dist = fx * x_dist + cx;
        double v_dist = fy * y_dist + cy;

        // 변위 크기
        double du = u_dist - pt.u;
        double dv = v_dist - pt.v;
        double displacement = std::sqrt(du * du + dv * dv);

        std::cout << "  " << pt.location;
        for (size_t i = pt.location.size(); i < 18; i++)
        {
            std::cout << " ";
        }
        std::cout << "| (" << (int)pt.u << ", " << (int)pt.v << ")"
                  << "\t| " << r2 << "\t | " << displacement << std::endl;
    }

    std::cout << "\n✅ 중심에서 변위=0, 모서리에서 최대 변위 확인!" << std::endl;
}

int main()
{
    std::cout << "\n╔═══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║     기초 퀴즈 정답 및 상세 설명              ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════╝\n" << std::endl;

    problem1_camera_matrix();
    problem2_distortion_type();
    problem3_rms_evaluation();
    problem4_3d_2d_correspondence();
    problem5_distortion_magnitude();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "🎓 모든 문제 정답 확인 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
