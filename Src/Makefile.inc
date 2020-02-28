#
# File   : Makefile.inc
# Remark : Makefile for ARM Cortex-M processor based MCU
#

CORT_VER			= m3
TGT_CPU				= arm_cortex_$(CORT_VER)
OUT_NAME			= NanoOSOut

# Change the line below to if the root of your source directory is other than D:\DevDevel\NanoOS
# for example ROO_DIR = D:\Dev\NanoOS
ROOT_DIR			= D:\DevDevel\NanoOS


# Change the line below if the gcc compiler in on somewhere else, for example GCC_BIN_DIR = D:\OtherDir
GCC_BIN_DIR			= $(ROOT_DIR)\..\gcc


OUT_DIR				= $(ROOT_DIR)\..\$(OUT_NAME)

PATH				= $(GCC_BIN_DIR);$(PATH)

CC					= gcc.exe
					  
CC_OPTS				= \
					  -D_CHIP_STM32F10XXX\
!IFDEF _DEBUG
					  -Wall\
					  -D_DEBUG\
					  -g\
!ENDIF
					  -mcpu=cortex-$(CORT_VER)\
					  -mthumb\
					  -ffunction-sections\
					  -O0\
					  -mfix-cortex-m3-ldrd\
					  -c
					  
LINK				= ld.exe
					  
GEN_BIN				= objcopy.exe
GEN_BIN_OPT			= -Obinary

OBJ_DUMP			= objdump.exe
OBJ_DUMP_OPT		= -d
