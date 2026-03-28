cmake_minimum_required(VERSION 3.28.0)
project(dndemask VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

include_directories(src)
add_executable(dndemask src/main.cpp src/show.cpp src/editor.cpp src/maskfile.cpp)
target_link_libraries(dndemask Qt6::Widgets)
