#!/bin/bash
docker build -t propanedb-clang -f ./docker/clang-tidy/Dockerfile .
#docker run --env CODACY_PROJECT_TOKEN=${CODACY_PROJECT_TOKEN} propanedb-test bash ./uploadCoverage.sh
