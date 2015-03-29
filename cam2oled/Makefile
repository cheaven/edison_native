#
#  Simple OpenCV Validator
#  
#  By CSK

PRJ_ROOT:=..
MOD_NAME:=$(notdir $(CURDIR))


include $(PRJ_ROOT)/scripts/def.mak


CSRC = $(shell find . -name "*.c")
CXXSRC= $(shell find . -name "*.cpp")


CDEFS+= 
INCLUDES+= -I$(PRJ_ABS_ROOT)/oled_dispatcher/src
DEP_AR+= $(OUTPUT_ROOT)/oled_dispatcher.a $(OUTPUT_ROOT)/edisonlib.a 
DEP_LIBS +=   $(OPENCV_LIBS)

all: $(EXEC_DEST)

additional_clean:
	rm -r -f $(EXEC_DEST)

include $(PRJ_ROOT)/scripts/common.mak
