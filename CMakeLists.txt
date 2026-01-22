cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(playground)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.cpp)

# Opcjonalnie: Ustawienie ikony i wersji
set_target_properties(playground PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourname.openglapp"
    MACOSX_BUNDLE_BUNDLE_NAME "playground"
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
add_subdirectory(glfw)
include_directories(${PROJECT_SOURCE_DIR})
include_directories("glfw/deps") # for glad
include_directories("glfw/include")
include_directories("thirdparty")

if (MSVC)
    add_definitions( "-D _CRT_SECURE_NO_WARNINGS" )
endif()
#if (UNIX)
#    add_definitions( "-std=c99" )
#endif()
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
