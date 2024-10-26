project(Log C CXX)
file(GLOB SOURCES "*.c" "*.cc" "*.cpp")
add_library(Log STATIC ${SOURCES})
# Log依赖的一个开源包
add_subdirectory(EasyLog)
target_link_libraries(Log PUBLIC EasyLog)

