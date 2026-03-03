cmake_minimum_required(VERSION 3.15)
project(Tessera LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)

# Use modern C23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Inlcude from Homebrew
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")

# Find the required system libraries
find_package(OpenGL REQUIRED)
find_package(Curses REQUIRED)

# Build GLFW
if(NOT TARGET glfw)
    message(STATUS "GLFW not found, building from '/include'")
    add_subdirectory("include/glfw/")
endif()

# Define shader source and .h file
set(SHADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/patterns/hadamard.frag")
set(SHADER_HEADER "${CMAKE_CURRENT_BINARY_DIR}/hadamard.frag.h")

# Convert shader using xxd command
add_custom_command(
    OUTPUT "${SHADER_HEADER}"

    COMMAND xxd -i hadamard.frag > ${SHADER_HEADER}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/patterns"

    # Re-run this command if the shader file changes
    DEPENDS ${SHADER_FILE}
    COMMENT "Generating hadamard.frag.h from hadamard.frag..."
    VERBATIM
)

# Add source files
if (APPLE)
add_executable(tessera MACOSX_BUNDLE
    src/app.c 
    src/ui/ui.c 
    src/ui/dropdown.c 
    src/patterns/patterns.c 
    ${SHADER_HEADER}
    src/patterns/glad.c 
    src/sample/sample.c 
    src/reconstruction/reconstruction.c
)

set_target_properties(tessera PROPERTIES
    MACOSX_BUNDLE True
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
)
else()
add_executable(tessera
    src/app.c 
    src/ui/ui.c 
    src/ui/dropdown.c 
    src/patterns/patterns.c 
    ${SHADER_HEADER}
    src/patterns/glad.c 
    src/sample/sample.c 
    src/reconstruction/reconstruction.c
)
endif()

# Add include directories
target_include_directories(tessera PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CURSES_INCLUDE_DIRS}
)

# Link the graphics libraries
target_link_libraries(tessera PRIVATE 
    glfw 
    OpenGL::GL
    ${CURSES_LIBRARIES}
)