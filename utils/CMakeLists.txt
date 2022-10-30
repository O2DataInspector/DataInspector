find_package(Boost 1.75 REQUIRED serialization)

include_directories(${PROJECT_SOURCE_DIR}/utils/include)
add_library(utils ThreadPool.cpp)
target_link_libraries(utils Boost::boost Boost::serialization)
