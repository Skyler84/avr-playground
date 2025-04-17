# Requires a few input variables
#   SUBDIRS -> sub directories to include

include ${patsubst %,${DIR}/%/Makefile,${SUBDIRS}}