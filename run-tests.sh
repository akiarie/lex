#!/bin/sh
# https://git.sr.ht/~sircmpwn/harec/tree/master/item/tests/run

printf 'Running tests on %s\n\n' "$(date)"
start=$(date +"%s")

ntests=0
npass=0
nfail=0

for f in ./*_test
do
	if [ -x "$f" ]
	then
		ntests=$((ntests+1))
		name="$(basename "$f")"
		echo $name
		$f
		if [ $? -eq 0 ]
		then
			npass=$((npass+1))
			printf 'PASS\n'
		else
			nfail=$((nfail+1))
			printf 'FAIL\n'
		fi
	fi
done

finish=$(date +"%s")
printf '\n%d tests:\t%d passed\t%d failed\tin %d seconds\n' \
	$ntests $npass $nfail $((finish-start))
if [ $nfail -ne 0 ]
then
	exit 1
fi
