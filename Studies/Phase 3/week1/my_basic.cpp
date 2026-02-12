/**
 * Phase 3 Week 1 - VO 자료구조 직접 구현
 *
 * vo_types.h는 header-only이므로 별도 구현이 필요 없습니다.
 * 추가 연습이 필요하면 이 파일에 자유롭게 작성하세요.
 *
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "vo_types.h"
#include <iostream>

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] VO 자료구조 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Frame 생성
    vo::Frame frame;
    frame.id = 0;
    frame.image = cv::Mat::zeros(480, 640, CV_8UC1);
    std::cout << "Frame ID: " << frame.id << std::endl;
    std::cout << "Image 크기: " << frame.image.cols << "x" << frame.image.rows << std::endl;

    // MapPoint 생성
    vo::MapPoint mp;
    mp.id = 0;
    mp.position = Eigen::Vector3d(1.0, 2.0, 5.0);
    std::cout << "MapPoint: (" << mp.position.x() << ", " << mp.position.y() << ", "
              << mp.position.z() << ")" << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
