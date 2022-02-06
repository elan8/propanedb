#!/bin/bash
release=$1
docker tag ghcr.io/elan8/propanedb:latest ghcr.io/elan8/propanedb:$release 
docker push ghcr.io/elan8/propanedb:latest
docker push ghcr.io/elan8/propanedb:$release 