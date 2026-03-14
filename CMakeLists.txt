cmake_minimum_required(VERSION 3.15)
project(Tessera VERSION 1.0.0 LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)

# Use modern C23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Inlcude from Homebrew
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")

# Find the required system libraries
find_package(OpenGL REQUIRED)
find_package(Curses REQUIRED)
find_package(glfw3 QUIET)

# If glfw wasn't found
if(NOT glfw3_FOUND AND NOT TARGET glfw)
    message(STATUS "GLFW system package not found, building from 'include/glfw/'")
    
    # Prevent GLFW from installing its own files to the system during 'make install'
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    # Add the subdirectory
    add_subdirectory("include/glfw")
    
    # Create an alias to glfw
    if(TARGET glfw)
        add_library(glfw_lib INTERFACE)
        target_link_libraries(glfw_lib INTERFACE glfw)
    endif()
endif()

# Define shader source and .h file
set(HADAMARD_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/patterns/hadamard.frag")
set(HADAMARD_HEADER "${CMAKE_CURRENT_BINARY_DIR}/hadamard.frag.h")
set(WAVELET_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/patterns/wavelet.frag")
set(WAVELET_HEADER "${CMAKE_CURRENT_BINARY_DIR}/wavelet.frag.h")

# Convert shader using xxd command
add_custom_command(
    OUTPUT ${HADAMARD_HEADER} ${WAVELET_HEADER}

    COMMAND xxd -i hadamard.frag > ${HADAMARD_HEADER}
    COMMAND xxd -i wavelet.frag > ${WAVELET_HEADER}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/patterns"

    # Re-run this command if the shader file changes
    DEPENDS ${HADAMARD_FILE} ${WAVELET_FILE}
    COMMENT "Generating hadamard.frag.h from hadamard.frag..."
    VERBATIM
)

# Add source files
if (APPLE)
    # Put app icon in /resources/
    set(MACOS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/Icon.icns")
    set_source_files_properties(${MACOS_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )

    # Make excecutable as a MacOS bundle
    add_executable(tessera MACOSX_BUNDLE
        src/app.c 
        src/ui/ui.c 
        src/ui/dropdown.c 
        src/patterns/patterns.c 
        ${HADAMARD_HEADER}
        ${WAVELET_HEADER}
        src/patterns/glad.c 
        src/sample/sample.c 
        src/reconstruction/reconstruction.c
        ${MACOS_ICON}
    )

    # Configure bundle
    set_target_properties(tessera PROPERTIES
        MACOSX_BUNDLE True
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOS/Info.plist
    )

    # Load app start script
    set(SCRIPT_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/start.sh")

    # Copy the script into the bundle's MacOS directory after the build
    add_custom_command(
        COMMENT "Copying the shell script into the app bundle's MacOS directory"
        TARGET tessera POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${SCRIPT_SOURCE_PATH}"
            $<TARGET_BUNDLE_CONTENT_DIR:tessera>/MacOS/
        COMMAND chmod +x "$<TARGET_BUNDLE_CONTENT_DIR:tessera>/MacOS/start.sh"
    )

    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_BUNDLE_ICON ${MACOS_ICON})
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-v${PROJECT_VERSION}-macOS")
else()
    add_executable(tessera
        src/app.c 
        src/ui/ui.c 
        src/ui/dropdown.c 
        src/patterns/patterns.c 
        ${HADAMARD_HEADER}
        ${WAVELET_HEADER}
        src/patterns/glad.c 
        src/sample/sample.c 
        src/reconstruction/reconstruction.c
    )
endif()

# Testing
# option(TESTING "Build tests for ${PROJECT_NAME}" ON)
if (TESTING)
    message("-- Building tests")
    enable_testing()
    add_subdirectory(tests)
endif()

# Add include directories
target_include_directories(tessera PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CURSES_INCLUDE_DIRS}
)

# Link the graphics libraries
target_link_libraries(tessera PRIVATE 
    m
    glfw 
    OpenGL::GL
    ${CURSES_LIBRARIES}
)

install(TARGETS tessera
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

include(CPack)