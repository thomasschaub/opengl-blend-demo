cmake_minimum_required(VERSION 3.5)

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(opengl-blend-demo main.cpp)
target_link_libraries(opengl-blend-demo PRIVATE GL GLEW SDL2)
target_compile_features(opengl-blend-demo PRIVATE cxx_auto_type)
