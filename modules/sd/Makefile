DIR := modules/sd
SRCS := sd.c
TARGET := sd
LOAD_ADDR := 0xa000
MODULES := blockdev module
LIBS :=
MODLIBS := 


${DIR}_INCLUDES += \
	${patsubst %,-I${ROOT}/libs/%, 	    ${LIBS}}       \
	${patsubst %,-I${ROOT}/modules/%, 	${MODLIBS}} \
	${patsubst %,-I${ROOT}/modules/%, 	${MODULES}} \
	-I${ROOT}/module

${DIR}_CFLAGS += ${${DIR}_INCLUDES}

include ${MAKEDIR}/module.mk
include ${MAKEDIR}/lib.mk