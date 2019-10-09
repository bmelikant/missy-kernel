#
# MISSY Microsystem kernel makefile
# Outline taken from OSDev.org Meaty Skeleton project, modified to meet the needs
# of MISSY Microsystem Alpha 3.0 build
#
# Ben Melikant, 8/2016
#

HOST?=i686-elf
HOSTARCH:=i386

CC:=i686-elf-gcc

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -fbuiltin -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_missy_kernel -DDEBUG_BUILD -D__build_i386 -Iinclude -Iarch -Ikernel 
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lgcc

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

EARLY_OBJS:=\
	src/init/kinit.o \
	src/init/kterm.o \
	src/init/kutils.o \
	src/init/multiboot/api.o \
	src/init/multiboot/mboot.o \
	src/init/multiboot/mboot2.o \
	src/init/memory/kmemlow.o \
	asm/kernel_stub.o

OBJS:=\
$(EARLY_OBJS)

CRTI_OBJ:=asm/crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtend.o)
CRTN_OBJ:=asm/crtn.o

ALL_MY_OBJS:=\
$(CRTI_OBJ) \
$(OBJS) \
$(CRTN_OBJ)

OBJ_LINK_LIST:=\
$(CRTI_OBJ) \
$(CRTBEGIN_OBJ) \
$(OBJS) \
$(CRTEND_OBJ) \
$(CRTN_OBJ)

all: kernel_test

.PHONY: all clean

kernel_test: $(OBJ_LINK_LIST) link.ld
	$(CC) -T link.ld -o $@ $(CFLAGS) $(OBJ_LINK_LIST) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

%.o: %.asm
	nasm $< -o $@ -felf

%.o: %.S
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f kernel_test $(OBJS) $(ALL_OUR_OBJS) *.o */*.o */*/*.o
