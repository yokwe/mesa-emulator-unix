TARGET   = xns

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += XNS.h   Config.h   Time.h   Courier.h
SOURCES += XNS.cpp Config.cpp Time.cpp Courier.cpp


###############################################
include(../common.pro)
