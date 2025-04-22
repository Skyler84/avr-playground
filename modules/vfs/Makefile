DIR := modules/vfs
SRCS := vfs.c
TARGET := vfs
LOAD_ADDR := 0xa000
MODULES := blockdev module
LIBS :=
MODLIBS := 
INTERFACES := fs


${DIR}_INCLUDES += \
	${patsubst %,-I${ROOT}/libs/%, 	    ${LIBS}}       \
	${patsubst %,-I${ROOT}/interfaces/%,${INTERFACES}} \
	${patsubst %,-I${ROOT}/modules/%, 	${MODLIBS}} \
	${patsubst %,-I${ROOT}/modules/%, 	${MODULES}} \
	-I${ROOT}/module

${DIR}_CFLAGS += ${${DIR}_INCLUDES}

include ${MAKEDIR}/module.mk
include ${MAKEDIR}/lib.mk