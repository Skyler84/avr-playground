# Requires a few input variables
#   SRCS -> C source files
#   DIR  -> directory of source
#   TARGET -> name of target to build

${DIR}_APP_DISCARDS := ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.%, elf hex bin} ${SRCS:%.c=${BLD_DIR}/${DIR}/%.o}
${DIR}_APP_CSRC := ${SRCS:%.c=${DIR}/%.c}
${DIR}_APP_OBJS := ${SRCS:%.c=${BLD_DIR}/${DIR}/%.o}

DIR_FROM_APP_OBJ = ${patsubst ${BLD_DIR}/%/$*.o,%,$@}

include ${MAKEDIR}/internal/targets.mk
${DIR}_APP_CFLAGS :=   ${CFLAGS}  ${${DIR}_CFLAGS}
${DIR}_APP_LDFLAGS :=  ${LDFLAGS} ${${DIR}_LDFLAGS} 

${BLD_DIR}/${DIR}/all:: ${BLD_DIR}/%/all : ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.%, elf hex bin}

${BLD_DIR}/${DIR}/${TARGET}.bin : ${BLD_DIR}/%.bin : ${BLD_DIR}/${DIR}/${TARGET}.elf
	${OBJCOPY} -O binary $< $@

${BLD_DIR}/${DIR}/${TARGET}.hex : ${BLD_DIR}/%.hex : ${BLD_DIR}/${DIR}/${TARGET}.elf
	${OBJCOPY} -O ihex $< $@

${BLD_DIR}/${DIR}/${TARGET}.elf :: ${BLD_DIR}/%/${TARGET}.elf : ${${DIR}_APP_OBJS}
	${CC} $^ -o $@ ${LDFLAGS} ${$*_APP_LDFLAGS} 
	${SIZE} $@

${${DIR}_APP_OBJS} : ${BLD_DIR}/${DIR}/%.o : ${DIR}/%.c
	mkdir -p ${@D}
	@echo ${DIR_FROM_APP_OBJ}_APP_CFLAGS
	@echo ${${DIR_FROM_APP_OBJ}_APP_CFLAGS}
	${CC} ${CFLAGS} ${${DIR_FROM_APP_OBJ}_APP_CFLAGS}  -c $< -o $@

${BLD_DIR}/${DIR}/clean::
	rm -f ${${patsubst ${BLD_DIR}/%/clean,%,$@}_APP_DISCARDS}

