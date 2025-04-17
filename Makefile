SUBDIRS := modules
ROOT := ${shell pwd}
MAKEDIR := ${ROOT}/make
BLD_DIR := build

CC := avr-gcc
SIZE := avr-size
OBJCOPY := avr-objcopy

ifdef BOARD
include boards/${BOARD}.mk
endif

ifndef MCU
${error "Missing MCU definition. Define BOARD or specify MCU manually on the command line."}
endif
ifndef F_CPU
${error "Missing F_CPU definition. Define BOARD or specify F_CPU manually on the command line."}
endif

include ${patsubst %,%/Makefile,${SUBDIRS}}