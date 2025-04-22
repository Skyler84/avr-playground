DIR := modules/fat
SRCS := fat.c
TARGET := fat
LOAD_ADDR := 0xb000
MODULES := blockdev module vfs sd
LIBS :=
MODLIBS := 


${DIR}_INCLUDES += \
	${patsubst %,-I${ROOT}/libs/%, 	    ${LIBS}}       \
	${patsubst %,-I${ROOT}/modules/%, 	${MODLIBS}} \
	${patsubst %,-I${ROOT}/modules/%, 	${MODULES}} \
	-I${ROOT}/module

${DIR}_CFLAGS += ${${DIR}_INCLUDES}

include ${MAKEDIR}/module.mk
# include ${MAKEDIR}/lib.mk