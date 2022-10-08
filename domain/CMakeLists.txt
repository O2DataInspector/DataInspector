FIND_PACKAGE(Boost 1.75 REQUIRED serialization)

include_directories(${PROJECT_SOURCE_DIR}/domain/include)
add_library(domain DevicesService.cpp MessageService.cpp SocketManagerService.cpp)
target_link_libraries(domain infrastructure api Boost::boost Boost::serialization)
