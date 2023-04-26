#!/bin/bash
TARGET=i686-elf

echo " AS boot.s"
$TARGET-as -o boot.o boot.s
echo " CC kernel.c"
$TARGET-gcc -O2 \
  -Wall \
  -Wextra \
  -c \
  -ffreestanding \
  -std=gnu99 \
  -o kernel.o \
  kernel.c

echo " LD"
$TARGET-gcc -T link.ld \
  -o uaflag.bin \
  -ffreestanding \
  -O2 \
  -nostdlib \
  boot.o \
  kernel.o \
  -lgcc