#!/bin/bash
./unitTest
gcovr -r .. --exclude "../dependencies/deploy/*"  -x test_coverage.xml
bash <(curl -Ls https://coverage.codacy.com/get.sh) report -l CPP --force-language  -r test_coverage.xml