cmake_minimum_required(VERSION 2.00)
project(tlb C)

set(CMAKE_C_STANDARD 11)

add_executable(tlb main.c utils.c utils.h)
add_executable(measure_tlb measure_tlb.c utils.c utils.h)
add_executable(measure_interval measure_interval.c utils.c utils.h)
add_executable(bw utils.c utils.h measure_bandwidth.c)
