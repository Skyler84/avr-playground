# Requires a few input variables
#   SRCS -> C source files
#   DIR  -> directory of source
#   TARGET -> name of target to build

${DIR}_APP_DISCARDS := ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.%, elf hex bin} ${SRCS:%.c=${BLD_DIR}/${DIR}/%.o}
${DIR}_APP_CSRC := ${SRCS:%.c=${DIR}/%.c}
${DIR}_APP_OBJS := ${SRCS:%.c=${BLD_DIR}/${DIR}/%.o}

DIR_FROM_BOOT_OBJ = ${patsubst %,%/$*.o,%$@}

include ${MAKEDIR}/internal/targets.mk

${BLD_DIR}/${DIR}/all:: ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.%, elf hex bin}

${BLD_DIR}/${DIR}/${TARGET}.bin : ${BLD_DIR}/%.bin : ${BLD_DIR}/${DIR}/${TARGET}.elf
	${OBJCOPY} -O binary $< $@

${BLD_DIR}/${DIR}/${TARGET}.hex : ${BLD_DIR}/%.hex : ${BLD_DIR}/${DIR}/${TARGET}.elf
	${OBJCOPY} -O ihex $< $@

${BLD_DIR}/${DIR}/${TARGET}.elf : ${BLD_DIR}/%/${TARGET}.elf : ${${DIR}_APP_OBJS}
	${CC} ${LDFLAGS} ${$*_LDFLAGS} $^ -o $@ 
	${SIZE} $@

${${DIR}_APP_OBJS} : ${BLD_DIR}/${DIR}/%.o : ${DIR}/%.c
	mkdir -p ${@D}
	${CC} ${CFLAGS} ${${DIR_FROM_BOOT_OBJ}_CFLAGS}  -c $< -o $@

${BLD_DIR}/${DIR}/clean::
	rm -f ${${patsubst ${BLD_DIR}/%/clean,%,$@}_APP_DISCARDS}

undefine LOAD_ADDR