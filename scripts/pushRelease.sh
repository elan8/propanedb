#!/bin/bash
release="v0.1.2"
docker tag ghcr.io/elan8/propanedb:latest ghcr.io/elan8/propanedb:$release 
docker push ghcr.io/elan8/propanedb:$release 