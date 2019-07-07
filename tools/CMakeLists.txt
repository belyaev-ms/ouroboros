################################################################################
# The test tools for checking the ouroboros
################################################################################
add_definitions(-DOUROBOROS_TEST_TOOLS_ENABLED)
include_directories(..)

# The test tool for checking the use of one data set by different process
add_executable(worker_transaction worker_transaction.cpp)
target_link_libraries(worker_transaction ouroboros)

# The test tool for checking the speed of i/o operations
set(CMAKE_CXX_FLAGS "-O3")
add_executable(speed_test
    speed_test.cpp
    ../ouroboros/file.cpp
    ../ouroboros/memory.cpp
    ../ouroboros/transaction.cpp
)
target_link_libraries(speed_test ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} pthread)
if (NOT APPLE)
target_link_libraries(speed_test rt)
endif (NOT APPLE)
