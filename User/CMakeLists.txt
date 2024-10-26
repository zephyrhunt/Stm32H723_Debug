project(User C CXX)
file(GLOB SOURCES "*.c" "*.cc" "*.cpp")
include_directories(Log)
add_library(User STATIC ${SOURCES})

# 添加需要链接的库
add_subdirectory(Log)
target_link_libraries(User Log) # User链接Log库
