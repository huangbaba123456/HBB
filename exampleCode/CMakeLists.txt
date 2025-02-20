cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(testProject)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)  # 可执行文件输出路径
add_subdirectory(./my_client)
add_subdirectory(./my_service)
