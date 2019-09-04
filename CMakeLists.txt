cmake_minimum_required(VERSION 3.14)
project(SO_Tp1 C)

set(CMAKE_C_STANDARD 11)

add_executable(SO_Tp1
        slave_process.c application_process.c Include/application_process.h)
