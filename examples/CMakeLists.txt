################################################################################
# The examples of using the ouroboros
################################################################################
include_directories(..)
add_executable(sample sample.cpp)
target_link_libraries(sample ouroboros)
add_executable(writer writer.cpp)
target_link_libraries(writer ouroboros)
add_executable(reader reader.cpp)
target_link_libraries(reader ouroboros)