#
# BSDmakefile
#

SHELL != which bash
.export-env SHELL

HOSTNAME != uname -n
.export-env HOSTNAME

HOST_OS != uname -s
.export-env HOST_OS

BUILD_DIR := build
.export-env BUILD_DIR

LOG4CXX_CONFIGURATION := ${BUILD_DIR}/run/log4j-config.xml
export LOG4CXX_CONFIGURATION

# include common part
include Makefile
