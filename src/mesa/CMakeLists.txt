#
# mesa
#

list(APPEND fileList Memory.cpp MesaProcessor.cpp Pilot.cpp Type.cpp InterruptThread.cpp ProcessorThread.cpp TimerThread.cpp)

add_library (mesa STATIC ${fileList})
