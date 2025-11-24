
cmake_minimum_required(VERSION 3.6)
project(kart-example-epoll CXX)

include("${CMAKE_CURRENT_LIST_DIR}/CMakeOpt.cmake")

set(src "${CMAKE_CURRENT_LIST_DIR}/..")

add_executable(kart-server
  "${src}/protocol.cpp"
  "${src}/server.cpp"
  "${src}/epoll.cpp"
) 

add_executable(kart-client_tcp
  "${src}/protocol.cpp"
  "${src}/client_tcp.cpp"
) 

add_executable(kart-client_udp
  "${src}/protocol.cpp"
  "${src}/client_udp.cpp"
) 
