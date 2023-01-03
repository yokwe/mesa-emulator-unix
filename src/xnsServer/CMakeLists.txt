#
# xnsServer
#

add_executable (
  xnsServer
  main.cpp
  Listener.cpp PEXListener.cpp Server.cpp SPPListener.cpp SPPQueue.cpp SPPServer.cpp
  SPPPacket.cpp
  )

add_dependencies(xnsServer util xns courier courierImpl xnsServerImpl)

target_link_libraries (xnsServer util xns courier courierImpl xnsServerImpl)
