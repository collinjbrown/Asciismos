# CMakeList.txt : CMake project for Asciismos, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (asciismos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BASE_SRCS
    "src/animation_2D.cpp"
    "src/animation_2D.h"
    "src/check_error.cpp"
    "src/check_error.h"
    "src/component.h"
    "src/particleengine.h"
    "src/ecs.h"
    "src/ecs.cpp"
    "src/entity.h"
    "src/game.cpp"
    "src/game.h"
    "src/main.cpp"
    "src/main.h"
    "src/renderer.cpp"
    "src/renderer.h"
    "src/shader.cpp"
    "src/shader.h"
    "src/external/stb_image.cpp"
    "src/external/stb_image.h"
    "src/system.h"
    "src/texture_2D.cpp"
    "src/texture_2D.h"
    )

# Add source to this project's executable.
add_executable (asciismos ${BASE_SRCS})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy-assets.cmake
)

add_dependencies(asciismos copy_assets)

add_subdirectory(libs/glfw-3.3.4)
add_subdirectory(libs/glad)
add_subdirectory(libs/glm-0.9.9.8)

if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
     set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

target_link_libraries(asciismos glfw glad glm)
