include_directories(${PROJECT_SOURCE_DIR}/infrastructure/include ${PROJECT_SOURCE_DIR}/domain/include)
add_library(infrastructure InMemoryDevicesRepository.cpp InMemoryMessageRepository.cpp)
