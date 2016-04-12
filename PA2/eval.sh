#!/bin/bash

if (! [ -d submissions ] ); then
	echo "submissions folder not found. Creating one for you"
	echo "Copy your zip file inside this folder"
	mkdir submissions
	exit 1
fi  

if (! [ -d tests ] ); then 
        echo "tests folder not found. Creating one for you"
        echo "Copy all .frag and .out test file inside this folder"
	mkdir tests
        exit 1  
fi

for filename in $PWD/submissions/*.zip
do 
        #remove all spaces
        mv "$filename" "${filename// /}" &>/dev/null
done

for filename in $PWD/submissions/*.zip
do
	echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
        echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo " "

	zipName=$(basename $filename)
	pidNumbers=${zipName%.zip}

	if cd submissions; then
	        rm -rf $pidNumbers
	        unzip $zipName -d $pidNumbers &>/dev/null
	        if cd $pidNumbers; then
			##remove git directory
			rm -rf .git/
			rm -rf __MACOSX
		else 
	                echo "Could not unzip or change directory $zipName ! Aborting." 1>&2
                	exit 1
		fi
	else
		echo "Could not change directory to submissions! Aborting." 1>&2
		exit 1
	fi

        if [ ! -f main.cc ]; then
		find . -mindepth 2 -type f -print -exec mv {} . \; &>/dev/null
        fi
        rm -rf .git/
	rm -rf glc
	
        result=""
        correctOut=0

	make clean &>/dev/null 
	make &> make_res.txt
	#Compare make files result. Check for errors and warnings
	if cmp -s "$PWD/make_res.txt" "$PWD/../../make_res.txt"
       	then
	 	result="$result 2"
        	correctOut=$((correctOut+2))
	else 
		result="$result 0"
	fi
		
	for testname in $PWD/../../tests/*.frag
	do	
		testid=$(basename $testname)
		testbasename=${testid%.frag}
		rm -rf "$testbasename".out
		./glc <$testname &> $testbasename.out
	done

        for testname in $PWD/../../tests/*.out
	do
                testid=$(basename $testname)
                testbasename=${testid%.out}
		if cmp -s $testname "$testbasename.out"
		then
			result="$result, 1"
			correctOut=$((correctOut+1))
		else
			echo $testbasename
			result="$result, 0"	
		fi
	done
	cd ../
	#rm -rf $pidNumbers
	cd ../

	IFS='_' read -r -a array <<< "$pidNumbers"
	for element in "${array[@]}"	
	do
		res="$element, $result :: $correctOut"
		echo $res;
		echo $res >> result.txt
	done	
done

