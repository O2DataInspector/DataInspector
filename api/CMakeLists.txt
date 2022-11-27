find_package(httplib REQUIRED)
find_package(Arrow REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/api/include ${PROJECT_SOURCE_DIR}/domain/include ${PROJECT_SOURCE_DIR}/utils/include ${PROJECT_SOURCE_DIR}/serialization/include)
add_library(api DataEndpoint.cxx DevicesEndpoint.cxx BuildsEndpoint.cpp RunsEndpoint.cpp)
target_link_libraries(api serialization Arrow::arrow_shared)
