#!/bin/bash
sudo pkill sway
unset DISPLAY
unset WAYLAND_DISPLAY
export XDG_RUNTIME_DIR=/run/user/$(id -u)
export DISPLAY=:0
export WAYLAND_DISPLAY=wayland-0

LOG_DIR="./logs"
mkdir -p "$LOG_DIR"
LOG_FILE="$LOG_DIR/compositor_$(date +'%Y-%m-%d_%H-%M-%S').log"
./compositor > "$LOG_FILE" 2>&1
