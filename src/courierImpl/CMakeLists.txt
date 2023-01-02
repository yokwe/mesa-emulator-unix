#
# courierImpl
#

list(APPEND 
  fileList
  CHService.cpp Service.cpp
  )

add_library (courierImpl STATIC ${fileList})
