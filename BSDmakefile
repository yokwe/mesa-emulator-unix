#
# BSDmakefile
#


.if exists(/usr/bin/bash)
SHELL := /usr/bin/bash
.endif

.if exists(/usr/local/bin/bash)
SHELL := /usr/local/bin/bash
.endif

LOG4CPP_CONFIG    := data/debug.properties
.export-env LOG4CPP_CONFIG

include Makefile
