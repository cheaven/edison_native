#  
#  Unified Building System for Linux
#  By CSK (csk@live.com)
#  

PRJ_ROOT:=..
MOD_NAME:=$(notdir $(CURDIR))

include $(PRJ_ROOT)/scripts/def.mak


CSRC = $(shell find . -name "*.c")
CXXSRC= $(shell find . -name "*.cpp")

EXTRA_FLAGS:=-Wno-deprecated
CDEFS+= 
DEP_AR+=  $(OUTPUT_ROOT)/oled_dispatcher.a $(OUTPUT_ROOT)/edisonlib.a

INCLUDES+= -I$(PRJ_ABS_ROOT)/oled_dispatcher/src


all: $(EXEC_DEST)
additional_clean:
	rm -f $(EXEC_DEST)

include $(PRJ_ROOT)/scripts/common.mak



