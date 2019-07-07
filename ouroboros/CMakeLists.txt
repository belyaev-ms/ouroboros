################################################################################
# Library the Ouroboros
################################################################################
include_directories(..)
add_library(ouroboros STATIC file.cpp memory.cpp memoryfile.cpp transaction.cpp)
target_link_libraries(ouroboros ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} pthread)
if (NOT APPLE)
target_link_libraries(ouroboros rt)
endif (NOT APPLE)