# 自动搜索当前目录下的所有 .cpp 和 .cc 文件
file(GLOB register_server_sources *.cpp *.cc)

# 添加可执行文件
add_executable(register_server ${register_server_sources})

# 链接 xplatform 静态库和 protobuf 库
find_package(Protobuf REQUIRED) # 确保找到 protobuf
target_link_libraries(register_server PRIVATE
    basicBox                 # 链接 basicBox 静态库
    ${Protobuf_LIBRARIES}     # 链接 protobuf 静态库
)

# 设置头文件搜索路径
target_include_directories(register_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}      # register_server 自己的头文件路径
    ${CMAKE_SOURCE_DIR}/src/basicBox # basicBox 的头文件路径
    ${Protobuf_INCLUDE_DIRS}         # protobuf 的头文件路径
)

# 设置输出路径（推荐输出到 bin 目录）
set_target_properties(register_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin
)