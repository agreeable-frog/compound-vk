cmake_minimum_required(VERSION 3.28)

add_executable(${PROJECT_NAME}-test ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/shaders/basic.frag.spv
               ${CMAKE_CURRENT_SOURCE_DIR}/shaders/basic.vert.spv)
target_link_libraries(${PROJECT_NAME}-test PUBLIC ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME}-test PUBLIC TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")