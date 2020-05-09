#
# Makefile for ARM Cortex-M processor based MCU
#

!INCLUDE "..\..\..\Src\Makefile.inc"

TGT_NAME			= VDispTest

SRCS_DIR			= $(ROOT_DIR)\Src\Application
OUT_BIN_DIR			= $(OUT_DIR)\Bin\Application\$(TGT_NAME)
OUT_OBJ_DIR			= $(OUT_DIR)\Objs\Application\$(TGT_NAME)

TGT					= $(TGT_NAME).bin
ELF					= $(TGT_NAME).elf

SRCS				= \
					  $(SRCS_DIR)\$(TGT_NAME)\VDispTest.c\
					  $(SRCS_DIR)\VDispLib.c\
					  $(SRCS_DIR)\NanoApi.c

LINK_OPTS			= \
					  --gc-sections\
					  -T$(SRCS_DIR)\NanoApp.ld\
					  -o$(OUT_BIN_DIR)\$(ELF)

GEN_BIN				= objcopy.exe
GEN_BIN_OPT			= -I elf32-littlearm -O binary

OBJ_DUMP			= objdump.exe
OBJ_DUMP_OPT		= -d

all			: $(TGT)

prep		:
	@echo Preparing...
	@if not exist "$(OUT_BIN_DIR)" mkdir "$(OUT_BIN_DIR)"
	@if not exist "$(OUT_OBJ_DIR)" mkdir "$(OUT_OBJ_DIR)"
	@set OBJ_FILES=
	
$(SRCS)		: prep
	@echo Compiling $@
	@set OBJ_FILES=%%OBJ_FILES%% $(OUT_OBJ_DIR)\$(*B).obj
	@$(CC) $(CC_OPTS) -o $(OUT_OBJ_DIR)\$(*B).obj $@

$(ELF)		: $(SRCS)
	@echo Linking...
	@$(LINK) $(LINK_OPTS) %%OBJ_FILES%%

$(TGT)		: $(ELF)
	@echo Generating binary...
	@$(GEN_BIN) $(GEN_BIN_OPT) $(OUT_BIN_DIR)\$(ELF) $(OUT_BIN_DIR)\$(TGT)
!IFDEF _DEBUG
	@$(OBJ_DUMP) $(OBJ_DUMP_OPT) $(OUT_BIN_DIR)\$(ELF) > $(OUT_OBJ_DIR)\$(TGT_NAME)Dbg.asm
!ELSE
	@$(OBJ_DUMP) $(OBJ_DUMP_OPT) $(OUT_BIN_DIR)\$(ELF) > $(OUT_OBJ_DIR)\$(TGT_NAME)Rel.asm
!ENDIF

clean		:
	@echo Cleaning up...
	@if exist "$(OUT_BIN_DIR)\$(TGT_NAME).*" erase /S /Q "$(OUT_BIN_DIR)\$(TGT_NAME).*"
	@if exist "$(OUT_OBJ_DIR)" erase /S /Q "$(OUT_OBJ_DIR)\*.*"

rebuild		: clean all
