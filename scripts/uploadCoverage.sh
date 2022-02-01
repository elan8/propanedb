#!/bin/bash
gcovr -x test_coverage.xml
bash <(curl -Ls https://coverage.codacy.com/get.sh) report -l CPP --force-language  -r test_coverage.xml