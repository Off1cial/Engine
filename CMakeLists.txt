cmake_minimum_required(VERSION 3.10)

project(Engine VERSION 1.0 LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL3 REQUIRED sdl3)
pkg_check_modules(SDL3IMAGE REQUIRED sdl3-image)
pkg_check_modules(SDL3TTF REQUIRED sdl3-ttf)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files (C)
file(GLOB_RECURSE SOURCE_FILES
    src/*.c
    src/types/*.c
    src/types/*.cpp
    src/*.cpp
)

#add_compile_options(-Wno-incompatible-pointer-types)

# Imgui sources
set(IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_impl_sdl3.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_impl_opengl3.cpp
)


# Create executable
add_executable(Engine
  ${IMGUI_SRC}
  ${SOURCE_FILES}
  ${EXTRA_SOURCES}
)

# Include directories for target
target_include_directories(Engine PRIVATE
    ${SDL3_INCLUDE_DIRS}
    ${SDL3IMAGE_INCLUDE_DIRS}
    ${SDL3TTF_INCLUDE_DIRS}
    /include
    ${CMAKE_SOURCE_DIR}/include
)

# Link directories (if needed)
link_directories(
    ${SDL3_LIBRARY_DIRS}
    ${SDL3IMAGE_LIBRARY_DIRS}
    ${SDL3TTF_LIBRARY_DIRS}
)

# Link libraries
target_link_libraries(Engine PRIVATE
    ${SDL3_LIBRARIES}
    ${SDL3IMAGE_LIBRARIES}
    ${SDL3TTF_LIBRARIES}

    m
    GL
    stdc++ # Needed for C++ linkage
)

# Force linker to use C++ (because we mix C and C++)
set_target_properties(Engine PROPERTIES
  LINKER_LANGUAGE CXX
  C_STANDARD 11
  C_STANDARD_REQUIRED YES
  C_EXTENSIONS NO
)

# -------------------------
# AddressSanitizer Support
# -------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(Engine PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
        -g
    )
    target_link_options(Engine PRIVATE
        -fsanitize=address
    )
endif()

# Debug info messages
message(STATUS "SDL3 Include Dir: ${SDL3_INCLUDE_DIRS}")
message(STATUS "SDL3 Libraries: ${SDL3_LIBRARIES}")
message(STATUS "SDL3_image Include Dir: ${SDL3IMAGE_INCLUDE_DIRS}")
message(STATUS "SDL3_image Libraries: ${SDL3IMAGE_LIBRARIES}")
message(STATUS "SDL3_ttf Include Dir: ${SDL3TTF_INCLUDE_DIRS}")
message(STATUS "SDL3_ttf Libraries: ${SDL3TTF_LIBRARIES}")

