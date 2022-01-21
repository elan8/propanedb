FROM ghcr.io/elan8/propanedb-base-ubuntu as builder
WORKDIR /app 
RUN mkdir /var/rocksdb && chmod 777 /var/rocksdb
COPY CMakeLists.txt CMakeLists.txt 
COPY protos protos
COPY src src
COPY test test
COPY cmake cmake
RUN mkdir build 
WORKDIR /app/build 
RUN cmake -DCMAKE_BUILD_TYPE=Debug .. 
RUN export DEBUG=true && make -j 2
RUN ctest -C Debug --output-on-failure --verbose
  