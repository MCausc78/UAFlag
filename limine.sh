#!/bin/bash
git clone https://github.com/limine-bootloader/limine.git \
  --branch=v4.x-branch-binary \
  --depth=1

make -C limine

rm -frv ./iso_root
mkdir -p iso_root/
cp -v uaflag.bin \
  limine.cfg \
  limine/limine.sys \
  limine/limine-cd.bin \
  limine/limine-cd-efi.bin \
  iso_root/

xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-cd-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o uaflag.iso

./limine/limine-deploy uaflag.iso