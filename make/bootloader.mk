# Requires a few input variables
#   SRCS -> C source files
#   DIR  -> directory of source
#   TARGET -> name of target to build
#   LOAD_ADDR -> hexadecimal address to load bootloader


ifndef BOOT_START_ADDR
$(error BOOT_START_ADDR not defined for module target ${TARGET} in ${DIR})
endif

${DIR}_BOOT_DISCARDS := ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.boot.%, elf hex bin} ${SRCS:%.c=${BLD_DIR}/${DIR}/%.boot.o}
${DIR}_BOOT_CSRC := ${SRCS:%.c=${DIR}/%.c}
${DIR}_BOOT_OBJS := ${SRCS:%.c=${BLD_DIR}/${DIR}/%.boot.o}
${DIR}_BOOT_START_ADDR := ${BOOT_START_ADDR}
${DIR}_BOOT_MODULE_START_ADDR := ${MODULE_START_ADDR}

DIR_FROM_BOOT_OBJ = ${patsubst ${BLD_DIR}/%/$*.boot.o,%,$@}
${DIR}_BOOT_CFLAGS :=  ${CFLAGS} ${${DIR}_CFLAGS}
${DIR}_BOOT_LDFLAGS :=  ${LDFLAGS} ${${DIR}_LDFLAGS} -Wl,-section-start=.text=${BOOT_START_ADDR}
${DIR}_BOOT_MODULES := ${MODULES}

include ${MAKEDIR}/internal/targets.mk

${BLD_DIR}/${DIR}/all:: ${patsubst %,${BLD_DIR}/${DIR}/${TARGET}.boot.%, elf hex bin}

${BLD_DIR}/${DIR}/%.boot.mod.bin : ${BLD_DIR}/modules/%.mod.bin
	@echo $@
	mkdir -p ${@D}
	cp $< $@

${BLD_DIR}/${DIR}/${TARGET}.boot.mods.bin : ${BLD_DIR}/%/${TARGET}.boot.mods.bin : ${patsubst %,${BLD_DIR}/${DIR}/%.boot.mod.bin,${${DIR}_BOOT_MODULES}}
	mkdir -p ${@D}
	srec_cat ${patsubst %,% -binary,$^} -o $@ -binary

${BLD_DIR}/${DIR}/${TARGET}.boot.mods.hex : ${BLD_DIR}/%/${TARGET}.boot.mods.hex : ${BLD_DIR}/%/${TARGET}.boot.mods.bin
	${OBJCOPY} -O ihex -I binary $< $@ --change-addresses ${$*_BOOT_MODULE_START_ADDR}

${BLD_DIR}/${DIR}/${TARGET}.boot.all.hex : ${BLD_DIR}/%/${TARGET}.boot.all.hex : ${BLD_DIR}/%/${TARGET}.boot.mods.hex ${BLD_DIR}/%/${TARGET}.boot.hex
	srec_cat $< -intel -o $@ -intel

${BLD_DIR}/${DIR}/${TARGET}.boot.bin : ${BLD_DIR}/%.boot.bin : ${BLD_DIR}/${DIR}/${TARGET}.boot.elf
	${OBJCOPY} -O binary $< $@

${BLD_DIR}/${DIR}/${TARGET}.boot.hex : ${BLD_DIR}/%.boot.hex : ${BLD_DIR}/${DIR}/${TARGET}.boot.elf
	${OBJCOPY} -O ihex $< $@

${BLD_DIR}/${DIR}/${TARGET}.boot.elf :: ${BLD_DIR}/%/${TARGET}.boot.elf : ${${DIR}_BOOT_OBJS}
	${CC} $^ -o $@ ${$*_BOOT_LDFLAGS}
	${SIZE} $@

${${DIR}_BOOT_OBJS} : ${BLD_DIR}/${DIR}/%.boot.o : ${DIR}/%.c
	mkdir -p ${@D}
	${CC} ${CFLAGS} ${${DIR_FROM_BOOT_OBJ}_BOOT_CFLAGS}  -c $< -o $@

${BLD_DIR}/${DIR}/clean::
	rm -f ${${patsubst ${BLD_DIR}/%/clean,%,$@}_BOOT_DISCARDS}

undefine BOOT_START_ADDR