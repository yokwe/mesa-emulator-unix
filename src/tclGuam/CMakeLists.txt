#
# tclGuam
#

add_executable(tclGuam)

target_sources(
	tclGuam
	PUBLIC
		guam.h
	PRIVATE
		guam.cpp
		main.cpp
)

target_link_libraries(tclGuam util tcl log4cxx)
