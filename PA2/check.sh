#! /bin/sh

[ -x glc ] || { echo "Error: glc not executable"; exit 1; }

LIST=
if [ "$#" = "0" ]; then
	LIST=`ls samples/*.out`
else
	for test in "$@"; do
		LIST="$LIST samples/$test.out"
	done
fi

for file in $LIST; do
	base=`echo $file | sed 's/\(.*\)\.out/\1/'`

	ext=''
	if [ -r $base.frag ]; then
		ext='frag'
	elif [ -r $base.glsl ]; then
		ext='glsl'
	else
		echo "Error: Input file for base: $base not found"
		exit 1
	fi

	tmp=${TMP:-"/tmp"}/check.tmp
	./glc < $base.$ext 1>$tmp 2>&1

	printf "Checking %-27s: " $file
	if ! cmp -s $tmp $file; then
		echo "FAIL <--"
		diff $tmp $file
	else
		echo "PASS"
	fi
done
