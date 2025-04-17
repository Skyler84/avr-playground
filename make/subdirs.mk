# Requires a few input variables
#   DIR -> current directory relative to ROOT
#   SUBDIRS -> sub directories to include

${DIR}/:: ${DIR}/all
${DIR}/all:: ${SUBDIRS:%=${DIR}/%/all}
${DIR}/clean:: ${SUBDIRS:%=${DIR}/%/clean}
${SUBDIRS:%=${DIR}/%/all}::
${SUBDIRS:%=${DIR}/%/clean}::

include ${patsubst %,${DIR}/%/Makefile,${SUBDIRS}}