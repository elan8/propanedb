FROM ghcr.io/elan8/propanedb-base-ubuntu as builder
WORKDIR /app 
# RUN apt-get update -y && apt-get -y --no-install-recommends install gcovr && \
#     wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
#     echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' |  tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
#     apt-get update -y && apt-get install cmake -y \
#     && apt-get clean \
#     && rm -rf /var/lib/apt/lists/*
RUN mkdir /var/rocksdb && chmod 777 /var/rocksdb
COPY CMakeLists.txt CMakeLists.txt 
COPY protos protos
COPY src src
COPY test test
COPY cmake cmake
RUN mkdir build 
WORKDIR /app/build 
RUN cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TEST=ON .. 
RUN export DEBUG=true && make -j 2
RUN ctest -C Debug --output-on-failure --verbose
# RUN cmake -DUSE_GCOV:STRING=yes -DCMAKE_BUILD_TYPE:STRING=Debug ..
# RUN make test_coverage
# ENV CODACY_PROJECT_TOKEN=ef0f6f06eea647c0a4471e6deb824474
# RUN bash <(curl -Ls https://coverage.codacy.com/get.sh) report -r test_coverage.xml