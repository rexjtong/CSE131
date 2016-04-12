#!/bin/bash

cd final_test_cases

for testname in $PWD/*.glsl
        do
                testid=$(basename $testname)
                testbasename=${testid%.glsl}
                rm -rf "$testbasename".out
                ../glc <$testname &> $testbasename.out
        done

cd -
