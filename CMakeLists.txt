cmake_minimum_required(VERSION 3.22)
project(DIProxy)
set(CMAKE_CXX_STANDARD 17)

# Dependencies
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(Boost 1.75 REQUIRED serialization)
FIND_PACKAGE(httplib REQUIRED)

# Include directories and libraries
INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_EXECUTABLE(proxy Proxy.cxx)

TARGET_LINK_LIBRARIES(proxy Threads::Threads Boost::boost Boost::serialization httplib::httplib)
