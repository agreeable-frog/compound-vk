cmake_minimum_required(VERSION 3.28)

project(compound VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "build type" FORCE)
endif()

message(STATUS "Build type is : ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "hello")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=address")
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(log4cplus REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(IMGUI REQUIRED imgui)

add_library(${PROJECT_NAME} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/init.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src/device.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src/swapchain.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src/framebuffer.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src/commandstructs.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src/renderloop.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan glfw log4cplus::log4cplus)

add_subdirectory(test)