cmake_minimum_required(VERSION 3.10.2)
project(ruby)

file(GLOB_RECURSE RUBY_INCLUDE inlude/*.hpp inlude/*.tcc)
file(GLOB_RECURSE RUBY_SOURCES src/*.cpp)

include_directories(include)

add_subdirectory(imgui)

add_executable(ruby ${RUBY_INCLUDE} ${RUBY_SOURCES})
target_link_libraries(ruby imgui)
set_property(TARGET ruby PROPERTY CXX_STANDARD 17)
