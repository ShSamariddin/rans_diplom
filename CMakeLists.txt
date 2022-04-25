cmake_minimum_required(VERSION 3.21)
project(rans_diplom)

set(CMAKE_CXX_STANDARD 14)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fopenmp")

#add_executable(rans_diplom main.cpp )

add_executable(rans_encoder main.cpp pgm/pgm.cpp pgm/pgm.h rANS/rANS.cpp rANS/rANS.h rANS/rans_byte.h)
add_executable(rans_decoder decoder.cpp pgm/pgm.cpp pgm/pgm.h rANS/rANS.cpp rANS/rANS.h rANS/rans_byte.h)
