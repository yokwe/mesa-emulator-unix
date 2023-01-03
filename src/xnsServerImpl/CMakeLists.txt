#
# xnsServer
#

list(APPEND
  fileList
  CHSListener.cpp EchoListener.cpp RIPListener.cpp SPPCourier.cpp TimeListener.cpp
  )

add_library (xnsServerImpl STATIC ${fileList})
