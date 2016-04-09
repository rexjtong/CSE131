#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

cd $dir

for f in *.glsl; do
    ../glc < $f &> .temp
    printf "Comparing output with %s: " ${f%%.*}.out 
 
    
    if  diff .temp ${f%%.*}.out ; then
        printf "${GREEN}PASS${NC}\n"
    else
        printf "${RED}FAIL${NC}\n"    
    fi
   
done



