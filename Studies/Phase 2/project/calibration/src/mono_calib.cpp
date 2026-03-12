#include "camera_calibration.hpp"
#include <opencv2/videoio.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <camera_id>" << std::endl;
        std::cout << "Examle: " << argv[0] << " 0" << std::endl;

        return -1;
    }

    int camera_id = std::stoi(argv[1]);

    CameraCalibration calib(cv::Size(8, 6), 30.0);

    cv::VideoCapture cap(camera_id);
    if (!cap.isOpened())
    {
        std::cerr << "Cannot open camera " << camera_id << std::endl;
        return -1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 800);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 600);

    std::vector<std::vector<cv::Point2f>> imagePoints;
    cv::Mat frame;
    int capturedFrames = 0;
    const int targetFrames = 20;

    std::cout << "\n📸 카메라 캘리브레이션 시작" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "SPACE: 캡처 (" << targetFrames << "장 필요)" << std::endl;
    std::cout << "ESC: 캡처 종료" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    while (capturedFrames < targetFrames)
    {
        cap >> frame;
        if (frame.empty())
        {
            break;
        }

        cv::Mat display = frame.clone();
        std::vector<cv::Point2f> corners;

        bool found = calib.findChessboardCorners(frame, corners);

        if (found)
        {
            cv::drawChessboardCorners(display, cv::Size(8, 6), corners, found);
            cv::putText(display,
                        "Chessboard Found! (Press SPACE)",
                        cv::Point(10, 60),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.7,
                        cv::Scalar(0, 255, 0),
                        2);
        }
        else
        {
            cv::putText(display,
                        "Searching for chessboard ...",
                        cv::Point(10, 60),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.7,
                        cv::Scalar(0, 0, 255),
                        2);
        }

        cv::putText(
            display,
            "Captured: " + std::to_string(capturedFrames) + "/" + std::to_string(targetFrames),
            cv::Point(10, 30),
            cv::FONT_HERSHEY_SIMPLEX,
            1,
            cv::Scalar(255, 255, 0),
            2);

        cv::imshow("Calibration", display);

        int key = cv::waitKey(30);
        if (key == 27)
        {
            break;
        }
        if (key == 32 && found)
        {
            imagePoints.push_back(corners);
            capturedFrames++;
            std::cout << "📷 Frame " << capturedFrames << " captured" << std::endl;
        }
    }

    cv::destroyAllWindows();

    if (capturedFrames < 10)
    {
        std::cerr << "❌ Not enough frames! (minimum 10)" << std::endl;
        return -1;
    }

    std::cout << "\n⚙️  캘리브레이션 수행 중..." << std::endl;

    cv::Size imageSize(frame.cols, frame.rows);

    cv::Mat K, dist;
    double rms = calib.calibrate(imagePoints, imageSize, K, dist);

    std::cout << "\n📊 결과:" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Camera Matrix K:\n" << K << std::endl;
    std::cout << "\nDistortion Coefficients:\n" << dist << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // std::string filename = (camera_id == 0) ? "camera_left_calib.yaml" :
    // "camera_right_calib.yaml"
    std::string filename = "macbook_calib.yaml";
    calib.saveCalibration(filename, K, dist, imageSize);

    if (rms < 0.5)
    {
        std::cout << "Great Calibration (RMS < 0.5)" << std::endl;
    }
    else if (rms < 1.0)
    {
        std::cout << "Good Calibration (RMS < 1.0)" << std::endl;
    }
    else
    {
        std::cout << "Poor Calibration (RMS >= 1.0)" << std::endl;
    }

    return 0;
}