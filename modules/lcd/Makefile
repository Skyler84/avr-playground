DIR := modules/lcd
SRCS := lcd.c
TARGET := lcd
LOAD_ADDR := 0x8000

${DIR}_CFLAGS += -I${ROOT}/modules/fonts -I${ROOT}/modules/module

include ${MAKEDIR}/module.mk
include ${MAKEDIR}/lib.mk