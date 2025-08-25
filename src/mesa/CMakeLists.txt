#
# mesa
#

add_library(mesa STATIC)

target_sources(
	mesa
	PUBLIC
        Constant.h
        Function.h
        InterruptThread.h
        Memory.h
        MesaBasic.h
        MesaProcessor.h
        Pilot.h
        ProcessorThread.h
        TimerThread.h
        Type.h
        Variable.h
    PRIVATE
        InterruptThread.cpp
        Memory.cpp
        MesaProcessor.cpp
        Pilot.cpp
        ProcessorThread.cpp
        TimerThread.cpp
        Type.cpp
        Variable.cpp
)

add_dependencies(mesa util)
