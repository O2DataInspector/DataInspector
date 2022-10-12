FIND_PACKAGE(Boost 1.75 REQUIRED serialization filesystem)

include_directories(${PROJECT_SOURCE_DIR}/domain/include ${PROJECT_SOURCE_DIR}/utils/include)
add_library(domain DevicesService.cpp MessageService.cpp SocketManagerService.cpp AnalysisService.cpp BuildManager.cpp RunManager.cpp)
target_link_libraries(domain infrastructure api Boost::boost Boost::serialization Boost::filesystem utils)
