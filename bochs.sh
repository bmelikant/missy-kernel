#!/bin/bash
set -e
. ./iso.sh

bochs 'boot:cdrom' 'ata1-master: type=cdrom,path=missy.iso,status=inserted' 'memory: guest=128,host:128'