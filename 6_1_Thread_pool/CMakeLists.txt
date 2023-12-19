cmake_minimum_required(VERSION 3.23)
project(6.1Thread_pool C)

set(CMAKE_C_STANDARD 11)

# 查找 pthreads
find_package(Threads REQUIRED)

add_executable(6.1Thread_pool thread_pool.c)

# 链接 pthreads 库
target_link_libraries(6.1Thread_pool Threads::Threads)


