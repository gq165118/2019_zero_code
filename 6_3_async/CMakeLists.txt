cmake_minimum_required(VERSION 3.23)
project(6_3_async C)

set(CMAKE_C_STANDARD 11)

# 查找 pthreads
find_package(Threads REQUIRED)

# async_dns_client_noblock.c
add_executable(6_3_async async_dns_client_noblock.c)

# 链接 pthreads 库
target_link_libraries(6_3_async Threads::Threads)