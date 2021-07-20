#
# GNUmakefile
#


ifneq ("$(wildcard /usr/local/bin/bash)","")
SHELL := /usr/local/bin/bash
export SHELL
endif

ifneq ("$(wildcard /usr/bin/bash)","")
SHELL := /usr/bin/bash
export SHELL
endif

LOG4CPP_CONFIG    := data/debug.properties
export LOG4CPP_CONFIG


include Makefile
