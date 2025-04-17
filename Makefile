SUBDIRS := apps bootloaders libs modules
ROOT := ${shell pwd}
MAKEDIR := ${ROOT}/make
BLD_DIR := build

ifdef BOARD
	include boards/${BOARD}.mk
endif

include ${patsubst %,%/Makefile,${SUBDIRS}}