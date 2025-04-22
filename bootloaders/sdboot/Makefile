SRCS := main.c encoder.c
DIR := bootloaders/sdboot
TARGET := sdboot
MODLIBS := lcd fonts module sd blockdev vfs
MODULES := fat
LIBS :=
LOAD_ADDR := 0x1e000

OPT:=s
# LDFLAGS +=                                        

${DIR}_INCLUDES += \
	${patsubst %,-I${ROOT}/libs/%, 	    ${LIBS}}       \
	${patsubst %,-I${ROOT}/modules/%, 	${MODLIBS}} \
	${patsubst %,-I${ROOT}/modules/%, 	${MODULES}} \
	-I${ROOT}/module
${DIR}_CFLAGS += ${${DIR}_INCLUDES}
${DIR}_LDFLAGS += 																			\
	${patsubst %,-L${ROOT}/${BLD_DIR}/libs/%, 	      ${LIBS}}     \
	${patsubst %,-l%, 	                  ${LIBS}}                  \
	${patsubst %,-L${ROOT}/${BLD_DIR}/modules/%, 	  ${MODLIBS}}     \
	${patsubst %,-l%, 	                  ${MODLIBS}}                  \
	-L${ROOT}/module                                \
	-lmodule

${BLD_DIR}/${DIR}/all :: ${BLD_DIR}/${DIR}/${TARGET}.all.hex
${BLD_DIR}/${DIR}/${TARGET}.elf :: ${patsubst %,${BLD_DIR}/modules/%/, ${MODLIBS} ${MODULES}}
${BLD_DIR}/${DIR}/${TARGET}.boot.elf :: ${patsubst %,${BLD_DIR}/modules/%/, ${MODLIBS} ${MODULES}}

${BLD_DIR}/${DIR}/${TARGET}.mods.bin :: ${BLD_DIR}/modules/fat/fat.mod.bin 
	${OBJCOPY} -I ihex -O ihex ${BLD_DIR}/modules/fat/fat.mod.elf ${BLD_DIR}/bootloaders/sdboot/fat.mod.hex --change-addresses 0xe000
	srec_cat ${patsubst %,% -binary,$^} -o $@ -binary

${BLD_DIR}/bootloaders/sdboot/${TARGET}.mods.hex :: ${BLD_DIR}/bootloaders/sdboot/${TARGET}.mods.bin
	${OBJCOPY} -I binary -O ihex $< $@ --change-addresses 0xe000

${BLD_DIR}/bootloaders/sdboot/sdboot.all.hex :: ${BLD_DIR}/bootloaders/sdboot/${TARGET}.mods.hex ${BLD_DIR}/bootloaders/sdboot/sdboot.boot.hex
	srec_cat ${patsubst %,% -intel,$^} -o $@ -intel

${DIR}/flash_boot :: ${BLD_DIR}/${DIR}/${TARGET}.all.hex
	@echo "Flashing $<"
	avrdude -c c232hm -p usb1286 -U flash:w:$<

${BLD_DIR}/${DIR}/clean :: %/clean :
	rm -f $*/fat.mod.hex $*/sdboot.all.hex

include ${MAKEDIR}/app.mk
include ${MAKEDIR}/bootloader.mk