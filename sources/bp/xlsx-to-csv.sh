#!/bin/bash
mkdir -p csv
rm csv/*
src=xlsx/bp-stats-review-2021-all-data.xlsx
while IFS= read sheet; do
	output=csv/$(echo $sheet | sed -e 's/ /_/g' -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//').csv
	in2csv $src --sheet "$sheet" --skip-lines 2 \
		| grep -Pv "^," \
		| sed '1,/^Total World/!d' \
		| sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' \
		>$output
done < <(in2csv $src -n)

echo "Done"
