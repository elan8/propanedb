FROM mcr.microsoft.com/cstsectools/codeql-container
RUN apt-get update -y && apt-get -y install zlib1g-dev

WORKDIR /app/dependencies
#build Boost
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.bz2  && \
    tar --bzip2 -xf boost_1_76_0.tar.bz2 && cd /app/dependencies/boost_1_76_0 && \
    ./bootstrap.sh --prefix=../deploy/ && \
    ./b2  --with-filesystem --with-system install && cd /app/dependencies/ && rm -R boost_1_76_0 && rm boost_1_76_0.tar.bz2 

RUN apt-get update -y && apt-get -y install gpg wget && \
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' |  tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
apt-get update -y && apt-get install cmake -y

#build gflags
WORKDIR /app/dependencies
RUN git clone https://github.com/gflags/gflags.git && \
    cd gflags && \
    mkdir build && \
    cd build && \
    cmake -DBUILD_SHARED_LIBS=1 -DGFLAGS_INSTALL_SHARED_LIBS=1 -DCMAKE_INSTALL_PREFIX=../../deploy/ .. && \
    make && make install && cd /app/dependencies/ && rm -R gflags   

#install gRPC
WORKDIR /app/dependencies
# RUN apk add --update zlib-dev
RUN git clone -b v1.38.1 https://github.com/grpc/grpc && \
cd grpc && \
git submodule update --init && cd /app/dependencies/grpc &&\
mkdir -p build && \
cd build && \
cmake .. \
-DgRPC_BUILD_TESTS=OFF \
-DgRPC_BUILD_CSHARP_EXT=OFF \
-DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
-DgRPC_ZLIB_PROVIDER=package \
-DCMAKE_INSTALL_PREFIX=../../deploy/ && \
make -j 2 && make install && cd /app/dependencies/grpc && rm -R build

#install RocksDB
WORKDIR /app/dependencies
RUN wget https://github.com/facebook/rocksdb/archive/refs/tags/v6.20.3.tar.gz && \
tar -xzf v6.20.3.tar.gz && cd rocksdb-6.20.3 && \
mkdir build && cd build && \
cmake .. -DCMAKE_INSTALL_PREFIX=../../deploy/ -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=0 -DWITH_BENCHMARK_TOOLS=0 -DUSE_RTTI=1 && \
make && make install && cd /app/dependencies && rm -R rocksdb-6.20.3 && rm v6.20.3.tar.gz

WORKDIR /app/dependencies
RUN wget https://github.com/google/glog/archive/refs/tags/v0.5.0.tar.gz && \
tar -xzf v0.5.0.tar.gz && \
cd glog-0.5.0 && \
mkdir build && cd build && \
cmake .. -DCMAKE_INSTALL_PREFIX=../../deploy/ -DCMAKE_BUILD_TYPE=Release && \
make && \
make install

ENV LD_LIBRARY_PATH="/usr/lib:${LD_LIBRARY_PATH}"

#build PEGTL
WORKDIR /app/dependencies
RUN wget https://github.com/taocpp/PEGTL/archive/refs/tags/3.2.0.tar.gz && \
tar -xzf 3.2.0.tar.gz && \
cd PEGTL-3.2.0 && \
mkdir build && \
cd build && \
cmake .. -DCMAKE_INSTALL_PREFIX=../../deploy/ && \
make && \
make install && \