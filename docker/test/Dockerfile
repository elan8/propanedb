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

# && gcovr -x test_coverage.xml#
# RUN ../codacy-coverage-reporter report -r test_coverage.xml
#RUN wget  -O - https://coverage.codacy.com/get.sh | sh
#RUN ./uploadCoverage.sh
# && bash <(curl -Ls https://coverage.codacy.com/get.sh) report -r test_coverage.xml
# RUN cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TEST=ON .. 
# RUN export DEBUG=true && make -j 2
# RUN ctest -C Debug --output-on-failure --verbose
# RUN cmake -DUSE_GCOV:STRING=yes -DCMAKE_BUILD_TYPE:STRING=Debug ..
# RUN make test_coverage
# ENV CODACY_PROJECT_TOKEN=ef0f6f06eea647c0a4471e6deb824474
# RUN bash <(curl -Ls https://coverage.codacy.com/get.sh) report -r test_coverage.xml