#
# mesa
#

add_library(mesa STATIC)

target_sources(
	mesa
	PUBLIC
        Constant.h
        Function.h
        interrupt_thread.h
        keymap.h
        memory.h
        MesaBasic.h
        guam.h
        Pilot.h
        processor_thread.h
        Setting.h
        timer_thread.h
        Type.h
        Variable.h
    PRIVATE
        interrupt_thread.cpp
        keymap.cpp
        memory.cpp
        guam.cpp
        Pilot.cpp
        processor_thread.cpp
        Setting.cpp
        timer_thread.cpp
        Type.cpp
        Variable.cpp
)

add_dependencies(mesa util)
