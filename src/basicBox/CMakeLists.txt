# 自动搜索当前目录下的所有 .cpp 文件
file(GLOB basic_sources *.cpp *.cc)
# 加入而外的.cc文件
# 添加静态库
add_library(basicBox STATIC ${basic_sources})

# 设置 BASIC 的头文件路径（供其他模块使用）
target_include_directories(basicBox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 链接系统库
target_link_libraries(basicBox PUBLIC
    pthread         # 多线程库
    event           # Libevent
    ssl             # OpenSSL
    crypto          # OpenSSL 加密库
    protobuf        # Protocol Buffers
    event_openssl   # Libevent 的 OpenSSL 支持库
)

# 设置输出路径
set_target_properties(basicBox PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib
)