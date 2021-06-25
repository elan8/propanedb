cmake_minimum_required(VERSION 3.10)

# set the project name
project(PropaneDB)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
#RocksDB
ExternalProject_Add(
    rocksdbProject
    URL https://github.com/facebook/rocksdb/archive/refs/tags/v6.20.3.zip
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=0 -DWITH_BENCHMARK_TOOLS=0 -DUSE_RTTI=1
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

#GRPC
include(FetchContent)
set(gRPC_BUILD_TESTS  OFF)
set(gRPC_BUILD_CODEGEN ON)
set(gRPC_BUILD_CSHARP_EXT  OFF)

option(gRPC_BUILD_GRPC_PHP_PLUGIN "Build grpc_php_plugin" OFF)
option(gRPC_BUILD_GRPC_PYTHON_PLUGIN "Build grpc_python_plugin" OFF)

option(gRPC_BUILD_GRPC_RUBY_PLUGIN "Build grpc_ruby_plugin" OFF)


FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.38.1  # e.g v1.28.0
)

set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

set(GRPC_FETCHCONTENT ON)

include(./cmake/common.cmake)
# Proto file
get_filename_component(hw_proto "./protos/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

add_executable(server src/server.cpp)
add_dependencies(server  rocksdbProject hw_grpc_proto)
target_link_libraries(server rocksdb pthread 
hw_grpc_proto
${_REFLECTION}
${_GRPC_GRPCPP}
${_PROTOBUF_LIBPROTOBUF})

add_executable(client src/client.cpp)
add_dependencies(client  rocksdbProject hw_grpc_proto) 
target_link_libraries(client rocksdb pthread 
hw_grpc_proto
${_REFLECTION}
${_GRPC_GRPCPP}
${_PROTOBUF_LIBPROTOBUF})