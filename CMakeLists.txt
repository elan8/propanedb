cmake_minimum_required(VERSION 3.10)

# set the project name
project(PropaneDB)

include(FetchContent)
#include(FindProtobuf)
#include(./cmake/FindProtobuf.cmake)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# include(ExternalProject)

# set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
# #RocksDB
# ExternalProject_Add(
#     rocksdbProject
#     URL https://github.com/facebook/rocksdb/archive/refs/tags/v6.20.3.zip
#     CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=0 -DWITH_BENCHMARK_TOOLS=0 -DUSE_RTTI=1
# )

# include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
# link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

#GRPC
#include(FetchContent)
# set(gRPC_BUILD_TESTS  OFF)
# set(gRPC_BUILD_CODEGEN ON)
# set(gRPC_BUILD_CSHARP_EXT  OFF)
# option(gRPC_BUILD_GRPC_PHP_PLUGIN "Build grpc_php_plugin" OFF)
# option(gRPC_BUILD_GRPC_PYTHON_PLUGIN "Build grpc_python_plugin" OFF)
# option(gRPC_BUILD_GRPC_RUBY_PLUGIN "Build grpc_ruby_plugin" OFF)


# FetchContent_Declare(
#   gRPC
#   GIT_REPOSITORY https://github.com/grpc/grpc
#   GIT_TAG        v1.38.1  # e.g v1.28.0
# )

# set(FETCHCONTENT_QUIET OFF)
# FetchContent_MakeAvailable(gRPC)

# set(GRPC_FETCHCONTENT ON)

#set(CMAKE_PREFIX_PATH "./grpc/deploy/lib/cmake/grpc")
list(APPEND CMAKE_PREFIX_PATH "./dependencies/deploy")
list(APPEND CMAKE_PREFIX_PATH "./dependencies/deploy/lib/cmake")
list(APPEND CMAKE_PREFIX_PATH "./dependencies/deploy/lib/cmake/rocksdb")
#list(APPEND CMAKE_PREFIX_PATH "./grpc/deploy")
#set(CMAKE_PREFIX_PATH "./grpc/deploy")
#set(CMAKE_PREFIX_PATH "./dependencies/deploy")
#set(CMAKE_PREFIX_PATH "./dependencies/deploy/lib/cmake")
include_directories("./dependencies/deploy/include")
link_directories("./dependencies/deploy/lib")
LIST(APPEND CMAKE_PROGRAM_PATH "./dependencies/deploy/bin")

find_package (Boost 1.76.0 PATHS ./boost REQUIRED COMPONENTS filesystem)

# find_package(Boost 1.76.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if ( Protobuf_FOUND )
    message( STATUS "Protobuf version : ${Protobuf_VERSION}" )
    message( STATUS "Protobuf include path : ${Protobuf_INCLUDE_DIRS}" )
    message( STATUS "Protobuf libraries : ${Protobuf_LIBRARIES}" )
    message( STATUS "Protobuf compiler libraries : ${Protobuf_PROTOC_LIBRARIES}")
    message( STATUS "Protobuf lite libraries : ${Protobuf_LITE_LIBRARIES}")
    message( STATUS "Protoc : ${Protobuf_PROTOC_EXECUTABLE}")
else()
    message( WARNING "Protobuf package not found -> specify search path via PROTOBUF_ROOT variable")
endif()

#find_package(gRPC REQUIRED )
find_package(gRPC REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
message(STATUS "gRPC libraries ${gRPC_LIBRARIES}")

find_package(RocksDB REQUIRED )
set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure)

#include(./cmake/common.cmake)
# Proto file
get_filename_component(hw_proto "./protos/propanedb.proto" ABSOLUTE)
get_filename_component(test_proto "./protos/test.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)
get_filename_component(base_proto "${CMAKE_SOURCE_DIR}/dependencies/grpc/third_party/protobuf/src" ABSOLUTE)
#"${CMAKE_SOURCE_DIR}/dependencies/grpc/third_party/abseil-cpp")
# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/propanedb.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/propanedb.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/propanedb.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/propanedb.grpc.pb.h")
set(test_hdrs "${CMAKE_CURRENT_BINARY_DIR}/test.pb.cc")
set(test_srcs "${CMAKE_CURRENT_BINARY_DIR}/test.pb.h")
set(descriptor "${CMAKE_CURRENT_BINARY_DIR}/descriptor.bin")

#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${hw_proto}")

#message(STATUS "PROTO_SRCS  ${PROTO_SRCS}")
#protobuf_generate_cpp("${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}" "${hw_proto}")

set(BIN_DIR "../dependencies/deploy/bin")

add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}" -I "${base_proto}"
        --plugin=protoc-gen-grpc="${BIN_DIR}/grpc_cpp_plugin"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

add_custom_command(
      OUTPUT "${test_srcs}" "${test_hdrs}" 
      COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      ARGS --descriptor_set_out "${CMAKE_CURRENT_BINARY_DIR}/descriptor.bin"
      --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}" -I "${base_proto}"
        "${test_proto}"
      DEPENDS "${test_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/dependencies/grpc/third_party/abseil-cpp")

link_directories("${CMAKE_SOURCE_DIR}/dependencies/deploy/lib")


add_library(propane_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})
target_link_libraries(propane_proto
gRPC::grpc++_reflection
  gRPC::grpc++
  ${Protobuf_LIBRARIES})

add_library(test_proto
  ${test_srcs}
  ${test_hdrs}
)
target_link_libraries(test_proto
gRPC::grpc++_reflection
  gRPC::grpc++
  ${Protobuf_LIBRARIES})  

add_executable(server src/server.cpp src/DatabaseServiceImpl.cpp  )
add_dependencies(server propane_proto)
target_link_libraries(server 
rocksdb 
pthread 
propane_proto
gRPC::grpc++_reflection
gRPC::grpc++
${Protobuf_LIBRARIES})

enable_testing()

add_executable(
  server_test
  test/server_test.cpp
  src/DatabaseServiceImpl.cpp 
)
add_dependencies(server_test propane_proto test_proto)
target_link_libraries(
  server_test
  gtest_main
  rocksdb 
  pthread 
  propane_proto
  test_proto
  Boost::filesystem
  gRPC::grpc++_reflection
  gRPC::grpc++
  ${Protobuf_LIBRARIES})

include(GoogleTest)
gtest_discover_tests(server_test)

