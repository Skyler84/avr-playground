# Requires a few input variables
#   SRCS -> C source files
#   DIR  -> directory of source
#   TARGET -> name of target to build
#   LOAD_ADDR -> hexadecimal address to load module

ifndef LOAD_ADDR
$(error LOAD_ADDR not defined for module target ${TARGET} in ${DIR})
endif

${DIR}_MOD_DISCARDS := ${BLD_DIR}/${DIR}/ ${BLD_DIR}/${DIR}.*
${DIR}_MOD_CSRC := ${SRCS:%.c=${DIR}/%.c}
${DIR}_MOD_OBJS := ${SRCS:%.c=${BLD_DIR}/${DIR}/%.mod.o}
${DIR}/${TARGET}_LOAD_ADDR := ${LOAD_ADDR}

DIR_FROM_MOD_OBJ = ${patsubst ${BLD_DIR}/%/$*.mod.o,%,$@}
${DIR}_MOD_CFLAGS :=  ${CFLAGS} ${${DIR}_CFLAGS} -I ${ROOT}/modules/module
${DIR}_MOD_LDFLAGS :=  ${LDFLAGS} ${${DIR}_LDFLAGS} -T ${ROOT}/modules/module/module.x

include ${MAKEDIR}/internal/targets.mk

${BLD_DIR}/${DIR}/all:: ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.mod.%, elf hex bin}
${BLD_DIR}/${DIR}/all:: ${BLD_DIR}/${DIR}.mod.bin

${BLD_DIR}/${DIR}/${TARGET}.mod.bin : ${BLD_DIR}/%.mod.bin : ${BLD_DIR}/${DIR}/${TARGET}.mod.elf
	${OBJCOPY} -O binary $< $@
	
${BLD_DIR}/${DIR}.mod.bin : ${BLD_DIR}/%.mod.bin : ${BLD_DIR}/${DIR}/${TARGET}.mod.elf
	${OBJCOPY} -O binary $< $@

${BLD_DIR}/${DIR}/${TARGET}.mod.hex : ${BLD_DIR}/%.mod.hex : ${BLD_DIR}/${DIR}/${TARGET}.mod.elf
	${OBJCOPY} -O ihex $< $@ --change-addresses ${$*_LOAD_ADDR}

${BLD_DIR}/${DIR}/${TARGET}.mod.elf : ${BLD_DIR}/%/${TARGET}.mod.elf : ${${DIR}_MOD_OBJS}
	${CC} $^ -o $@ -nostartfiles ${$*_MOD_LDFLAGS} 
	${SIZE} $@

${${DIR}_MOD_OBJS} : ${BLD_DIR}/${DIR}/%.mod.o : ${DIR}/%.c
	mkdir -p ${@D}
	${CC} ${${DIR_FROM_MOD_OBJ}_MOD_CFLAGS}  -c $< -o $@

${BLD_DIR}/${DIR}/clean::
	rm -rf ${${patsubst ${BLD_DIR}/%/clean,%,$@}_MOD_DISCARDS}

undefine LOAD_ADDR