cmake_minimum_required(VERSION 3.10)

# set the project name
project(PropaneDB)

# add the executable

# target_include_directories(main PRIVATE ../rocksdb/include)

# target_link_libraries(Tutorial PUBLIC MathFunctions)


include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

ExternalProject_Add(rocksdbProject
    GIT_REPOSITORY https://github.com/facebook/rocksdb
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=0 -DWITH_BENCHMARK_TOOLS=0
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_executable(PropaneDB src/main.cpp)
add_dependencies(PropaneDB  rocksdbProject)
target_link_libraries(PropaneDB rocksdb )