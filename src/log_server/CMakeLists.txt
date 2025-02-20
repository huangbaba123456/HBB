# 自动搜索当前目录下的所有 .cpp 文件
file(GLOB log_server_sources *.cpp)

# 添加可执行文件
add_executable(log_server ${log_server_sources})

# 链接 xplatform、LXMysql 静态库和 libprotobuf 静态库
target_link_libraries(log_server PRIVATE
    basicBox
    LXMysql
    protobuf
)

# 设置头文件搜索路径
target_include_directories(log_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # log_server 自己的头文件路径
    ${CMAKE_SOURCE_DIR}/src/basicBox 
    ${CMAKE_SOURCE_DIR}/src/LXMysql   # LXMysql 的头文件路径
)

# 设置输出路径
set_target_properties(log_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin
)