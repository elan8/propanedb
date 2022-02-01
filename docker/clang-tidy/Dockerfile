FROM ghcr.io/elan8/propanedb-base-ubuntu as builder
WORKDIR /app 
RUN mkdir /var/rocksdb && chmod 777 /var/rocksdb
COPY CMakeLists.txt CMakeLists.txt 
COPY protos protos
COPY src src
COPY test test
COPY cmake cmake
COPY .git .git

WORKDIR /app/build 
RUN cmake -DUSE_CLANG_TIDY:STRING=yes -DCMAKE_BUILD_TYPE:STRING=Debug ..
RUN make 
COPY scripts .

# RUN clang-tidy "<clang-tidy-configs>" | \
# # 2. Convert the Clang-Tidy output to a format that the Codacy API accepts
# ./codacy-clang-tidy-"<version>" | \
# # 3. Send the results to Codacy
# curl -XPOST -L -H "project-token: $PROJECT_TOKEN" \
#     -H "Content-type: application/json" -d @- \
#     "https://api.codacy.com/2.0/commit/$COMMIT/issuesRemoteResults"

# # 4. Signal that Codacy can use the sent results and start a new analysis
# curl -XPOST -L -H "project-token: $PROJECT_TOKEN" \
# 	-H "Content-type: application/json" \
# 	"https://api.codacy.com/2.0/commit/$COMMIT/resultsFinal"

