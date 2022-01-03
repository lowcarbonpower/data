#!/bin/bash
while read region; do
	echo "$region"
	region_encoded=$(echo "$region" | sed -e 's/ /-/g' -e 's/&/and/g' | tr '[:upper:]' '[:lower:]')
	url="https://data.worldbank.org/region/$region_encoded?view=chart"
	curl -v $url | grep -oP '(?<=<li class="label" data-reactid="\d{3}">)[^<]+?(?=</li>)'
	echo
done <regions >regions_and_children
