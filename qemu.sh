#!/bin/bash
set -e
. ./iso.sh

qemu-system-i386 -gdb tcp::1234 -cdrom missy.iso -m 512