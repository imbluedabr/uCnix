# uCnix

![Kernel Build](https://github.com/imbluedabr/uCnix/actions/workflows/c-cpp.yml/badge.svg)

uCnix is a toy oprating system for extremely resource constrained microcontrollers.
Currently i am targeting arm/riscv microcontrollers with around 64KiB sram but i also want to suport more powerfull mcu's like the rp2040/rp2350.

## Suported devices
- MCXA153VFM

## Dependencies

- GNU Make
- cross compiler toolchain (for ARM this is arm-none-eabi)

## Building

1. clone and cd to the repo root.
2. build the tool(s): `make tools`
3. create a filesystem: `tools/mkfs ./staging`
4. build the kernel image: `make image`
5. flash the image onto your microcontroller(see Flashing).
3. profit.

> [!NOTE]
> the build system is still in its early stages so this is subject to change.

## Flashing

To actually install the operating system on a microcontroller you have to flash `image.bin` or `kernel.elf`. Below are flashing instructions per suported microcontroller.

### MCXA153VFM

1. install the `pyocd` debugger/programmer.
2. install the mcxa153 pack: `pyocd pack install MCXA153VFM`
3. connect the microcontroller.
4. check the connection, the mcxa153 should show up: `pyocd list`
5. flash the image: `pyocd load --format=bin image.bin`
6. connect a serial terminal to the usb virtual com port.
7. done.

