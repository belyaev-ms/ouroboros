################################################################################
# The examples of using the ouroboros
################################################################################
include_directories(..)
add_executable(sample sample.cpp)
target_link_libraries(sample ouroboros)