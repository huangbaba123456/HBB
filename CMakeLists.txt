cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(MyProject)

# 设置输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 静态库输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 动态库输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # 可执行文件输出路径

# 添加子目录
add_subdirectory(src/basicBox)
add_subdirectory(src/LXMysql)
add_subdirectory(src/register_server)
add_subdirectory(src/log_server)
add_subdirectory(src/gateway_server)