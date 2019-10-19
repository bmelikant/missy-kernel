#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/missy.kernel isodir/boot/missy.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "Kernel Testbed System Multiboot 2" {
	multiboot2 /boot/missy.kernel
}

menuentry "Kernel Testbed System Multiboot" {
	multiboot /boot/missy.kernel
}
EOF
grub-mkrescue -o missy.iso isodir