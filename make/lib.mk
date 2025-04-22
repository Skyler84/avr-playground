# Requires a few input variables
#   SRCS -> C source files
#   DIR  -> directory of source
#   TARGET -> name of target to build


${DIR}_LIB_DISCARDS := ${BLD_DIR}/${DIR}/lib${TARGET}.a ${SRCS:%.c=${BLD_DIR}/${DIR}/%.o}
${DIR}_LIB_CSRC := ${SRCS:%.c=${DIR}/%.c}
${DIR}_LIB_OBJS := ${SRCS:%.c=${BLD_DIR}/${DIR}/%.o}

DIR_FROM_LIB_OBJ = ${patsubst ${BLD_DIR}/%/$*.o,%,$@}
${DIR}_LIB_CFLAGS :=  ${CFLAGS} ${${DIR}_CFLAGS} -DMODULE_AS_STATIC_LIB
${DIR}_LIB_LDFLAGS :=  ${LDFLAGS} ${${DIR}_LDFLAGS} 

include ${MAKEDIR}/internal/targets.mk

${BLD_DIR}/${DIR}/all:: ${BLD_DIR}/${DIR}/lib${TARGET}.a

${BLD_DIR}/${DIR}/lib${TARGET}.a : ${BLD_DIR}/%/lib${TARGET}.a : ${${DIR}_LIB_OBJS}
	${AR} rcs $@ $^
	${SIZE} $@

${${DIR}_LIB_OBJS} : ${BLD_DIR}/${DIR}/%.o : ${DIR}/%.c
	mkdir -p ${@D}
	${CC} ${${DIR_FROM_LIB_OBJ}_LIB_CFLAGS}  -c $< -o $@

${BLD_DIR}/${DIR}/clean::
	rm -f ${${patsubst ${BLD_DIR}/%/clean,%,$@}_LIB_DISCARDS}

undefine LOAD_ADDR