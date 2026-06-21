# tests/CMakeLists.txt

cmake_minimum_required(VERSION 3.22)

# Unity test framework
add_library(unity STATIC
    ${CMAKE_SOURCE_DIR}/unity/unity.c
)
target_include_directories(unity PUBLIC
    ${CMAKE_SOURCE_DIR}/unity
)

# Helper function to register a test executable
function(add_unit_test test_name test_source)
    add_executable(${test_name}
        ${test_source}
        ${CMAKE_SOURCE_DIR}/Core/Src/vehicle_dynamics.c
    )
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/Core/Inc
        ${CMAKE_SOURCE_DIR}/unity
    )
    target_link_libraries(${test_name} PRIVATE unity)

    # Register with CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Register test executables
add_unit_test(test_vehicle_dynamics
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_vehicle_dynamics.c
)