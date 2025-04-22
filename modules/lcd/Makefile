DIR := modules/lcd
SRCS := lcd.c
TARGET := lcd
MODULES := fonts
LOAD_ADDR := 0x8000
INTERFACES := display

${DIR}_CFLAGS += \
	${patsubst %,-I${ROOT}/interfaces/%, ${INTERFACES}}\
	${patsubst %,-I${ROOT}/modules/%, 	${MODULES}}\
	-I${ROOT}/modules/module

include ${MAKEDIR}/module.mk
include ${MAKEDIR}/lib.mk