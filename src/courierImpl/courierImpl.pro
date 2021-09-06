TARGET   = courierImpl

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Service.h   CHS.h   CHService.h
SOURCES += Service.cpp CHS.cpp CHService.cpp


###############################################
include(../common.pro)
