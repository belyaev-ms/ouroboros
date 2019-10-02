################################################################################
# Coverage
################################################################################
set(OUROBOROS_COVERAGE_ENABLED yes)
if (OUROBOROS_COVERAGE_ENABLED)
    include(../cmake/coverage.cmake)
else (OUROBOROS_COVERAGE_ENABLED)
    set(CMAKE_CXX_FLAGS "-g3")
endif (OUROBOROS_COVERAGE_ENABLED)

################################################################################
# Unit-tests
################################################################################
add_definitions(-DOUROBOROS_TEST_ENABLED)
#macros for add an unit-test
macro(ouroboros_add_test test)
    add_executable(${test} ${test}.cpp)
    target_link_libraries(${test} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ouroboros)
    add_test(${test} ${test})
endmacro(ouroboros_add_test)
include_directories(..)
ouroboros_add_test(error_test)
ouroboros_add_test(page_test)
ouroboros_add_test(cache_test)
ouroboros_add_test(file_test)
ouroboros_add_test(cachefile_test)
ouroboros_add_test(backupfile_test)
ouroboros_add_test(journalfile_test)
ouroboros_add_test(hashmap_test)
ouroboros_add_test(key_test)
ouroboros_add_test(field_test)
ouroboros_add_test(table_test)
ouroboros_add_test(datatable_test)
ouroboros_add_test(fragtable_test)
ouroboros_add_test(indexedtable_test)
ouroboros_add_test(treedatatable_test)
ouroboros_add_test(dataset_test)
ouroboros_add_test(indexeddataset_test)
ouroboros_add_test(treedataset_test)
ouroboros_add_test(find_test)
ouroboros_add_test(transaction_test)
ouroboros_add_test(indexedtransaction_test)
ouroboros_add_test(rbtree_test)
if (OUROBOROS_COVERAGE_ENABLED)
    ouroboros_coverage(coverage)
endif (OUROBOROS_COVERAGE_ENABLED)

################################################################################
# Static code analysis
################################################################################
add_custom_target(
    cppcheck
    COMMAND cppcheck
    --enable=all
    --template=gcc
    --project=${CMAKE_BINARY_DIR}/compile_commands.json
)

add_custom_target(
    clang-tidy
    COMMAND run-clang-tidy-3.8.py
)
