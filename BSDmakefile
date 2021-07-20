#
# BSDmakefile
#

# Linux
.if exists(/usr/bin/bash)
SHELL := /usr/bin/bash
.export-env SHELL
.endif

# FreeBSD
.if exists(/usr/local/bin/bash)
SHELL := /usr/local/bin/bash
.export-env SHELL
.endif

# Darwin
.if exists(/bin/bash)
SHELL := /bin/bash
.export-env SHELL
.endif

LOG4CPP_CONFIG    := data/debug.properties
.export-env LOG4CPP_CONFIG

include Makefile
