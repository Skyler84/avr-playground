# Requires a few input variables
#   SRCS -> C source files
#   DIR  -> directory of source
#   TARGET -> name of target to build
#   LOAD_ADDR -> hexadecimal address to load module

ifndef LOAD_ADDR
$(error LOAD_ADDR not defined for module target ${TARGET} in ${DIR})
endif

${DIR}_MOD_DISCARDS := ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.%, elf hex bin} ${SRCS:%.c=${BLD_DIR}/${DIR}/%.mod.o}
${DIR}_MOD_CSRC := ${SRCS:%.c=${DIR}/%.c}
${DIR}_MOD_OBJS := ${SRCS:%.c=${BLD_DIR}/${DIR}/%.mod.o}
${DIR}/${TARGET}_LOAD_ADDR := ${LOAD_ADDR}

DIR_FROM_MOD_OBJ = ${patsubst ${BLD_DIR}/%/$*.mod.o,%,$@}
${DIR}_CFLAGS +=  ${CFLAGS} -I ${ROOT}/modules/module

include ${MAKEDIR}/internal/targets.mk

${BLD_DIR}/${DIR}/all:: ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.mod.%, elf hex bin}

${BLD_DIR}/${DIR}/${TARGET}.mod.bin : ${BLD_DIR}/%.mod.bin : ${BLD_DIR}/${DIR}/${TARGET}.mod.elf
	${OBJCOPY} -O binary $< $@

${BLD_DIR}/${DIR}/${TARGET}.mod.hex : ${BLD_DIR}/%.mod.hex : ${BLD_DIR}/${DIR}/${TARGET}.mod.elf
	${OBJCOPY} -O ihex $< $@ --change-addresses ${$*_LOAD_ADDR}

${BLD_DIR}/${DIR}/${TARGET}.mod.elf : ${BLD_DIR}/%/${TARGET}.mod.elf : ${${DIR}_MOD_OBJS}
	${CC} ${LDFLAGS} ${$*_LDFLAGS} $^ -o $@ -nostartfiles -nostdlib
	${SIZE} $@

${${DIR}_MOD_OBJS} : ${BLD_DIR}/${DIR}/%.mod.o : ${DIR}/%.c
	mkdir -p ${@D}
	@echo ${DIR_FROM_MOD_OBJ}_CFLAGS
	@echo ${${DIR_FROM_MOD_OBJ}_CFLAGS}
	${CC} ${${DIR_FROM_MOD_OBJ}_CFLAGS}  -c $< -o $@

${BLD_DIR}/${DIR}/clean::
	rm -f ${${patsubst ${BLD_DIR}/%/clean,%,$@}_MOD_DISCARDS}

undefine LOAD_ADDR