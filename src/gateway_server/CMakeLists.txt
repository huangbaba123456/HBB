# 自动搜索当前目录下的所有 .cpp 文件
file(GLOB gateway_sources *.cpp)

# 添加可执行文件
add_executable(gateway_server ${gateway_sources})

# 链接 xplatform 静态库和 libprotobuf 静态库
target_link_libraries(gateway_server PRIVATE
    basicBox
    protobuf
)

# 设置头文件搜索路径
target_include_directories(gateway_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # gateway_server 自己的头文件路径
    ${CMAKE_SOURCE_DIR}/src/basicBox  # xplatform 的头文件路径
)

# 设置输出路径
set_target_properties(gateway_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin
)