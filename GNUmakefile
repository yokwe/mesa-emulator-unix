#
# GNUmakefile
#

SHELL := $(shell which bash)
export SHELL

HOSTNAME := $(shell uname -n)
export HOSTNAME

BUILD_DIR := tmp/cmake/${HOSTNAME}
export BUILD_DIR

LOG_CONFIG := ${BUILD_DIR}/run/debug.properties
export LOG_CONFIG

# include common part
include Makefile
