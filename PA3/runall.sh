#! /bin/sh

[ -x glc ] || { echo "Error: glc not executable"; exit 1; }

LIST=
if [ "$#" = "0" ]; then
	LIST=`ls samples/*.glsl`
else
	for test in "$@"; do
		LIST="$LIST samples/$test.glsl"
	done
fi

for file in $LIST; do
        echo $file
	./glc < $file 
done
