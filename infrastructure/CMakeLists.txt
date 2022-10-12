find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/infrastructure/include ${PROJECT_SOURCE_DIR}/domain/include)
add_library(infrastructure InMemoryDevicesRepository.cpp InMemoryMessageRepository.cpp InMemoryAnalysisRepository.cpp InMemoryRunRepository.cpp)
target_link_libraries(infrastructure mongo::mongocxx_shared mongo::bsoncxx_shared)