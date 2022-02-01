#!/bin/bash
cd ./build/
cmake -DUSE_GCOV:STRING=yes -DCMAKE_BUILD_TYPE:STRING=Debug ..
make test_coverage