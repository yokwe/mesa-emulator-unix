#
# mesa
#

add_library(mesa STATIC)

target_sources(
	mesa
	PUBLIC
        Constant.h
        display.h
        Function.h
        interrupt_thread.h
        keymap.h
        memory.h
        MesaBasic.h
        guam.h
        Pilot.h
        processor_thread.h
        setting.h
        timer_thread.h
        Type.h
        Variable.h
    PRIVATE
        display.cpp
        interrupt_thread.cpp
        keymap.cpp
        memory.cpp
        guam.cpp
        Pilot.cpp
        processor_thread.cpp
        setting.cpp
        timer_thread.cpp
        Type.cpp
        Variable.cpp
)

add_dependencies(mesa util)
