<?php

class BPUnspecifiedFossilFuels extends DataSeriesCombination
{

    private $bp_coal_gas_oil;
    private $region_parents = [
    '_OSCAM' => [
    '_CAM',
    'CL',
    'CO',
    'EC',
    '_OCA',
    '_OSAM',
    'PE',
    'TT',
    'VE',
    ],
    '_OE' => [ 
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
    'GR',
    'HU',
    'IS',
    'IE',
    'LV',
    'LT',
    'LU',
    'MK',
    'NO',
    'PT',
    'RO',
    'SK',
    'SI',
    'SE',
    'CH',
    ],
    '_OCIS' => [ 
    'AZ',
    'BY',
    'TM',
    'UZ',
    ],
    '_OME' => [ 
    'IQ',
    'IL',
    'KW',
    'OM',
    'QA',
    ],
    '_OAF' => [ 
    'DZ',
    'MA',
    '_EAF',
    '_MAF',
    '_WAF',
    '_ONAF2',
    '_OSAF',
    ],
    '_OAP' => [ 
    'BD',
    'HK',
    'NZ',
    'PK',
    'PH',
    'SG',
    'LK',
    ]
    ];

    private $region_children = [];

    public function __construct($total, $bp_coal_gas_oil, $non_fossil_plus_other) 
    {
        parent::__construct('Unspecified Fossil Fuels');
        $this->bp_coal_gas_oil = $bp_coal_gas_oil;

        $this->addMember($total);

        $this->addInverted($bp_coal_gas_oil);
        $this->addInverted($non_fossil_plus_other);

        foreach($this->region_parents as $parent => $children) {
            foreach($children as $child) {
                $this->region_children[] = $child;
            }
        }
    }

    // For each sub-region, calculate diff between total and others,
    // add diff to this region, and subtract it from parent.
    // Make sure parent equals sum of children and then remove it
    protected function setRegions() 
    {
        parent::setRegions();
        $regions = array_filter(
            parent::getRegions(), function ($region) {
                return in_array($region, $this->region_children);
            }, ARRAY_FILTER_USE_KEY
        );

        foreach($this->region_parents as $parent => $children) {
            $regions[$parent] = array_fill_keys($this->getYears(), 0);

            // Subtract sum of children from their parents
            foreach($children as $child) {
                foreach($regions[$child] as $year => $value) {
                    $regions[$parent][$year] -= $value;
                }
            }

            foreach($this->bp_coal_gas_oil->getYears() as $year) {
                $regions[$parent][$year] += $this->bp_coal_gas_oil->getValue($parent, $year);
            }
        }

        $world = array_fill_keys($this->getYears(), 0);
        foreach($regions as $region => $years) {
            foreach($years as $year => $value) {
                $world[$year] += $value;
            }
        }
        $regions['W'] = $world;

        $this->regions = $regions;
    }
        
    private function addInverted($data_series) 
    {
        $inverted = new DataSeriesInverted($data_series);
        $this->addMember($inverted);
    }
}
