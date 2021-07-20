#
# GNUmakefile
#


ifneq ("$(wildcard /usr/local/bin/bash)","")
SHELL := /usr/local/bin/bash
endif

ifneq ("$(wildcard /usr/bin/bash)","")
SHELL := /usr/bin/bash
endif

LOG4CPP_CONFIG    := data/debug.properties
export LOG4CPP_CONFIG


include Makefile
