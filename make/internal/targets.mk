# Requires both BLD_DIR and BIR to be set

ifndef BLD_DIR
${error "BLD_DIR variable undefined"}
endif
ifndef DIR
${error "DIR variable undefined"}
endif

all::                ${BLD_DIR}/${DIR}/all
${DIR}/::            ${BLD_DIR}/${DIR}/all
${DIR}/all::         ${BLD_DIR}/${DIR}/all
${BLD_DIR}/${DIR}/:: ${BLD_DIR}/${DIR}/all


clean:: ${DIR}/clean
${DIR}/clean:: ${BLD_DIR}/${DIR}/clean