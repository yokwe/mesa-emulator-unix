#
# GNUmakefile
#

# Linux
ifneq ("$(wildcard /usr/bin/bash)","")
SHELL := /usr/bin/bash
export SHELL
endif

# FreeBSD
ifneq ("$(wildcard /usr/local/bin/bash)","")
SHELL := /usr/local/bin/bash
export SHELL
endif

# Darwin
ifneq ("$(wildcard /bin/bash)","")
SHELL := /usr/bash
export SHELL
endif


LOG4CPP_CONFIG    := data/debug.properties
export LOG4CPP_CONFIG


include Makefile
