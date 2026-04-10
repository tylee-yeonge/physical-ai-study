#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>

constexpr int N = 16;
constexpr int CONSECUTIVE = 9;
constexpr int WINDOW = 3;

enum PixelState
{
    LOWER,
    SAME,
    HIGHER
};

PixelState get_state(int Ip, int val, int threshold)
{
    if (val > Ip + threshold)
    {
        return HIGHER;
    }
    if (val < Ip - threshold)
    {
        return LOWER;
    }
    return SAME;
}

bool high_speed_test(
    const cv::Mat& gray, int x, int y, int Ip, const int offsets[][2], int threshold)
{
    const int check_index[4] = {0, 4, 8, 12};
    int higher_count = 0;
    int lower_count = 0;

    for (int i = 0; i < 4; i++)
    {
        PixelState s = get_state(
            Ip,
            gray.at<uchar>(y + offsets[check_index[i]][1], x + offsets[check_index[i]][0]),
            threshold);

        if (s == HIGHER)
            higher_count++;
        if (s == LOWER)
            lower_count++;
    }

    return (higher_count >= 3 || lower_count >= 3);
}

bool check_consecutive(
    const cv::Mat& gray, int x, int y, const int Ip, const int offsets[][2], int threshold)
{
    int count = 0;
    PixelState last = SAME;
    for (int i = 0; i < N * 2; i++)
    {
        PixelState s =
            get_state(Ip, gray.at<uchar>(y + offsets[i % N][1], x + offsets[i % N][0]), threshold);
        if (s != SAME && s == last)
        {
            count++;
        }
        else
        {
            count = 1;
            last = s;
        }

        if (count >= CONSECUTIVE)
        {
            return true;
        }
    }
    return false;
}

bool is_corner(const cv::Mat& gray, int x, int y, const int offsets[][2], int threshold)
{
    if (x < 3 || x >= gray.cols - 3 || y < 3 || y >= gray.rows - 3)
    {
        return false;
    }

    int Ip = gray.at<uchar>(y, x);

    if (!high_speed_test(gray, x, y, Ip, offsets, threshold))
        return false;

    return check_consecutive(gray, x, y, Ip, offsets, threshold);
}

int compute_corner_score(const cv::Mat& gray, int x, int y, const int offsets[][2])
{
    int Ip = gray.at<uchar>(y, x);
    int sum = 0;
    for (int i = 0; i < N; i++)
    {
        int score = gray.at<uchar>(y + offsets[i][1], x + offsets[i][0]);
        sum += abs(score - Ip);
    }
    return sum;
}

std::vector<cv::Point> compute_non_maximum_supperssion(const cv::Mat& gray,
                                                       const int offsets[][2],
                                                       int threshold)
{
    std::vector<cv::Point> result;
    cv::Mat scores;
    for (int i = 0; i < gray.cols; i++)
    {
        for (int j = 0; j < gray.rows; j++)
        {
            if (is_corner(gray, i, j, offsets, threshold))
                scores.at<float>(y, x) = compute_corner_score(gray, j, i, offsets);
        }
    }
}

int main()
{
    const int offsets[N][2] = {
        {0, -3},
        {1, -3},
        {2, -2},
        {3, -1},
        {3, 0},
        {3, 1},
        {2, 2},
        {1, 3},
        {0, 3},
        {-1, 3},
        {-2, 2},
        {-3, 1},
        {-3, 0},
        {-3, -1},
        {-2, -2},
        {-1, -3},
    };

    return 0;
}