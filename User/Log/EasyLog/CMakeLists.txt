project(EasyLog C CXX)
file(GLOB_RECURSE SOURCES "*.c" "*.cc" "*.cpp")
add_library(EasyLog STATIC ${SOURCES})
target_include_directories(EasyLog PUBLIC inc)

