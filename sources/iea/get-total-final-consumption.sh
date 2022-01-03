#!/bin/bash
curl -v "https://api.iea.org/stats/indicator/TFCbySource?countries=ALL&startYear=1990" >json/tfc.json
