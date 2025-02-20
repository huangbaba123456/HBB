# 自动搜索当前目录下的所有 .cpp 文件
file(GLOB service_source *.cpp *.cc)

# 添加可执行文件
add_executable(my_service ${service_source})
# 添加静态库的路径

# 链接 xplatform 静态库和 libprotobuf 静态库
target_link_libraries(my_service PRIVATE
    ${CMAKE_SOURCE_DIR}/../lib/libbasicBox.a
    protobuf
    event              # Libevent 库
    ssl                # OpenSSL 库
    crypto             # OpenSSL 的 crypto 库
    pthread            # POSIX 线程库
    event_openssl      # Libevent 的 OpenSSL 支持库
)

# 设置头文件搜索路径
target_include_directories(my_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # gateway_server 自己的头文件路径
    ${CMAKE_SOURCE_DIR}/../src/basicBox  # xplatform 的头文件路径
)

