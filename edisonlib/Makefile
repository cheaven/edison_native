#  
#  Unified Building System for Linux
#  By CSK (csk@live.com)
#  

PRJ_ROOT:=..
MOD_NAME:=edisonlib

include $(PRJ_ROOT)/scripts/def.mak


CSRC:= $(shell find core -name "*.c")
CXXSRC:= $(shell find core -name "*.cpp")

CSRC+= $(shell find libraries -name "*.c")
CXXSRC+= $(shell find libraries -name "*.cpp")

CSRC+= $(shell find variants/edison_fab_c -name "*.c")
CXXSRC+= $(shell find variants/edison_fab_c -name "*.cpp")


EXTRA_FLAGS:=-Wno-deprecated
CDEFS+=
DEP_AR+= 

INCLUDES+= -Icore/arduino \
	   -Ilibraries \
	   -Ivariants/edison_fab_c

all: $(STATIC_DEST)
additional_clean:
	rm -f $(STATIC_DEST)

include $(PRJ_ROOT)/scripts/common.mak



