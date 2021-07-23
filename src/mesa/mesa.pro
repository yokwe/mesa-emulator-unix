TARGET   = mesa

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Constant.h Function.h MesaBasic.h Variable.h

HEADERS += Memory.h   MesaProcessor.h   MesaThread.h   Pilot.h   Type.h
SOURCES += Memory.cpp MesaProcessor.cpp MesaThread.cpp Pilot.cpp Type.cpp


###############################################
include(../common.pro)
