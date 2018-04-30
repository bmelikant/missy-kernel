#!/bin/sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp kernel_test isodir/boot/kernel_test
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "Kernel Testbed System" {
	multiboot2 /boot/kernel_test
}
EOF
grub-mkrescue -o missy.iso isodir