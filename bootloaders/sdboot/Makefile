SRCS := main.c encoder.c gui.c buttons.c
DIR := bootloaders/sdboot
TARGET := sdboot
MODLIBS := lcd fonts module sd blockdev vfs
MODULES := fat
LIBS :=
INTERFACES := fs display
BOOT_START_ADDR := 0x1e000
MODULE_START_ADDR := 0x1c000
LOAD_ADDR := 0x1e000

OPT:=s
# LDFLAGS +=                                        



${DIR}_INCLUDES += \
	${patsubst %,-I${ROOT}/libs/%, 	    ${LIBS}}       \
	${patsubst %,-I${ROOT}/interfaces/%,${INTERFACES}} \
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

${BLD_DIR}/${DIR}/all :: ${BLD_DIR}/${DIR}/${TARGET}.boot.all.hex
${BLD_DIR}/${DIR}/${TARGET}.elf :: ${patsubst %,${BLD_DIR}/modules/%/, ${MODLIBS} ${MODULES}}
${BLD_DIR}/${DIR}/${TARGET}.boot.elf :: ${patsubst %,${BLD_DIR}/modules/%/, ${MODLIBS} ${MODULES}}

# ${BLD_DIR}/${DIR}/${TARGET}.mods.bin :: ${BLD_DIR}/modules/fat/fat.mod.bin 
# 	mkdir -p ${@D}
# 	srec_cat ${patsubst %,% -binary,$<} -o $@ -binary

# ${BLD_DIR}/bootloaders/sdboot/${TARGET}.mods.hex :: ${BLD_DIR}/bootloaders/sdboot/${TARGET}.mods.bin
# 	${OBJCOPY} -I binary -O ihex $< $@ --change-addresses $$((0x1e000-`wc -c $< | cut -d ' ' -f1`))

# ${BLD_DIR}/bootloaders/sdboot/sdboot.all.hex :: ${BLD_DIR}/bootloaders/sdboot/${TARGET}.mods.hex ${BLD_DIR}/bootloaders/sdboot/sdboot.boot.hex
# 	srec_cat ${patsubst %,% -intel,$^} -o $@ -intel

${DIR}/flash_boot :: ${BLD_DIR}/${DIR}/${TARGET}.boot.all.hex
	@echo "Flashing $<"
	avrdude -c c232hm -p usb1286 -U flash:w:$<

${BLD_DIR}/${DIR}/clean :: %/clean :
	rm -f $*/*



ifeq (${BOARD}, lafortuna)

include ${MAKEDIR}/app.mk
include ${MAKEDIR}/bootloader.mk

endif