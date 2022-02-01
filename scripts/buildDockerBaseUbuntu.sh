#!/bin/bash
docker build -t propanedb-base-ubuntu -f ./docker/base-ubuntu/Dockerfile .
docker tag propanedb-base-ubuntu ghcr.io/elan8/propanedb-base-ubuntu 
docker push ghcr.io/elan8/propanedb-base-ubuntu 