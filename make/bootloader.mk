# Requires a few input variables
#   SRCS -> C source files
#   DIR  -> directory of source
#   TARGET -> name of target to build
#   LOAD_ADDR -> hexadecimal address to load bootloader


ifndef LOAD_ADDR
$(error LOAD_ADDR not defined for module target ${TARGET} in ${DIR})
endif

${DIR}_BOOT_DISCARDS := ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.boot.%, elf hex bin} ${SRCS:%.c=${BLD_DIR}/${DIR}/%.boot.o}
${DIR}_BOOT_CSRC := ${SRCS:%.c=${DIR}/%.c}
${DIR}_BOOT_OBJS := ${SRCS:%.c=${BLD_DIR}/${DIR}/%.boot.o}
${DIR}/${TARGET}_BOOT_LOAD_ADDR := ${LOAD_ADDR}

DIR_FROM_BOOT_OBJ = ${patsubst %,%/$*.boot.o,%$@}

include ${MAKEDIR}/internal/targets.mk

${BLD_DIR}/${DIR}/all:: ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.boot.%, elf hex bin}

${BLD_DIR}/${DIR}/${TARGET}.boot.bin : ${BLD_DIR}/%.boot.bin : ${BLD_DIR}/${DIR}/${TARGET}.boot.elf
	${OBJCOPY} -O binary $< $@

${BLD_DIR}/${DIR}/${TARGET}.boot.hex : ${BLD_DIR}/%.boot.hex : ${BLD_DIR}/${DIR}/${TARGET}.boot.elf
	${OBJCOPY} -O ihex $< $@

${BLD_DIR}/${DIR}/${TARGET}.boot.elf : ${BLD_DIR}/%/${TARGET}.boot.elf : ${${DIR}_BOOT_OBJS}
	${CC} ${LDFLAGS} ${$*_LDFLAGS} $^ -o $@ 
	${SIZE} $@

${${DIR}_BOOT_OBJS} : ${BLD_DIR}/${DIR}/%.boot.o : ${DIR}/%.c
	mkdir -p ${@D}
	${CC} ${CFLAGS} ${${DIR_FROM_BOOT_OBJ}_CFLAGS}  -c $< -o $@

${BLD_DIR}/${DIR}/clean::
	rm -f ${${patsubst ${BLD_DIR}/%/clean,%,$@}_BOOT_DISCARDS}

undefine LOAD_ADDR