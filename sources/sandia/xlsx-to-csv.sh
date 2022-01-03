#!/bin/bash
mkdir -p csv
src=GESDB_Projects_11_17_2020-2.xlsx
while IFS= read sheet; do
	output=csv/$(echo $sheet | sed -e 's/ /_/g' -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//').csv
	in2csv $src --sheet "$sheet" >$output
done < <(in2csv $src -n)

echo "Done"
