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
./b2  --with-filesystem --with-system install
cd ..

#build gRPC
git clone -b v1.38.1 https://github.com/grpc/grpc
cd grpc
git submodule update --init

mkdir -p build
cd build
cmake .. \
-DgRPC_BUILD_TESTS=OFF \
-DgRPC_BUILD_CSHARP_EXT=OFF \
-DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
-DgRPC_ZLIB_PROVIDER=package \
-DCMAKE_INSTALL_PREFIX=../../deploy/
make
make install
cd ..
cd ..

#build RocksDB
wget https://github.com/facebook/rocksdb/archive/refs/tags/v6.20.3.tar.gz
tar -xzf v6.20.3.tar.gz
cd rocksdb-6.20.3
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../deploy/ -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=0 -DWITH_BENCHMARK_TOOLS=0 -DUSE_RTTI=1
make
make install