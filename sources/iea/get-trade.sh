#!/bin/bash
curl -v "https://api.iea.org/stats/indicator/ElecImportsExports?countries=ALL&startYear=1990" >json/trade.json
