#
# BSDmakefile
#


UNAME != uname
.export-env UNAME

.if exists(/opt/local/libexec/qt5/bin/qmake)
QMAKE := /opt/local/libexec/qt5/bin/qmake
.else
QMAKE != which qmake
.endif
.export-env QMAKE

SHELL != which bash
.export-env SHELL

LOG4CPP_CONFIG    := data/debug.properties
.export-env LOG4CPP_CONFIG


include Makefile
