#!/bin/sh
EMU=../lpc55s69/m33mu/build/m33mu
FLAG=""

if [ "$1" = "debug" ]; then
    FLAG="--gdb --gdb-symbols kernel.elf"
fi

$EMU $FLAG --uart-stdout --cpu lpc55s69 ./image.bin


