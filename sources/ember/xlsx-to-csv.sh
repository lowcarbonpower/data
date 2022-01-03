#!/bin/bash
mkdir -p csv
src=xlsx/Data-Global-Electricity-Review-2021.xlsx
while IFS= read sheet; do
	output=csv/$(echo $sheet | sed -e 's/ /_/g' -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//').csv
	in2csv $src --sheet "$sheet" --skip-lines 1 \
	| grep -Pv "^," \
	>$output
done < <(in2csv $src -n)
echo "Done"
