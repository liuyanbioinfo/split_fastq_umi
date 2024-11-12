cmake_minimum_required(VERSION 3.15)
project(split_fastq_umi CXX)

# 设置编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g -O3")
set(CMAKE_STATIC_LINKER_FLAGS "-Wl,--no-as-needed -pthread")

# 包含头文件目录
include_directories(inc)

# 查找源文件
file(GLOB SOURCES "src/*.cpp")

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
find_package(ZLIB REQUIRED)
find_package(libdeflate REQUIRED)
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB pthread libdeflate::libdeflate_static)

# 设置Build输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# 安装路径配置, linux/macos 默认/usr/local/bin， 可以通过CMAKE_INSTALL_PREFIX设置安装路径
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
