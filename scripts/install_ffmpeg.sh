#!/bin/bash

echo "Install FFMPEG libraries"

sudo apt-get update
sudo apt install -y \
 libavutil-dev      \
 libavdevice-dev    \
 libavfilter-dev    \
 libavcodec-dev     \
 libavformat-dev    \
 libswscale-dev     \
 libresample-dev    \
 libswresample-dev  \
 libpostproc-dev
