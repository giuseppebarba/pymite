# PyMite: Configuration
PLATFORM := $(notdir $(CURDIR))
PM_LIB_ROOT = pmvm_$(PLATFORM)
PM_LIB_FN = lib$(PM_LIB_ROOT).a
PM_LIB_PATH = ../../vm/$(PM_LIB_FN)
PM_USR_SOURCES = main.py
PMIMGCREATOR := ../../tools/pmImgCreator.py
PMGENPMFEATURES := ../../tools/pmGenPmFeatures.py
TARGET = main

SRC = 	$(TARGET).c plat.c \
		$(TARGET)_nat.c \
		$(TARGET)_img.c \
		clock.c \
		syscalls.c \
		main.c \
		stm32f0xx_nucleo.c \
		stm32f0xx_it.c
		
SRC += $(wildcard ST/hal/src/*.c)
#SRC += $(wildcard ST/btle/src/*.c)
SRC += $(wildcard ST/hal/src/*.s)
# Define all object files.
OBJ = $(SRC:.c=.o) $(ASRC:.S=.o)

# If you want to be able to run ipm, uncomment the following line
IPM = true

# PyMite: CFLAGS
ifeq ($(DEBUG),true)
	CDEFS += -g -gstabs -D__DEBUG__=1
endif

CINCS += -I$(abspath .) -I$(abspath ST/hal/inc) -I$(abspath ST/btle/inc) -I../../vm
CFLAGS += $(CDEFS) $(CINCS)

EXT_TOOL := $(abspath .)/../../../ext_tools/gcc-arm-none-eabi-4_7-2013q1/bin/
# Define the toolchain
TOOL_CHAIN = $(EXT_TOOL)/arm-none-eabi
CC = $(TOOL_CHAIN)-gcc
CPP = $(TOOL_CHAIN)-g++
OBJCOPY = $(TOOL_CHAIN)-objcopy
OBJDUMP = $(TOOL_CHAIN)-objdump
SIZE = $(TOOL_CHAIN)-size
AR = $(TOOL_CHAIN)-ar
NM = $(TOOL_CHAIN)-nm
REMOVE = rm -f

# Default target.
all: pmfeatures.h pmvm build

# PyMite: Build the VM archive if it doesn't exist
pmvm : $(PM_LIB_PATH)

build: elf

elf: $(TARGET).elf

$(PM_LIB_PATH) : ../../vm/*.c ../../vm/*.h
		make -C ../../vm

pmfeatures.h : pmfeatures.py $(PMGENPMFEATURES)
		python2.6 $(PMGENPMFEATURES) pmfeatures.py > $@

# PyMite: Generate native code and module images from the python source
$(TARGET)_nat.c $(TARGET)_img.c : $(PM_USR_SOURCES)
		python2.6 $(PMIMGCREATOR) -f pmfeatures.py -c -u --memspace=flash -o $(TARGET)_img.c --native-file=$(TARGET)_nat.c $(PM_USR_SOURCES)

# PyMite: tell the compiler where to find pm.h and libpmvm_<plat>.a
CFLAGS += -DUART_BAUD=9600 -DUSE_STDPERIPH_DRIVER -DSTM32F091xC -Wall -mcpu=cortex-m0 -mthumb -O0 -g -mlittle-endian -mfloat-abi=soft
LDFLAGS = -Tstm32f091rc.ld  -v -nostartfiles -L../../vm -l$(PM_LIB_ROOT) -L.

# Link: create ELF output file from object files.
$(TARGET).elf: $(OBJ) $(PM_LIB_PATH)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJ)

# PyMite: Append pmvm to the list of .PHONY targets
.PHONY : all elf 

# PyMite: Append these intermediate files to the list of things to clean
clean :
		$(REMOVE) $(TARGET)_img.c
		$(REMOVE) $(TARGET)_nat.c
		$(REMOVE) $(OBJ)
		$(REMOVE) $(TARGET).elf

# PyMite: Export these env variables so the Makefile in ../../vm gets them
export CC OBJCOPY NM CFLAGS ALL_CFLAGS AR IPM PM_LIB_FN
