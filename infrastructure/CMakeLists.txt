find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
find_package(mongoc-1.0 1.7 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/infrastructure/include ${PROJECT_SOURCE_DIR}/domain/include /usr/local/include/libmongoc-1.0 /usr/local/include/libbson-1.0)
add_library(infrastructure InMemoryDevicesRepository.cpp InMemoryMessageRepository.cpp InMemoryAnalysisRepository.cpp InMemoryRunRepository.cpp)
target_link_libraries(infrastructure mongo::mongocxx_shared mongo::bsoncxx_shared)
