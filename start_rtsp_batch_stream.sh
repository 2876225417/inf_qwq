#!/bin/bash

# Launched RTSP stream emualtor 

cameras=("/dev/video0" "/dev/video2" "/dev/video4")
rtsp_port=8554
base_name="cam"

if ! pgrep -x "mediamtx" > /dev/null; then
    mediamtx &
fi

for index in "${!cameras[@]}"; do
    device="${cameras[$index]}"
    stream_name="${base_name}${index}"
   
    ffmpeg -loglevel error \
           -f v4l2 \
           -input_format mjpeg \
           -i "$device" \
           -c:v copy \
           -f rtsp \
           "rtsp://localhost:$rtsp_port/$stream_name" &
done

echo "Launched ${#cameras[@]}: "
for index in "${!cameras[@]}"; do
    echo "rtsp://localhost:$rtsp_port/${base_name}${index}"
done
