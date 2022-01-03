#!/bin/bash
dir=$(date +%Y-%m-%d)
mkdir -p $dir
curl -v "https://api.iea.org/stats/indicator/TPESbySource?countries=ALL&startYear=1990" >$dir/tpes.json
