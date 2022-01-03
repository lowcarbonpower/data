# Scripts used to generate data used on LowCarbonPower.org

This repository contains a series of scripts used to download, parse and generate electricity generation, net imports and electricity storage data used on the website https://lowcarbonpower.org/. 

## Sources directory

The Sources directory contains the raw source data obtained from BP, EIA, Ember, Enerdata, IEA, Sandia and the World Bank. In many cases there is a download script which can be used to download the source data, though it's likely these scripts need to be updated as new source data is released. They also contain scripts to perform basic parsing of the source data, such as converting Excel files to CSV.

## parse.php

The main script is called parse.php. By running it (php parse.php), a number of files will be generated in the same folder. The main ones are called:

- data-excluding-net-imports.csv
- data-excluding-net-imports.json
- data-including-net-imports.csv
- data-including-net-imports.json

All of these files contain a dataset that is created by combining all of the data sources in the Sources directory. The differences between the four files are that they either do or do not include net imports, and are either in a CSV or JSON format.

In addition to these main files, specific files for each source are also generated (data-bp.csv, data-eia.json etc). These files are exclusively based on data from the source specified by the filename.

For more information about the methodology, see https://lowcarbonpower.org/methodology. Also feel free to contact olof@lowcarbonpower.org or https://twitter.com/lowcarbonpower.
