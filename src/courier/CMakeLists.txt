#
# courier
#

list(APPEND 
  fileList
  Type.cpp Protocol.cpp DataStream.cpp TextStream.cpp
  Authentication1.cpp Authentication3.cpp BulkData.cpp Clearinghouse2.cpp Clearinghouse3.cpp
  )

add_library (courier STATIC ${fileList})
