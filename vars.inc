<?php

define('CURRENT_YEAR', 2021);

define(
    'EMISSIONS', [
    'coal' => 820,
    'gas' => 490,
    'biofuels' => 230,
    'geothermal' => 38,
    'hydro' => 24,
    'nuclear' => 12,
    'solar' => 45,
    'wind' => 11,
    'oil' => 650,

    // Estimated ranges
    'geothermal-and-biofuels' => [38, 230],
    'renewables-except-hydro' => [11, 230],
    'renewables-including-hydro' => [11, 230],
    'unspecified-renewables' => [11, 230],
    'unspecified-fossil-fuels' => [490, 820],
    'other' => [11, 820],
    'trade' => [24, 820]
    ]
);

// Energy Types Structure
define(
    'ETSS', [
    'total' => [ 
        'lowcarbon',
        'fossil',
        'other',
        'trade'
    ],
     'lowcarbon' => [ 
        'hydro',
        'nuclear',
        'renewables-including-hydro'
     ],
     'renewables-including-hydro' => [
        'hydro',
        'renewables-except-hydro',
     ],
     'renewables-except-hydro' => [ 
        'wind-and-solar',
        'geothermal-and-biofuels',
        'unspecified-renewables',
     ],
    'wind-and-solar' => [
        'wind',
        'solar'
    ],
     'geothermal-and-biofuels' => [ 
        'geothermal',
        'biofuels'
     ],
     'fossil' => [ 
        'coal',
        'gas',
        'unspecified-fossil-fuels',
        'oil',
     ]
    ]
);

// Energy Types
define(
    'ETS', (function () {
        $ets = [];
        foreach(ETSS as $k => $ets_) {
            if($k != 'total') {
                $ets[] = $k;
            }
            foreach($ets_ as $et) {
                $ets[] = $et;
            }
        }
        $ets = array_unique($ets);
        $ets = array_values($ets);

        // sort these last
        foreach(['unspecified-fossil-fuels', 'other', 'trade'] as $et) {
            $k = array_search($et, $ets);
            if($k !== false) {
                unset($ets[$k]);
                $ets[] = $et;
            }
        }

        return $ets;
    })()
);

// Region Groups
define(
    'REGION_GROUPS', [
    'EU' => [
        'AT',
        'BE',
        'BG',
        'HR',
        'CY',
        'CZ',
        'DK',
        'EE',
        'FI',
        'FR',
        'DE',
        'GR',
        'HU',
        'IE',
        'IT',
        'LV',
        'LT',
        'LU',
        'MT',
        'NL',
        'PL',
        'PT',
        'RO',
        'SK',
        'SI',
        'ES',
        'SE'
    ],
    'SSA' => [
        'AO',
        'BJ',
        'BW',
        'BF',
        'BI',
        'CM',
        'CV',
        'CF',
        'TD',
        'KM',
        'CG',
        'CD',
        'CI',
        'DJ',
        'GQ',
        'ER',
        'ET',
        'GA',
        'GM',
        'GH',
        'GN',
        'GW',
        'KE',
        'LS',
        'LR',
        'MG',
        'ML',
        'MW',
        'MR',
        'MU',
        'MZ',
        'NA',
        'NE',
        'NG',
        'RE',
        'RW',
        'ST',
        'SN',
        'SC',
        'SL',
        'SO',
        'ZA',
        'SS',
        'SD',
        'SZ',
        'TZ',
        'TG',
        'UG',
        // 'EH',
        'ZM',
        'ZW'
    ]
    ]
);
