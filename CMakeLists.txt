cmake_minimum_required(VERSION 3.16)
project(DIProxy)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(utils)
add_subdirectory(api)
add_subdirectory(domain)
add_subdirectory(infrastructure)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/api/include ${CMAKE_CURRENT_SOURCE_DIR}/domain/include ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/include ${CMAKE_CURRENT_SOURCE_DIR}/utils/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/api ${CMAKE_CURRENT_SOURCE_DIR}/domain ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure ${CMAKE_CURRENT_SOURCE_DIR}/utils)

# Dependencies
FIND_PACKAGE(Threads REQUIRED)
find_package(mongoc-1.0 1.7 REQUIRED)

ADD_EXECUTABLE(proxy Proxy.cxx)

TARGET_LINK_LIBRARIES(proxy Threads::Threads api domain infrastructure utils mongo::mongoc_shared)
