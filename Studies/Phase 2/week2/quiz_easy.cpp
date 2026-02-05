/**
 * Phase 2 Week 2 - 카메라 캘리브레이션 기초 퀴즈
 * 
 * 이 퀴즈는 카메라 캘리브레이션의 기본 개념을 확인합니다.
 * 각 문제를 풀고, main() 함수를 실행하여 정답을 확인하세요.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

/**
 * 문제 1: 카메라 행렬 K의 의미 이해
 * 
 * 다음 카메라 행렬 K가 주어졌을 때, 각 요소의 의미를 파악하고
 * 이미지 중심 좌표와 초점 거리를 출력하세요.
 * 
 * K = [600.0,   0.0, 400.0]
 *     [  0.0, 600.0, 300.0]
 *     [  0.0,   0.0,   1.0]
 * 
 * TODO: 아래 함수를 완성하세요.
 */
void problem1_camera_matrix() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 카메라 행렬 K 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    cv::Mat K = (cv::Mat_<double>(3, 3) << 
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    
    std::cout << "주어진 카메라 행렬 K:" << std::endl;
    std::cout << K << "\n" << std::endl;
    
    // TODO: K 행렬에서 값을 추출하세요
    // 힌트: K.at<double>(row, col)로 접근
    double fx = 0.0;  // TODO: fx 값 추출
    double fy = 0.0;  // TODO: fy 값 추출
    double cx = 0.0;  // TODO: cx 값 추출
    double cy = 0.0;  // TODO: cy 값 추출
    
    std::cout << "📊 추출한 값:" << std::endl;
    std::cout << "   fx (X방향 초점거리): " << fx << " 픽셀" << std::endl;
    std::cout << "   fy (Y방향 초점거리): " << fy << " 픽셀" << std::endl;
    std::cout << "   cx (주점 X좌표): " << cx << " 픽셀" << std::endl;
    std::cout << "   cy (주점 Y좌표): " << cy << " 픽셀" << std::endl;
    
}

/**
 * 문제 2: 왜곡 계수의 역할 이해
 * 
 * 왜곡 계수 dist = [k1, k2, p1, p2, k3]에서
 * - k1, k2, k3: 방사 왜곡 (radial distortion)
 * - p1, p2: 접선 왜곡 (tangential distortion)
 * 
 * 다음 왜곡 계수가 주어졌을 때, 배럴 왜곡인지 핀쿠션 왜곡인지 판별하세요.
 * 
 * TODO: 아래 함수를 완성하세요.
 */
void problem2_distortion_type() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 왜곡 유형 판별" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.28, 0.07, 0.0, 0.0, 0.0);
    
    std::cout << "주어진 왜곡 계수: " << dist << std::endl;
    
    double k1 = dist.at<double>(0);
    double k2 = dist.at<double>(1);
    
    std::cout << "   k1 = " << k1 << std::endl;
    std::cout << "   k2 = " << k2 << "\n" << std::endl;
    
    // TODO: k1의 부호로 왜곡 유형 판별
    std::string distortion_type = "UNKNOWN";  // "BARREL" 또는 "PINCUSHION"으로 변경
    
    // 힌트:
    // - k1 < 0: 배럴 왜곡 (광각 렌즈, 이미지 바깥으로 밀림)
    // - k1 > 0: 핀쿠션 왜곡 (망원 렌즈, 이미지 안쪽으로 당김)
    
    std::cout << "📋 판별 결과: " << distortion_type << " 왜곡" << std::endl;
    
}

/**
 * 문제 3: 재투영 오차 (RMS) 해석
 * 
 * 캘리브레이션 후 다음과 같은 RMS 값들을 얻었습니다.
 * 각 결과의 품질을 평가하세요.
 * 
 * TODO: 아래 함수를 완성하세요.
 */
void problem3_rms_evaluation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: RMS 재투영 오차 평가" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::vector<double> rms_values = {0.35, 0.85, 1.25};
    std::vector<std::string> scenarios = {
        "시나리오 A", "시나리오 B", "시나리오 C"
    };
    
    for (size_t i = 0; i < rms_values.size(); i++) {
        double rms = rms_values[i];
        std::cout << scenarios[i] << ": RMS = " << rms << " 픽셀" << std::endl;
        
        // TODO: RMS 값에 따라 품질 평가
        std::string quality = "UNKNOWN";  // "EXCELLENT", "GOOD", "FAIR", "POOR" 중 선택
        
        // 힌트:
        // - RMS < 0.5: 우수 (EXCELLENT)
        // - 0.5 <= RMS < 1.0: 양호 (GOOD)
        // - RMS >= 1.0: 재캘리브레이션 권장 (POOR)
        
        std::cout << "   → 품질: " << quality << "\n" << std::endl;
    }
    
    std::cout << "💡 일반적인 기준:" << std::endl;
    std::cout << "   - RMS < 0.5: 매우 우수, 대부분의 응용에 적합" << std::endl;
    std::cout << "   - 0.5 ≤ RMS < 1.0: 양호, 일반적인 SLAM에 사용 가능" << std::endl;
    std::cout << "   - RMS ≥ 1.0: 불량, 재캘리브레이션 필요" << std::endl;
}

/**
 * 문제 4: 3D-2D 대응점 이해
 * 
 * 체커보드 캘리브레이션에서 3D 객체 점과 2D 이미지 점의 관계를 이해합니다.
 * 
 * 체커보드: 9×6 내부 코너, 한 칸 크기 30mm
 * 
 * TODO: 첫 번째 행의 3D 좌표들을 생성하세요.
 */
void problem4_3d_2d_correspondence() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 3D-2D 대응점 생성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    int board_width = 9;
    int board_height = 6;
    float square_size = 30.0f;  // mm
    
    std::cout << "체커보드 설정:" << std::endl;
    std::cout << "   - 내부 코너: " << board_width << " × " << board_height << std::endl;
    std::cout << "   - 한 칸 크기: " << square_size << " mm\n" << std::endl;
    
    // TODO: 첫 번째 행 (y=0)의 3D 점들을 생성하세요
    std::vector<cv::Point3f> first_row;
    
    // 힌트: (0, 0, 0), (30, 0, 0), (60, 0, 0), ..., (240, 0, 0)
    // for 문을 사용하여 board_width 개의 점 생성
    
    std::cout << "첫 번째 행의 3D 좌표:" << std::endl;
    for (size_t i = 0; i < first_row.size(); i++) {
        std::cout << "   점 " << i << ": (" 
                  << first_row[i].x << ", " 
                  << first_row[i].y << ", "
                  << first_row[i].z << ") mm" << std::endl;
    }
    
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 2 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_camera_matrix()
    problem2_distortion_type()
    problem3_rms_evaluation()
    problem4_3d_2d_correspondence()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
