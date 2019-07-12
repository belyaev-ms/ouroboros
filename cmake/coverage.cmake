find_program(LCOV_PATH lcov)

set(CMAKE_C_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "-coverage -fprofile-arcs -ftest-coverage")

function(ouroboros_coverage coverage_name)
    if (NOT LCOV_PATH)
        message(FATAL_ERROR "lcov not found!")
    endif (NOT LCOV_PATH)

    add_custom_target(${coverage_name}
        # capturing lcov counters and generating report
        COMMAND ${LCOV_PATH} --directory . --capture --output-file ${coverage_name}.info
        COMMAND ${LCOV_PATH} --remove ${coverage_name}.info 'tests/*' 'ouroboros/*' '/usr/*' --output-file ${coverage_name}.info
        COMMAND ${LCOV_PATH} --list ${coverage_name}.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Processing code coverage counters and generating report."
    )
endfunction(ouroboros_coverage)
