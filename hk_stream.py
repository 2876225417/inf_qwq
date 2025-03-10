# cap video stream by RTSP

import cv2
#cap = cv2.VideoCapture("rtsp://admin:w12345678@169.254.36.138:554/Streaming/Channels/101")
cap = cv2.VideoCapture("rtsp://localhost:8554/cam")
while True:
    ret, frame = cap.read()
    if not ret: break
    cv2.imshow('Stream', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv2.destroyAllWindows()

