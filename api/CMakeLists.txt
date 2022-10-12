FIND_PACKAGE(RapidJSON)
FIND_PACKAGE(httplib REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/api/include ${PROJECT_SOURCE_DIR}/domain/include ${PROJECT_SOURCE_DIR}/utils/include)
add_library(api DataEndpoint.cxx DevicesEndpoint.cxx AnalysisEndpoint.cpp)
