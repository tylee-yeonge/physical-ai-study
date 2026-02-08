/**
 * Quiz Solutions - Easy
 * Week 12: Monocular 스케일 모호성
 */

#include <iostream>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 12 Quiz Solutions (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. 단안 카메라가 스케일을 결정할 수 없는 이유\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "정답: b) 3D 점 X와 λX가 같은 2D 픽셀에 투영되기 때문\n\n";
    std::cout << "설명:\n";
    std::cout << "  투영 방정식: λ * [u, v, 1]^T = K * [R|t] * [X, 1]^T\n";
    std::cout << "  \n";
    std::cout << "  3D 점 X = [1, 2, 5]^T 와\n";
    std::cout << "  2X = [2, 4, 10]^T 를 투영하면\n";
    std::cout << "  같은 픽셀 (u, v)에 나타남.\n";
    std::cout << "  \n";
    std::cout << "  이는 핀홀 모델의 근본적 한계로,\n";
    std::cout << "  하나의 이미지에서는 깊이(거리)를 알 수 없음.\n";
    std::cout << "  카메라 해상도나 렌즈 왜곡과는 무관한 문제!\n\n";

    std::cout << "Q2. Essential Matrix에서 ||t|| = 1 정규화의 의미\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "정답: b) t의 크기가 결정 불가하므로 방향만 사용하기 위해\n\n";
    std::cout << "설명:\n";
    std::cout << "  에피폴라 제약: x2^T [t]x R x1 = 0\n";
    std::cout << "  \n";
    std::cout << "  t를 αt로 바꾸면:\n";
    std::cout << "    x2^T [αt]x R x1 = α * (x2^T [t]x R x1) = α * 0 = 0\n";
    std::cout << "  \n";
    std::cout << "  어떤 α를 곱해도 제약이 성립!\n";
    std::cout << "  → t의 크기(norm)는 수학적으로 결정 불가\n";
    std::cout << "  → 유일한 해를 위해 ||t|| = 1로 고정\n";
    std::cout << "  → 방향 정보만 활용 가능\n\n";

    std::cout << "Q3. 스케일 드리프트의 정의\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "정답: b) 매 프레임 스케일 오차가 누적되어 맵이 왜곡되는 현상\n\n";
    std::cout << "설명:\n";
    std::cout << "  단안 VO에서는 매 프레임 t의 스케일이 조금씩 다름.\n";
    std::cout << "  \n";
    std::cout << "  예시 (실제 1m씩 이동):\n";
    std::cout << "    프레임 1: 추정 0.98m → 누적: 0.98m\n";
    std::cout << "    프레임 2: 추정 1.03m → 누적: 2.01m\n";
    std::cout << "    프레임 3: 추정 0.95m → 누적: 2.96m\n";
    std::cout << "    ...\n";
    std::cout << "    프레임 100: 누적 오차가 수 미터에 달할 수 있음!\n";
    std::cout << "  \n";
    std::cout << "  이는 카메라 해상도나 특징점 수와 무관한\n";
    std::cout << "  단안 카메라의 구조적 한계.\n\n";

    std::cout << "Q4. 스케일을 알 수 있는 경우\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "정답: b) 스테레오 카메라 (baseline 알려짐)\n\n";
    std::cout << "설명:\n";
    std::cout << "  스테레오 깊이 공식: Z = f * b / d\n";
    std::cout << "  \n";
    std::cout << "  - f: 초점거리 (캘리브레이션으로 알려짐)\n";
    std::cout << "  - b: baseline (물리적 측정값, 예: 12cm)\n";
    std::cout << "  - d: disparity (이미지에서 계산)\n";
    std::cout << "  \n";
    std::cout << "  f, b가 '미터 단위'로 알려져 있으므로\n";
    std::cout << "  Z도 '미터 단위'로 결정됨!\n";
    std::cout << "  \n";
    std::cout << "  다른 방법:\n";
    std::cout << "  - RGB-D 카메라: 깊이 센서로 직접 측정\n";
    std::cout << "  - IMU 융합: 가속도로 스케일 관측\n";
    std::cout << "  - 알려진 물체 크기 활용\n";

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
