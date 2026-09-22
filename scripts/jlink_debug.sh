#!/bin/sh

JLinkGDBServer -device MCXA153 -if SWD -speed 4000 > /dev/null 2>&1 &
sleep 0.5
arm-none-eabi-gdb kernel.elf -x ./scripts/jlink.gdbinit

echo "killing $(pgrep -i jlink)"
pkill -i "jlink"
