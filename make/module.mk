# Requires a few input variables
#   SRCS -> C source files
#   DIR  -> directory of source
#   TARGET -> name of target to build
#   LOAD_ADDR -> hexadecimal address to load module

ifndef LOAD_ADDR
$(error LOAD_ADDR not defined for module target ${TARGET} in ${DIR})
endif

${DIR}_MOD_DISCARDS := ${DIR}/${TARGET}.mod ${patsubst %.c,${DIR}/%.mod.o, ${SRCS}}
${DIR}_MOD_CSRC := ${patsubst %.c,${DIR}/%.c,     ${SRCS}}
${DIR}_MOD_OBJS := ${patsubst %.c,${BLD_DIR}/${DIR}/%.mod.o, ${SRCS}}
${DIR}/${TARGET}_LOAD_ADDR := ${LOAD_ADDR}

DIR_FROM_MOD_OBJ = ${patsubst %/$*.mod.o,%$@}

${DIR}/:: ${DIR}/all
${DIR}/all:: ${BLD_DIR}/${DIR}/all
${BLD_DIR}/${DIR}/all:: ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.mod.%, elf hex bin}

${BLD_DIR}/${DIR}/${TARGET}.mod.bin : ${BLD_DIR}/%.mod.bin : ${BLD_DIR}/${DIR}/${TARGET}.mod.elf
	${OBJCOPY} -O binary $< $@

${BLD_DIR}/${DIR}/${TARGET}.mod.hex : ${BLD_DIR}/%.mod.hex : ${BLD_DIR}/${DIR}/${TARGET}.mod.elf
	${OBJCOPY} -O ihex $< $@ --change-addresses ${$*_}

${BLD_DIR}/${DIR}/${TARGET}.mod.elf : ${BLD_DIR}/%/${TARGET}.mod.elf : ${${DIR}_MOD_OBJS}
	${CC} ${LDFLAGS} ${$*_LDFLAGS} $^ -o $@
	${SIZE} $@

${${DIR}_MOD_OBJS} : ${DIR}/%.mod.o : ${DIR}/%.c
	${CC} ${CFLAGS} ${${DIR_FROM_MOD_OBJ}_CFLAGS}  -c $< -o $@

clean:: ${DIR}/clean
${DIR}/clean::
	rm -f ${${patsubst %/clean,%,$@}_MOD_DISCARDS}

undefine LOAD_ADDR