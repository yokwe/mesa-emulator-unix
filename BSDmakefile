#
# BSDmakefile
#


.if exists(/usr/bin/bash)
SHELL := /usr/bin/bash
.export-env SHELL
.endif

.if exists(/usr/local/bin/bash)
SHELL := /usr/local/bin/bash
.export-env SHELL
.endif

LOG4CPP_CONFIG    := data/debug.properties
.export-env LOG4CPP_CONFIG

include Makefile
