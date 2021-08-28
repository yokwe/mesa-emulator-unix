TARGET   = courier

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Courier.h   Type.h
SOURCES += Courier.cpp Type.cpp


###############################################
include(../common.pro)
