# PyMite: Configuration
PLATFORM := $(notdir $(CURDIR))
PM_LIB_ROOT = pmvm_$(PLATFORM)
PM_LIB_FN = lib$(PM_LIB_ROOT).a
PM_LIB_PATH = ../../vm/$(PM_LIB_FN)
PM_USR_SOURCES = main.py ../../lib/stm.py
PMIMGCREATOR := ../../tools/pmImgCreator.py
PMGENPMFEATURES := ../../tools/pmGenPmFeatures.py
TARGET = main
TCHAIN = arm-none-eabi-
LINKERSCRIPT=stm32f091rc.ld
MCU = cortex-m0

DEBUG = true

# Toolchain path
EXT_TOOL_PATH := $(abspath .)/../../../ext_tools/gcc-arm-none-eabi-4_8-2013q4/bin

# Toolchain prefix
TOOL_CHAIN = arm-none-eabi-

TCHAIN = $(EXT_TOOL_PATH)/$(TOOL_CHAIN)

# If you want to be able to run ipm, uncomment the following line
#IPM = true


SRC = 	$(TARGET).c plat.c \
		$(TARGET)_nat.c \
		$(TARGET)_img.c \
		syscalls.c \
		stm32f0xx_nucleo.c \
		stm32f0xx_it.c

SRC_HAL_PATH = ST/hal/src
SRC_HAL_INC = $(abspath ST/hal/inc)
#SRC += $(wildcard ST/hal/src/*.c)
#SRC += $(wildcard ST/btle/src/*.c)

SRC_HAL = \
	$(SRC_HAL_PATH)/stm32f0xx_hal.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_comp.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_cortex.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_gpio.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_pwr.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_rcc.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_rcc_ex.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_rtc_ex.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_spi.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_tim.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_tim_ex.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_uart.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_uart_ex.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_usart.c \
	$(SRC_HAL_PATH)/stm32f0xx_hal_dma.c \
	$(SRC_HAL_PATH)/system_stm32f0xx.c

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC =

ASRC_HAL = \
	$(SRC_HAL_PATH)/startup_stm32f091xc.S

# List any extra directories to look for library files here.
#     Each directory must be seperated by a space.

LIBDIRS = . ../../vm

LIBS =$(PM_LIB_ROOT)

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

CDEFS =
# PyMite: CFLAGS

# Optimization level, can be [0, 1, 2, 3, s].
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
#DEBUG = stabs
DEBUG_FMT = dwarf-2
#DEBUG_FMT = 

ifeq ($(DEBUG),true)
	CDEFS += -g -g$(DEBUG_FMT) -D__DEBUG__=1
	OPT = 0
else
	OPT = s
endif

THUMB    = -mthumb

# Place -I options here
CINCS = -I$(abspath .)

# Place -D or -U options for ASM here
ADEFS =

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS = . ../../vm $(SRC_HAL_INC)

#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
PMCFLAGS = $(CDEFS) $(CINCS)
PMCFLAGS += -O$(OPT)
#PMCFLAGS += -Wcast-align
PMCFLAGS += -Wall -Wimplicit -mlittle-endian
PMCFLAGS +=  -ffunction-sections -fdata-sections -fno-strict-aliasing
PMCFLAGS += -Wpointer-arith -Wswitch
PMCFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
#PMCFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<)
PMCFLAGS += -DUART_BAUD=9600 -DUSE_STDPERIPH_DRIVER -DSTM32F091xC
PMCFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(<:.S=.lst),--g$(DEBUG_FMT)

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
PMLDFLAGS =  -Wl,-Map=$(TARGET).map
#PMLDFLAGS += -Wl,-verbose
PMLDFLAGS +=  -lc -lm -mthumb -lgcc
PMLDFLAGS += -u _start -Wl,-e,Reset_Handler -Wl,-static -Wl,--gc-sections
PMLDFLAGS += $(patsubst %,-L%,$(LIBDIRS))
PMLDFLAGS += $(patsubst %,-l%,$(LIBS))
PMLDFLAGS +=-Wl,-T -Xlinker $(LINKERSCRIPT)


CFLAGS += $(CDEFS) $(CINCS)

# Define programs and commands.
CC = $(TCHAIN)gcc
CPP = $(TCHAIN)g++
OBJCOPY = $(TCHAIN)objcopy
OBJDUMP = $(TCHAIN)objdump
SIZE = $(TCHAIN)size
AR = $(TCHAIN)ar
NM = $(TCHAIN)nm
REMOVE = rm -f

# Define all object files.
COBJ      = $(SRC:.c=.o)
AOBJ      = $(ASRC:.S=.o)
COBJHAL   = $(SRC_HAL:.c=.o)
AOBJHAL   = $(ASRC_HAL:.S=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(ASRC_HAL:.S=.lst) $(SRC:.c=.lst) $(SRC_HAL:.c=.lst)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS  = -mcpu=$(MCU) $(THUMB) -I. $(PMCFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB) -I. -x assembler-with-cpp $(ASFLAGS)

export CFLAGS IPM PM_LIB_FN

# Default target.
all: pmfeatures.h pmvm build

# PyMite: Build the VM archive if it doesn't exist
pmvm : $(PM_LIB_PATH)

$(PM_LIB_PATH) : ../../vm/*.c ../../vm/*.h
	make -C ../../vm

build: elf

elf: $(TARGET).elf

pmfeatures.h : pmfeatures.py $(PMGENPMFEATURES)
		python2.6 $(PMGENPMFEATURES) pmfeatures.py > $@

# PyMite: Generate native code and module images from the python source
$(TARGET)_nat.c $(TARGET)_img.c : $(PM_USR_SOURCES) pmfeatures.py
		python2.6 $(PMIMGCREATOR) -f pmfeatures.py -c -u --memspace=flash -o $(TARGET)_img.c --native-file=$(TARGET)_nat.c $(PM_USR_SOURCES)

../../vm/pmstdlib_img.c ../../vm/pmstdlib_nat.c :
	$(MAKE) -C ../../vm pmstdlib_img.c pmstdlib_nat.c

%.elf:  $(AOBJHAL) $(AOBJ) $(COBJHAL) $(COBJ)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(THUMB) $(ALL_CFLAGS) $(AOBJHAL) $(AOBJ) $(COBJHAL) $(COBJ) --output $@ $(PMLDFLAGS)


# Compile: create object files from C source files. ARM/Thumb
$(COBJ) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@

# Compile: create object files from C source files. ARM/Thumb
$(COBJHAL) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@

# Assemble: create object files from assembler source files. ARM/Thumb
$(AOBJ) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@

# PyMite: Append pmvm to the list of .PHONY targets
.PHONY : all elf pmvm

# PyMite: Append these intermediate files to the list of things to clean
clean :
		$(MAKE) -C ../../vm clean
		$(REMOVE) $(TARGET)_img.c
		$(REMOVE) $(TARGET)_nat.c
		$(REMOVE) $(TARGET).elf
		$(REMOVE) $(TARGET).map
		$(REMOVE) $(COBJ)
		$(REMOVE) $(AOBJ)
		$(REMOVE) $(LST)
		$(REMOVE) $(SRC:.c=.s)
		$(REMOVE) $(SRC:.c=.d)
		$(REMOVE) $(SRC:.c=.o)
		$(REMOVE) $(SRC_HAL:.c=.s)
		$(REMOVE) $(SRC_HAL:.c=.d)
		$(REMOVE) $(SRC_HAL:.c=.o)
		$(REMOVE) pmfeatures.h | exit 0

# PyMite: Export these env variables so the Makefile in ../../vm gets them
export CC OBJCOPY NM CFLAGS ALL_CFLAGS AR IPM PM_LIB_FN
