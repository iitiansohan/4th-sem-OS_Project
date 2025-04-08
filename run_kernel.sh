#!/bin/bash

# Compile the kernel
i686-elf-gcc -ffreestanding -m32 -c kernel.c -o kernel.o

# Link the kernel with the linker script
i686-elf-ld -T linker.ld -o kernel.elf kernel.o

# Run the kernel in QEMU with PC speaker enabled
qemu-system-i386 -kernel kernel.elf
