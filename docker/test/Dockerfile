FROM ghcr.io/elan8/propanedb-base-ubuntu as builder
WORKDIR /app 
RUN apt-get update && apt-get -y install curl jq
RUN curl -Ls -o codacy-coverage-reporter "$(curl -Ls https://api.github.com/repos/codacy/codacy-coverage-reporter/releases/latest | jq -r '.assets | map({name, browser_download_url} | select(.name | contains("codacy-coverage-reporter-linux"))) | .[0].browser_download_url')"
RUN chmod +x codacy-coverage-reporter

RUN mkdir /var/rocksdb && chmod 777 /var/rocksdb
COPY CMakeLists.txt CMakeLists.txt 
COPY protos protos
COPY src src
COPY test test
COPY cmake cmake
COPY .git .git
RUN mkdir build 

# RUN printenv
WORKDIR /app/build 
RUN cmake -DUSE_GCOV:STRING=yes -DCMAKE_BUILD_TYPE:STRING=Debug ..
RUN make test_coverage 
COPY scripts .

