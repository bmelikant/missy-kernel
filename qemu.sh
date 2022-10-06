#!/bin/bash
set -e
. ./iso.sh

LOGGING=""

for i in "$@"; do
    case "$i" in
        -l|--log)
            LOGGING="-d int -D output.log"
            shift
            ;;
        *)
            echo "Unrecognized option"
            shift
            ;;
    esac
done

qemu-system-i386 -cdrom missy.iso -m 128 -serial mon:stdio $LOGGING