TARGET   = courierImpl

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Service.h   CHService.h
SOURCES += Service.cpp CHService.cpp


###############################################
include(../common.pro)
