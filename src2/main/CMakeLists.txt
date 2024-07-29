#
# main
#

add_executable(main)

target_sources(
	main
	PRIVATE
		main.cpp
)

add_dependencies(main opcode mesa util)

target_link_libraries (main opcode mesa util)
