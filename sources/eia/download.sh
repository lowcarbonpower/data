#!/bin/bash
date=$(date +%m-%d-%Y_%H-%M-%S)
curl -v "https://www.eia.gov/international/api/series_data/csv?location32=ruvvvvvfvtvnvv1vrvvvvfvvvvvvfvvvou20evvvvvvvvvvnvvvs0008&products32=00000000000000000000000000000fvu&frequency=A&unit=0&start=315532800000&end=1546300800000&generated=$date" -o data.csv
