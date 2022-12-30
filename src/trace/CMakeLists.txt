#
# trace
#

list(APPEND
  fileList
  Trace.cpp Module.cpp
  )

add_library (trace STATIC ${fileList})
