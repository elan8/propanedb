#!/bin/bash
docker build -t propanedb-test -f ./docker/test/Dockerfile .
docker run --env CODACY_PROJECT_TOKEN=${CODACY_PROJECT_TOKEN} propanedb-test bash ./uploadCoverage.sh
