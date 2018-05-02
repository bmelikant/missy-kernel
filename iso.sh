#!/bin/sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp kernel_test isodir/boot/kernel_test
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "Kernel Testbed System Multiboot 2" {
	multiboot2 /boot/kernel_test
}

menuentry "Kernel Testbed System Multiboot" {
	multiboot /boot/kernel_test
}
EOF
grub-mkrescue -o missy.iso isodir