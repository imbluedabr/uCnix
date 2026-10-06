# uCnix

![Kernel Build](https://github.com/imbluedabr/uCnix/actions/workflows/c-cpp.yml/badge.svg)

uCnix is a *non* posix compliant toy kernel for extremely resource constrained microcontrollers.
Currently i am targeting arm/riscv microcontrollers with around 64KiB sram but i also want to suport more powerfull mcu's like the rp2040/rp2350.

## Suported devices
- MCXA153VFM
- LPC55S69 (barely suported)

## Dependencies

- GNU Make
- cross compiler toolchain (for ARM this is arm-none-eabi)

## Building

1. clone and cd to the repo root.
2. build the kernel: `make kernel`
3. install the kernel and headers into a sysroot: `make SYSROOT=$SYSROOT install`
4. profit.


