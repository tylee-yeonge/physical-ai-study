"""카메라 화면을 브라우저로 실시간 확인한다 -- 화면이 없는 (헤드리스) 컨테이너용 MJPEG 스트림.

실행:
    acl
    python Studies/Hardware-Arm/spike/week2/scripts/live_view.py          # 전체 뷰 ELP
    python Studies/Hardware-Arm/spike/week2/scripts/live_view.py wrist    # 손목 카메라
그다음 VS Code 의 PORTS 패널에서 18080 을 포워딩하고 브라우저로 http://localhost:18080 을 연다.
끝낼 때는 Ctrl+C. 켜 둔 동안에는 lerobot 이 같은 카메라를 열지 못한다 (장치는 한 번에 한 프로세스만 연다).
"""

import sys
import time
from http.server import BaseHTTPRequestHandler
from http.server import HTTPServer

import cv2

# 카메라별 설정 -- lerobot 에 주는 값 (week2_guide §1.4) 과 같게 해서 "lerobot 이 보는 화면" 을 그대로 본다
CAMERAS = {
    "overview": ("/dev/so101_cam_overview", 1280, 480, 60),
    "wrist": ("/dev/so101_cam_wrist", 1280, 720, 30),
}
NAME = sys.argv[1] if len(sys.argv) > 1 else "overview"
DEV, WIDTH, HEIGHT, FPS = CAMERAS[NAME]
PORT = 18080  # 8080 은 다른 개발 서버와 겹치기 쉬워 잘 안 쓰이는 번호로 둔다. 바꾸려면 이 줄만 고친다
STREAM_FPS = 15  # 브라우저로 보내는 속도. 터널 대역폭을 아끼려고 카메라 fps 보다 낮춘다

cap = cv2.VideoCapture(DEV, cv2.CAP_V4L2)  # 리눅스 카메라 (V4L2) 로 연다
cap.set(
    cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*"MJPG")
)  # 압축 전송 모드 (무압축 YUYV 는 fps 가 낮다)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, WIDTH)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, HEIGHT)
cap.set(cv2.CAP_PROP_FPS, FPS)
if not cap.isOpened():
    sys.exit(
        f"카메라를 열 수 없다: {DEV} (다른 프로세스가 쓰는 중이거나 so101-attach 가 필요하다)"
    )


class Handler(BaseHTTPRequestHandler):
    """브라우저 요청 1건에 JPEG 프레임을 끊임없이 이어 보낸다 (multipart/x-mixed-replace)."""

    def do_GET(self):
        """연결이 끊길 때까지 프레임을 읽어 JPEG 로 보낸다."""
        self.send_response(200)
        self.send_header("Content-Type", "multipart/x-mixed-replace; boundary=frame")
        self.end_headers()
        skip = max(1, FPS // STREAM_FPS)  # 카메라 프레임 몇 장마다 1장을 보낼지
        count = 0
        try:
            while True:
                ok, frame = (
                    cap.read()
                )  # 매 프레임 읽어서 버퍼를 비운다 (안 그러면 화면이 밀린다)
                if not ok:
                    time.sleep(0.05)
                    continue
                count += 1
                if count % skip:
                    continue  # 보내지 않는 프레임은 버린다
                ok, jpg = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 70])
                data = jpg.tobytes()
                self.wfile.write(b"--frame\r\nContent-Type: image/jpeg\r\n")
                self.wfile.write(f"Content-Length: {len(data)}\r\n\r\n".encode())
                self.wfile.write(data + b"\r\n")
        except (BrokenPipeError, ConnectionResetError):
            pass  # 브라우저 탭을 닫음

    def log_message(self, *args):
        """요청 로그를 찍지 않는다 (터미널이 지저분해진다)."""


print(
    f"{NAME}: {DEV} {WIDTH}x{HEIGHT}@{FPS} -> http://localhost:{PORT}  (Ctrl+C 로 종료)"
)
try:
    HTTPServer(
        ("127.0.0.1", PORT), Handler
    ).serve_forever()  # 컨테이너 밖으로는 열지 않는다
except KeyboardInterrupt:
    pass
finally:
    cap.release()  # 카메라를 놓아 lerobot 이 다시 열 수 있게 한다
    print("카메라 반환")
