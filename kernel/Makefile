NAME           := kernel
SRCS           := framework/main.c $(shell find -L ./src/ -name "*.c" -not -path "./src/dev/*") test/pmm_test.c
#SRCS           := thread-os.c
INC_PATH       := include/ framework/ test/

export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH    := x86_64-qemu
endif

include $(AM_HOME)/Makefile
include ../Makefile.lab
image: git

test: git
	@gcc $(shell find src/ -name "*.c")  \
             $(shell find test/ -name "*.c") \
	     -I$(AM_HOME)/am/include -I$(AM_HOME)/klib/include \
             -Iframework -Iinclude -ggdb3 -DTEST -lpthread \
             -o build/test
	@build/test
