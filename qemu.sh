#!/bin/bash
set -e
. ./iso.sh

qemu-system-i386 -cdrom missy.iso -m 128 -serial mon:stdio