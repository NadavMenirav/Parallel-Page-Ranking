cmake_minimum_required(VERSION 3.31)
project(assignment_2 C)

set(CMAKE_C_STANDARD 99)

add_executable(assignment_2
        thr_pool.c
        thr_pool.h
        pagerank.c
        pagerank.h)