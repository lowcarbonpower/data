#!/bin/bash
function download {
	source=$1
	unit=$2
	dir="csv/$source/$unit"
	mkdir -p $dir
	cd $dir
	curl -v "http://api.worldbank.org/v2/en/indicator/EG.ELC.$source.$unit?downloadformat=csv" >data.zip
	unzip data.zip
	mv API* data.csv
}

for source in COAL FOSL HYRO NGAS NUCL PETR RNEW RNWX; do
	for unit in KH ZS; do
		(download $source $unit)
	done
done

echo "Done"
