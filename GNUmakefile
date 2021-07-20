#
# GNUmakefile
#

ifneq ("$(wildcard /opt/local/libexec/qt5/bin/qmake)","")
QMAKE := /opt/local/libexec/qt5/bin/qmake
else
QMAKE := $(shell which qmake)
endif
export QMAKE

SHELL := $(shell which bash)
export SHELL

LOG4CPP_CONFIG    := data/debug.properties
export LOG4CPP_CONFIG


include Makefile
