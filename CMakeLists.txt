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
find_package(glfw3 CONFIG REQUIRED)
find_package(Curses REQUIRED)

# Add source files
add_executable(tessera src/app.c src/ui/ui.c src/ui/dropdown.c src/patterns/patterns.c src/patterns/glad.c src/sample/sample.c src/reconstruction/reconstruction.c)

# Add include directories
target_include_directories(tessera PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CURSES_INCLUDE_DIRS}
)

# Link the graphics libraries
target_link_libraries(tessera PRIVATE 
    glfw 
    OpenGL::GL
    ${CURSES_LIBRARIES}
)