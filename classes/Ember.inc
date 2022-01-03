<?php

class Ember extends DataSeriesFromCsvFile
{

    private $et;
    private static $rows;
    private $years = [];

    public function __construct($et = null) 
    {
        $this->et = $et;
        $this->unset_regions_with_zero_sums = false;
        parent::__construct('sources/ember', 'csv/Data.csv');
        if(!isset(self::$rows)) {
            self::$rows = $this->getRows(1);
        }

        // Keep trade data with 0 sum
        if($et == 'Net imports') {
        }
    }

    public function getEnergyType() 
    {
        return $this->et;
    }

    public function getEnergyTypes() 
    {
        $ets = [];
        foreach(self::$rows as $row) {
            $et = $row[2];
            if(!in_array($et, $ets)) {
                $ets[] = $et;
            }
        }
        return $ets;
    }

    public function getYears() 
    {
        return $this->years;
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        if($diff_rel < pow(10, -3)) {
            return true;
        }

        if($diff_abs < 2) {
            return true;
        }

        // Incomplete world data for 2020
        if($year == 2020 && $region == 'W') {
            return true;
        }

        // Net imports for world don't make sense
        if($region == 'W' && $this->et == 'Net imports') {
            return true;
        }

        // Electricity totals are production plus net imports, if net imports are positive (negative net imports are ignored), so sums don't add up
        if($region == 'W' && $this->et == 'Electricity') {
            return true;
        }
    }

    protected static function getNormalizedRegionName($in) 
    {
        switch($in) {
        case 'EU-27':
        case 'EU27+1':
        case 'Falkland Islands':
        case 'Netherlands Antilles':
        case 'U.S. Pacific Islands':
        case 'Wake Island':
            return self::IGNORE_REGION;
        }

        return parent::getNormalizedRegionName($in);
    }

    protected function setRegions() 
    {
        $this->region_map['Kosovo'] = '_K';

        foreach(self::$rows as $row) {
            $et = $row[2];
            if($et != $this->et) {
                continue;
            }
            $year = (int)$row[0];
            $region = $row[1];
            $value = $row[4];

            $this->setValueByRegionName($region, $year, $value);

            if(!in_array($year, $this->years)) {
                $this->years[] = $year;
            }
        }
    }
}
