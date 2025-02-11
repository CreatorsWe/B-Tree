cmake_minimum_required(VERSION 3.10)
project(test)
add_executable(test B-Tree.h test.cpp Bnode.cpp)
