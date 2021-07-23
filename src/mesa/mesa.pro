TARGET   = mesa

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Constant.h Function.h MesaBasic.h Variable.h

HEADERS += Memory.h   MesaProcessor.h   Pilot.h   Type.h
SOURCES += Memory.cpp MesaProcessor.cpp Pilot.cpp Type.cpp

HEADERS += InterruptThread.h   ProcessorThread.h   TimerThread.h
SOURCES += InterruptThread.cpp ProcessorThread.cpp TimerThread.cpp


###############################################
include(../common.pro)
