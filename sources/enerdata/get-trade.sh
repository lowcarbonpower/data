#!/bin/bash
while read cc; do
	echo $cc
	curl 'https://yearbook.enerdata.net/php/actions.php' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:79.0) Gecko/20100101 Firefox/79.0' -H 'Accept: application/json, text/javascript, */*; q=0.01' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Content-Type: application/x-www-form-urlencoded; charset=UTF-8' -H 'X-Requested-With: XMLHttpRequest' -H 'Origin: https://yearbook.enerdata.net' -H 'Connection: keep-alive' -H 'Referer: https://yearbook.enerdata.net/electricity/electricity-balance-trade.html' -H 'Cookie: _ga=GA1.2.1762597375.1574675324; PHPSESSID=38e2f8ac3bf01cb073a2e96c83c8b296; _gid=GA1.2.1468824553.1598525011' --data-raw "action=getGraph&pays=$cc&serie=elexm&annee=2019&idDiv=graph" -v >json/$cc.json
done < <(cat ../../iso-3166.json | jq -r '.[] | .["alpha-2"]')
