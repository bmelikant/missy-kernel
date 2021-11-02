#!/bin/bash
set -e
. ./iso.sh

serialtty="com1: enabled=1,mode=null"
echo $serialtty

$MAGIC_BREAK=""

for i in "$@"; do
	case -m|--magic-break)
		$MAGIC_BREAK="magic_break: enabled=1"
		shift
		;;
	*)
		echo "Unknown option"
		shift
		;;
	esac
done

if [[ "$serialout" != "" ]]; then
	echo "using serial port tty $serialout"
	serialtty="com1: enabled=1,mode=term,dev=$serialout"
fi

bochs 'boot:cdrom' 'ata1-master: type=cdrom,path=missy.iso,status=inserted' "memory: guest=128,host=128" "$serialtty" "cpu: reset_on_triple_fault=0" $MAGIC_BREAK