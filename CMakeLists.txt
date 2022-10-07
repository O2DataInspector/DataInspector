cmake_minimum_required(VERSION 3.22)
project(DIProxy)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(api)
add_subdirectory(domain)
add_subdirectory(infrastructure)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/api/include ${CMAKE_CURRENT_SOURCE_DIR}/domain/include ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/api ${CMAKE_CURRENT_SOURCE_DIR}/domain ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure)

# Dependencies
FIND_PACKAGE(Threads REQUIRED)

ADD_EXECUTABLE(proxy Proxy.cxx)

TARGET_LINK_LIBRARIES(proxy Threads::Threads api domain infrastructure)
