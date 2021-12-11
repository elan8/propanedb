cmake_minimum_required(VERSION 3.10)
project(PropaneDB)
include(FetchContent)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

list(APPEND CMAKE_PREFIX_PATH "./dependencies/deploy")
list(APPEND CMAKE_PREFIX_PATH "./dependencies/deploy/lib/cmake")
list(APPEND CMAKE_PREFIX_PATH "./dependencies/deploy/lib/cmake/rocksdb")
include_directories("./dependencies/deploy/include")
link_directories("./dependencies/deploy/lib")
LIST(APPEND CMAKE_PROGRAM_PATH "./dependencies/deploy/bin")

find_package (glog 0.5.0 REQUIRED)
find_package (Boost 1.76.0 REQUIRED COMPONENTS filesystem)
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

find_package(gRPC REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
message(STATUS "gRPC libraries ${gRPC_LIBRARIES}")

find_package(RocksDB REQUIRED )
set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure)

find_package(pegtl)

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

add_custom_target(protoGenerate DEPENDS "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}")      

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/dependencies/grpc/third_party/abseil-cpp")
link_directories("${CMAKE_SOURCE_DIR}/dependencies/deploy/lib")

add_library(propane_lib
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs}
  src/DatabaseServiceImpl.cpp 
  src/DatabaseImpl.cpp 
  src/QueryParser.cpp 
  src/Query.cpp
  src/FileWriter.cpp 
  src/FileReader.cpp
  )
  add_dependencies(propane_lib protoGenerate)
target_link_libraries(propane_lib
  rocksdb 
  pthread 
  gRPC::grpc++_reflection
  gRPC::grpc++
  ${Protobuf_LIBRARIES} 
  glog::glog
  taocpp::pegtl
  PocoFoundation
  PocoZip
  )


add_library(test_proto
  ${test_srcs}
  ${test_hdrs}
)
target_link_libraries(test_proto
gRPC::grpc++_reflection
  gRPC::grpc++
  ${Protobuf_LIBRARIES})  

add_executable(server src/server.cpp  )
add_dependencies(server propane_lib)
target_link_libraries(server 
propane_lib
gRPC::grpc++_reflection
gRPC::grpc++
${Protobuf_LIBRARIES})

enable_testing()

add_executable(
  propane_test
  test/server_test.cpp
)
add_dependencies(propane_test propane_lib test_proto)
target_link_libraries(
  propane_test
  gtest_main
  rocksdb 
  pthread 
  propane_lib
  test_proto
  Boost::filesystem
  gRPC::grpc++_reflection
  gRPC::grpc++
  ${Protobuf_LIBRARIES})

include(GoogleTest)
gtest_discover_tests(propane_test PROPERTIES TIMEOUT 600)
