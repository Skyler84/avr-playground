# Makefile for the Fonts module

TARGET := fonts
UPDIR := modules
DIR := ${UPDIR}/fonts
SRCS := fonts.c font5x7.c
LOAD_ADDR := 0x7000

${DIR}_LDFLAGS += ${${UPDIR}_LDFLAGS}
${DIR}_INCLUDES += ${${UPDIR}_INCLUDES}
${DIR}_CFLAGS += ${${UPDIR}_CFLAGS} \
	-I${ROOT}/modules/module


include ${MAKEDIR}/module.mk
include ${MAKEDIR}/lib.mk
