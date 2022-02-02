FROM ghcr.io/elan8/propanedb-base-ubuntu as builder
WORKDIR /app 
RUN wget -O codacy-clang-tidy  https://github.com/codacy/codacy-clang-tidy/releases/download/1.3.2/codacy-clang-tidy-linux-1.3.2 
RUN mkdir /var/rocksdb && chmod 777 /var/rocksdb
COPY CMakeLists.txt CMakeLists.txt 
COPY protos protos
COPY src src
COPY test test
COPY cmake cmake
COPY .git .git
COPY .clang-tidy .clang-tidy
COPY scripts scripts

RUN mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && make && \
cd .. && clang-tidy -p=./build ./src/*.cpp | \
./codacy-clang-tidy 

# curl -XPOST -L -H "project-token: $PROJECT_TOKEN" \
#     -H "Content-type: application/json" -d @- \
#     "https://api.codacy.com/2.0/commit/$COMMIT/issuesRemoteResults"




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

