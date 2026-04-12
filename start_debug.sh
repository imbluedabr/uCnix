#!/bin/bash

pyocd gdbserver --target mcxa153vlh > /dev/null 2>&1 &
../../arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gdb

