#!/bin/bash
set -e
. ./iso.sh

serialtty="com1: enabled=1,mode=null"
echo $serialtty

if [[ "$serialout" != "" ]]; then
	echo "using serial port tty $serialout"
	serialtty="com1: enabled=1,mode=term,dev=$serialout"
fi

bochs 'boot:cdrom' 'ata1-master: type=cdrom,path=missy.iso,status=inserted' "memory: guest=128,host=128" "$serialtty" "cpu: reset_on_triple_fault=0"