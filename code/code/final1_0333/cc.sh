#!/bin bash
make clean
make
adb push gobang /data/local
adb shell /data/local.gobang
