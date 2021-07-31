# FROM alpine:3.14
# RUN apk add --update alpine-sdk
# RUN apk add --update cmake linux-headers

# WORKDIR /app/dependencies
# #build Boost
# RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.bz2  && \
#     tar --bzip2 -xf boost_1_76_0.tar.bz2 
# WORKDIR /app/dependencies/boost_1_76_0
# RUN ./bootstrap.sh --prefix=../deploy/
# RUN ./b2  --with-filesystem --with-system install
# RUN ls -la

# #build gflags
# WORKDIR /app/dependencies
# RUN git clone https://github.com/gflags/gflags.git && \
#     cd gflags && \
#     mkdir build && \
#     cd build && \
#     cmake -DBUILD_SHARED_LIBS=1 -DGFLAGS_INSTALL_SHARED_LIBS=1 -DCMAKE_INSTALL_PREFIX=../../deploy/ .. && \
#     make && make install    

# #install gRPC
# WORKDIR /app/dependencies
# RUN apk add --update zlib-dev
# RUN git clone -b v1.38.1 https://github.com/grpc/grpc && \
# cd grpc && \
# git submodule update --init 
# WORKDIR /app/dependencies/grpc
# RUN mkdir -p build && \
# cd build && \
# cmake .. \
# -DgRPC_BUILD_TESTS=OFF \
# -DgRPC_BUILD_CSHARP_EXT=OFF \
# -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
# -DgRPC_ZLIB_PROVIDER=package \
# -DCMAKE_INSTALL_PREFIX=../../deploy/ && \
# make -j 2 && make install

# #install RocksDB
# WORKDIR /app/dependencies
# RUN wget https://github.com/facebook/rocksdb/archive/refs/tags/v6.20.3.tar.gz && \
# tar -xzf v6.20.3.tar.gz && cd rocksdb-6.20.3 && \
# mkdir build && cd build && \
# cmake .. -DCMAKE_INSTALL_PREFIX=../../deploy/ -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=0 -DWITH_BENCHMARK_TOOLS=0 -DUSE_RTTI=1 && \
# make && make install

FROM jevon82/propanedb-base

#build propaneDB
WORKDIR /app 
COPY CMakeLists.txt CMakeLists.txt 
COPY protos protos
COPY src src
COPY test test
COPY cmake cmake
RUN mkdir build 
WORKDIR /app/build 
RUN cmake .. 
RUN make -j 2
#RUN ./propane_test
