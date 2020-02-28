#
# File   : Makefile
# Remark : Makefile for ARM Cortex-M processor based MCU
#

!INCLUDE "Makefile.inc"

TGT_NAME			= NanoOS

SRCS_DIR			= $(ROOT_DIR)\Src
OUT_BIN_DIR			= $(OUT_DIR)\Bin\$(TGT_NAME)
OUT_OBJ_DIR			= $(OUT_DIR)\Objs\$(TGT_NAME)

TGT					= $(TGT_NAME).bin
ELF					= $(TGT_NAME).elf

SRCS				= \
					  $(SRCS_DIR)\Exception.c\
					  $(SRCS_DIR)\Clock.c\
					  $(SRCS_DIR)\Debug.c\
					  $(SRCS_DIR)\Memory.c\
					  $(SRCS_DIR)\Core.c\
					  $(SRCS_DIR)\Thread.c\
					  $(SRCS_DIR)\Service.c\
					  $(SRCS_DIR)\Event.c\
					  $(SRCS_DIR)\Usart.c\
					  $(SRCS_DIR)\Printf.c\
					  $(SRCS_DIR)\Console.c\
					  $(SRCS_DIR)\UsbSerial.c\
					  $(SRCS_DIR)\UsbService.c\
					  $(SRCS_DIR)\UsbDesc.c\
					  $(SRCS_DIR)\Flash.c\
					  $(SRCS_DIR)\Config.c

LINK_OPTS			= \
!IFDEF _DEBUG
					  -Map=$(OUT_OBJ_DIR)\$(TGT_NAME)Dbg.map\
!ELSE
					  -Map=$(OUT_OBJ_DIR)\$(TGT_NAME)Rel.map\
!ENDIF
#					  --gc-sections\
					  -T$(SRCS_DIR)\NanoOS.ld\
					  -o$(OUT_BIN_DIR)\$(ELF)
					  
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
	@if exist "$(OUT_BIN_DIR)" erase /S /Q "$(OUT_BIN_DIR)\*.*"
	@if exist "$(OUT_OBJ_DIR)" erase /S /Q "$(OUT_OBJ_DIR)\*.*"

rebuild		: clean all
