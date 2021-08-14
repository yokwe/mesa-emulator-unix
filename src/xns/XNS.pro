TARGET   = xns

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += XNS.h   Config.h
SOURCES += XNS.cpp Config.cpp


###############################################
include(../common.pro)
