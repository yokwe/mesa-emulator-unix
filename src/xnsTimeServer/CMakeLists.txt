#
# xnsTimeServer
#

add_executable(
  xnsTimeServer
  )

target_sources(
	xnsTimeServer
	PUBLIC
    Server.h
  PRIVATE
    ECHO.cpp
    IDP.cpp
    main.cpp
    PEX.cpp
    PEX_TIME.cpp
    RIP.cpp
    SPP.cpp
)

add_dependencies(xnsTimeServer xns util)

target_link_libraries (xnsTimeServer xns util)
