<?php

class EIA extends DataSeriesFromCsvFile
{

    protected $region_row_offset = 0;
    private $energy_types = [];
    private $et_key;

    public function __construct($et_key = null, $source_dir = 'sources/eia') 
    {
        parent::__construct($source_dir, 'data3.csv');
        $this->allowNegativeValues = true;
        $this->unset_regions_with_zero_sums = false;
        $this->et_key = $et_key;
    }

    public function getEnergyTypes() 
    {
        $this->getRows();
        return $this->energy_types;
    }

    public function getEnergyType() 
    {
        $et = $this->et_key;
        if($et == 'Electricity') {
            $et = 'Generation';
        }
        return $et;
    }

    public function getYears() 
    {
        static $years;
        if(!isset($years)) {
            $years = [];

            // Important: skip the first column which is the region. We unset this afterwards to make sure the 0 key isn't used.
            $years[0] = null;

            $rows = parent::getRows();
            foreach($rows as $row) {
                if($row[0] == 'Series Key') {
                    for($i = 8; $i < count($row); $i++) {
                        $year = (int)$row[$i];
                        if(!in_array($year, $years)) {
                            $years[] = $year;
                        }
                    }
                }
            }
        }

        unset($years[0]);

        return $years;
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        // Missing totals for 2019 and some regions
        if($year == 2019 && $diff_rel == 1) {
            return true;
        }

        if($diff_abs < 2.5) {
            return true;
        }
        if($diff_rel < 0.02) {
            return true;
        }
    }

    protected function getRows($row_offset = 0) 
    {
        $rows = parent::getRows($row_offset);
        $rows2 = [];
        foreach($rows as $row) {
            if($row[0] == 'Series Key') {
                $keys = $row;
                continue;
            }
            
            if(!isset($unit)) {
                $unit = $row[2];
                switch($unit) {
                 // Billion KWH = TWH
                case 'BKWH':
                    break;

                 // Quadrillion British thermal unit
                case 'QBTU':
                    $this->setMultiplier(293.0710701722222);
                    break;
                default:
                    trigger_error('Unknown unit: ' . $unit, E_USER_ERROR);
                }
            }
            $parts = preg_split('/(net generation|and other), /', $row[1], 2);
            if(count($parts) < 2) {
                $parts = explode(', ', $row[1], 2);
            }
            list($et, $region) = $parts;
            $et = str_replace('electricity', '', $et);
            $et = trim($et);
            $region = str_replace(', Annual', '', $region);

            if(!in_array($et, $this->energy_types)) {
                $this->energy_types[] = $et;
            }

            if($this->et_key && $this->et_key != $et) {
                continue;
            }

            $year_keys = array_flip(array_slice($keys, 8, null, true));
            $values = [];
            foreach($this->getYears() as $year) {
                $value = null;
                if(isset($year_keys[$year])) {
                    $year_key = $year_keys[$year];
                    $value = $row[$year_key];
                    if($value == '(s)') {
                        $value = 0;
                    }
                }
                $values[] = $value;
            }
            array_unshift($values, $region);
            $rows2[] = $values;
        }
        return $rows2;
    }

    protected static function getNormalizedRegionName($in) 
    {
        switch($in) {
        case 'American Samoa':
        case 'Antarctica':
        case 'Antigua and Barbuda':
        case 'Aruba':
        case 'British Virgin Islands':
        case 'Cayman Islands':
        case 'Cook Islands':
        case 'Falkland Islands':
        case 'Faroe Islands':
        case 'French Guiana':
        case 'French Polynesia':
        case 'Greenland':
        case 'Guadeloupe':
        case 'Guam':
        case 'Guinea-Bissau':
        case 'Hawaiian Trade Zone':
        case 'Martinique':
        case 'Micronesia':
        case 'Montserrat':
        case 'Nauru':
        case 'Netherlands Antilles':
        case 'New Caledonia':
        case 'Niue':
        case 'Northern Mariana Islands':
        case 'Palestinian Territories':
        case 'Reunion':
        case 'Saint Helena':
        case 'Saint Pierre and Miquelon':
        case 'Sao Tome and Principe':
        case 'Saint Kitts and Nevis':
        case 'Timor-Leste':
        case 'Tonga':
        case 'Turks and Caicos Islands':
        case 'Tuvalu':
        case 'U.S. Territories':
        case 'U.S. Virgin Islands':
        case 'U.S. Pacific Islands':
        case 'Vanuatu':
        case 'Wake Island':
        case 'Western Sahara':
            return self::IGNORE_REGION;
        }
        return WorldBank::getNormalizedRegionName($in);
    }

    protected function setRegions() 
    {
        $this->region_map['Former Czechoslovakia'] = '_FCS';
        $this->region_map['Former Serbia and Montenegro'] = '_FWM';
        $this->region_map['Former Yugoslavia'] = '_FY';
        $this->region_map['Germany, East'] = '_FGE';
        $this->region_map['Germany, West'] = '_FGW';
        $this->region_map['Kiribati'] = '_KI';
        $this->region_map['Kosovo'] = '_KO';
        $this->region_map['Former U.S.S.R.'] = '_FU';
        parent::setRegions();

        // World data for 2019 is incomplete
        $this->regions['W'][2019] = null;
    }
}
