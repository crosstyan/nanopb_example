# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 17)

# find_package(Nanopb REQUIRED)
# include_directories(${NANOPB_INCLUDE_DIRS})
set(NANOPB_INCLUDE_DIRS nanopb)
set(NANOPB_SRCS 
  nanopb/pb_common.c 
  nanopb/pb_decode.c 
  nanopb/pb_encode.c 
)
# Set the project name
project (hello_cmake)
include_directories(include ${NANOPB_INCLUDE_DIRS})

# Add an executable
add_executable(hello_cmake ${NANOPB_SRCS} main.cpp msg.pb.c)