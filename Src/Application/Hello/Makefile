#
# Makefile for ARM Cortex-M processor based MCU
# Minimal Makefile to compile Hello sample
#

CORT_VER			= m3
TGT_CPU				= arm_cortex_$(CORT_VER)

###
# Output directory (OUT_DIR), this directory will contain .elf and .bin file as result of building process
# change this line if you want specify another directory as build output
#
OUT_DIR				= D:\DevDevel\NanoOS\Build\MSVS2K8\Hello\Bin

###
# Source directory, change this line if want ot specify another directory as your source directory
#
SRCS_DIR			= D:\DevDevel\NanoOS\Src\Application\Hello

###
# Include path, change this line to specify another include path
#
INCLUDE				= D:\DevDevel\NanoOS\Src\Application

###
# GCC_BIN_DIR is directory where gcc.exe. ld.exe and so on exist
# Change the line below if the gcc compiler in on somewhere else, for example GCC_BIN_DIR = D:\OtherDir
#
GCC_BIN_DIR			= D:\DevDevel\gcc

PATH				= $(GCC_BIN_DIR);$(PATH)

CC					= gcc.exe
					  
CC_OPTS				= \
!IFDEF _DEBUG
					  -Wall\
					  -D_DEBUG\
					  -g\
!ENDIF
					  -I$(INCLUDE)\
					  -mcpu=cortex-$(CORT_VER)\
					  -mthumb\
					  -ffunction-sections\
					  -O0\
					  -mfix-cortex-m3-ldrd\
					  -c
					  
LINK				= ld.exe
LINK_OPTS			= \
					  --gc-sections\
					  -T$(INCLUDE)\NanoApp.ld\
					  -o$(OUT_DIR)\$(ELF)

					  
GEN_BIN				= objcopy.exe
GEN_BIN_OPT			= -Obinary

TGT_NAME			= Hello

TGT					= $(TGT_NAME).bin
ELF					= $(TGT_NAME).elf

SRCS				= \
					  $(SRCS_DIR)\Hello.c\
					  $(SRCS_DIR)\..\NanoApi.c

all			: $(TGT)

prep		:
	@echo Preparing...
	@if not exist "$(OUT_DIR)" mkdir "$(OUT_DIR)"
	@set OBJ_FILES=
	
$(SRCS)		: prep
	@echo Compiling $@
	@set OBJ_FILES=%%OBJ_FILES%% $(*B).obj
	@$(CC) $(CC_OPTS) -o $(*B).obj $@

$(ELF)		: $(SRCS)
	@echo Linking...
	@$(LINK) $(LINK_OPTS) %%OBJ_FILES%%

$(TGT)		: $(ELF)
	@echo Generating binary...
	@$(GEN_BIN) $(GEN_BIN_OPT) $(OUT_DIR)\$(ELF) $(OUT_DIR)\$(TGT)

clean		:
	@echo Cleaning up...
	@if exist "$(OUT_DIR)\*.*" erase /S /Q "$(OUT_DIR)\*.*"
	@erase /S /Q *.obj

rebuild		: clean all
