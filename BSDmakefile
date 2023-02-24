#
# BSDmakefile
#

SHELL != which bash
.export-env SHELL

HOSTNAME != uname -n
.export-env HOSTNAME

HOST_OS != uname -s
.export-env HOST_OS

BUILD_DIR := tmp/cmake/${HOSTNAME}
.export-env BUILD_DIR

LOG_CONFIG := ${BUILD_DIR}/run/debug.properties
.export-env LOG_CONFIG

# include common part
include Makefile
