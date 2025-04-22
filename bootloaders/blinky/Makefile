SRCS := main.c
DIR := bootloaders/blinky
TARGET := blinky
MODLIBS := 
MODULES := 
LIBS :=

ifeq (${MCU}, ${filter ${MCU}, atmega644p at90usb646 at90usb647})
BOOT_START_ADDR := 0xe000
endif
ifeq (${MCU}, ${filter ${MCU}, atmega1284p at90usb1286 at90usb1287})

OPT:=s
# LDFLAGS +=                                        

${DIR}_INCLUDES += \
	${patsubst %,-I${ROOT}/libs/%, 	    ${LIBS}}       \
	${patsubst %,-I${ROOT}/modules/%, 	${MODLIBS}} \
	${patsubst %,-I${ROOT}/modules/%, 	${MODULES}} \
	-I${ROOT}/module
${DIR}_CFLAGS += ${${DIR}_INCLUDES}
${DIR}_LDFLAGS +=

${BLD_DIR}/${DIR}/${TARGET}.boot.elf :: ${patsubst %,${BLD_DIR}/modules/%/, ${MODLIBS} ${MODULES}}
${BLD_DIR}/${DIR}/${TARGET}.elf :: ${patsubst %,${BLD_DIR}/modules/%/, ${MODLIBS} ${MODULES}}

${DIR}/flash_boot :: ${BLD_DIR}/${DIR}/${TARGET}.boot.hex
	@echo "Flashing $<"
	avrdude -c c232hm -p usb1286 -U flash:w:$<


include ${MAKEDIR}/bootloader.mk