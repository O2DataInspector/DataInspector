find_package(ROOT REQUIRED)
find_package(Arrow REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/serialization/include)
add_library(serialization RootSerialization.cpp ArrowSerialization.cpp)
target_link_libraries(serialization ROOT::Core ROOT::Tree Arrow::arrow_shared)
