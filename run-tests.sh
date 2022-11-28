#!/bin/sh
# https://git.sr.ht/~sircmpwn/harec/tree/master/item/tests/run

printf 'Running tests on %s\n\n' "$(date)" >&2
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
		echo $name >&2
		$f
		if [ $? -eq 0 ]
		then
			npass=$((npass+1))
			printf 'PASS\n' >&2
		else
			nfail=$((nfail+1))
			printf 'FAIL\n' >&2
		fi
	fi
done

finish=$(date +"%s")
printf '\n%d tests:\t%d passed\t%d failed\tin %d seconds\n' \
	$ntests $npass $nfail $((finish-start)) >&2
if [ $nfail -ne 0 ]
then
	exit 1
fi
