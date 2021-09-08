#!/bin/bash
mkdir dependencies 
cd  dependencies

#build Boost
if [ ! -f boost_1_76_0.tar.bz2 ]; then
    echo "File not found!"
    wget https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.bz2
fi

if [ ! -d "./boost_1_76_0 " ]; then 
    echo "Folder not found!"
    tar --bzip2 -xf boost_1_76_0.tar.bz2
fi
cd boost_1_76_0
./bootstrap.sh --prefix=../deploy/
./b2 release --with-filesystem --with-system install
cd ..

#build gRPC
git clone -b v1.38.1 https://github.com/grpc/grpc
cd grpc
git submodule update --init

mkdir -p build
cd build
cmake .. \
-DCMAKE_BUILD_TYPE=Release \
-DgRPC_BUILD_TESTS=OFF \
-DgRPC_BUILD_CSHARP_EXT=OFF \
-DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
-DgRPC_ZLIB_PROVIDER=package \
-DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF \
-DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
-DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF \
-DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
-DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF \
-DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
-DCMAKE_INSTALL_PREFIX=../../deploy/
make
make install
cd ..
cd ..

#build RocksDB
wget https://github.com/facebook/rocksdb/archive/refs/tags/v6.20.3.tar.gz
tar -xzf v6.20.3.tar.gz
cd rocksdb-6.20.3
mkdir build 
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../deploy/ -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=0 -DWITH_BENCHMARK_TOOLS=0 -DUSE_RTTI=1
make
make install
cd ..
cd ..

#build glog
wget https://github.com/google/glog/archive/refs/tags/v0.5.0.tar.gz
tar -xzf v0.5.0.tar.gz
cd glog-0.5.0
mkdir build 
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../deploy/ -DCMAKE_BUILD_TYPE=Release 
make
make install
cd ..
cd ..

#build PEGTL
wget https://github.com/taocpp/PEGTL/archive/refs/tags/3.2.0.tar.gz
tar -xzf 3.2.0.tar.gz
cd PEGTL-3.2.0
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../deploy/ -DCMAKE_BUILD_TYPE=Release -DPEGTL_BUILD_TESTS=0 -DPEGTL_BUILD_EXAMPLES=0
make
make install
cd ..
cd ..