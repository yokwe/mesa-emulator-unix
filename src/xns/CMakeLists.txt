#
# xns
#

list(APPEND
  fileList
  XNS.cpp Config.cpp
  RIP.cpp Error.cpp Echo.cpp SPP.cpp PEX.cpp Boot.cpp
  Time.cpp
  )

add_library (xns STATIC ${fileList})
