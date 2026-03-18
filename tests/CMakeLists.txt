include(CTest)

set(SHADER_HEADER "${CMAKE_BINARY_DIR}/hadamard.frag.h")
set_source_files_properties(${SHADER_HEADER} PROPERTIES GENERATED TRUE)

# Tests
function(add_test_executable name)
    set(TEST_NAME "test-${name}")

    # The test executable ONLY needs its own source file
    add_executable("${TEST_NAME}" "src/${name}.c" "src/tests.h")

    # Link the core application code to the test!
    # Because we used PUBLIC in the root, this automatically links GLFW, OpenGL, and Curses too.
    target_link_libraries("${TEST_NAME}" PRIVATE tessera_core)

    # Ensure the shader is generated before tests compile
    add_dependencies("${TEST_NAME}" generate_shader_headers)

    # Set the test executable name
    string(REPLACE "_" "/" TEST_EXECUTABLE_NAME ${name})
    add_test(NAME ${TEST_EXECUTABLE_NAME} COMMAND "${TEST_NAME}")

    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options("${TEST_NAME}" PRIVATE --coverage -w)
        target_link_options("${TEST_NAME}" PRIVATE --coverage)
    endif()

    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests/
    )
endfunction()

# Find Test Files
file(GLOB TESTS "src/*.c")

foreach(TEST_FILE ${TESTS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_test_executable(${TEST_NAME})
endforeach()