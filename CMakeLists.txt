cmake_minimum_required(VERSION 3.16)
project(DIProxy)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(utils)
add_subdirectory(api)
add_subdirectory(domain)
add_subdirectory(infrastructure)
add_subdirectory(serialization)

find_package(RapidJSON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/api/include ${CMAKE_CURRENT_SOURCE_DIR}/domain/include ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/include ${CMAKE_CURRENT_SOURCE_DIR}/utils/include ${CMAKE_CURRENT_SOURCE_DIR}/serialization/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/api ${CMAKE_CURRENT_SOURCE_DIR}/domain ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure ${CMAKE_CURRENT_SOURCE_DIR}/utils ${CMAKE_CURRENT_SOURCE_DIR}/serialization)

# Dependencies
FIND_PACKAGE(Threads REQUIRED)

ADD_EXECUTABLE(proxy Proxy.cxx)

TARGET_LINK_LIBRARIES(proxy Threads::Threads api domain infrastructure utils serialization)
