find_package(mongoc-1.0 REQUIRED)
find_package(bson-1.0 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/infrastructure/include ${PROJECT_SOURCE_DIR}/domain/include)
add_library(infrastructure InMemoryAnalysisRepository.cpp MongoAnalysisRepository.cpp MongoMessageRepository.cpp InMemoryDevicesRepository.cpp InMemoryRunRepository.cpp)
target_link_libraries(infrastructure mongo::mongoc_shared mongo::bson_shared)