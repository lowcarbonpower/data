#!/bin/bash
cat data.csv | tail -n +2 >data2.csv
csvtool transpose data2.csv >data3.csv
