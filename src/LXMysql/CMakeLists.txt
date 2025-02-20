# 自动搜索当前目录下的所有 .cpp 文件
file(GLOB LXMysql_sources *.cpp)

# 添加静态库
add_library(LXMysql STATIC ${LXMysql_sources})

# 设置 LXMysql 的头文件路径（供其他模块使用）
target_include_directories(LXMysql PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 链接系统库
target_link_libraries(LXMysql PUBLIC
    mysqlclient     # MySQL 静态库
    protobuf        # Protocol Buffers 静态库
)

# 设置输出路径
set_target_properties(LXMysql PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib
)