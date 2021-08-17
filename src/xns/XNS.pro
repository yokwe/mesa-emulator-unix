TARGET   = xns

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += XNS.h   Config.h   Time.h
SOURCES += XNS.cpp Config.cpp Time.cpp


###############################################
include(../common.pro)
