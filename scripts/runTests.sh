#!/bin/bash
cd ./build/
# cmake -DUSE_GCOV:STRING=yes -DCMAKE_BUILD_TYPE:STRING=Debug ..
# make test_coverage
#gcovr
#tail test_coverage.xml
bash <(curl -Ls https://coverage.codacy.com/get.sh) report -r test_coverage.xml