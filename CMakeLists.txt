cmake_minimum_required(VERSION 3.10)

# set the project name
project(PropaneDB)

# add the executable

# target_include_directories(main PRIVATE ../rocksdb/include)

# target_link_libraries(Tutorial PUBLIC MathFunctions)


include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

ExternalProject_Add(
    rocksdbProject
    URL https://github.com/facebook/rocksdb/archive/refs/tags/v6.20.3.zip
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=0 -DWITH_BENCHMARK_TOOLS=0 -DUSE_RTTI=1
)

include(FetchContent)
FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.38.1  # e.g v1.28.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

# ExternalProject_Add(
#     gabseil-cpp
#     URL https://github.com/abseil/abseil-cpp/archive/refs/tags/20210324.2.zip
#     CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE
# )

# ExternalProject_Add(
#     grpcProject
#     URL https://github.com/grpc/grpc/archive/refs/tags/v1.38.1.zip
#     CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF
# )




include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_executable(PropaneDB src/main.cpp)
add_dependencies(PropaneDB  rocksdbProject)
target_link_libraries(PropaneDB rocksdb pthread)