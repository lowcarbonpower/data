<?php

class IEAMonthly extends DataSeriesFromCsvFile
{

    public static $current_year_months = 0;
    private $et;
    private $years = [];

    private static $rows_static;

    public function __construct($et = null) 
    {
        $this->region_row_offset = 9;
        $this->setMultiplier(1 / 1000);

        if(in_array($et, ['Total Imports', 'Total Exports'])) {
            $this->unset_regions_with_zero_sums = false;
        }

        $this->verifyWorldSums = false;

        parent::__construct('sources/iea', 'MES_092021.csv');

        if($et) {
            $this->et = $et;
        }
    }

    public function getEnergyType() 
    {
        return $this->et;
    }

    public function getEnergyTypes() 
    {
        $rows = $this->getRows($this->region_row_offset);
        $ets = [];
        foreach($rows as $i => $row) {
            $et = $row[3];
            if($et == 'Electricity') {
                $et = $row[2];
            }
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
        if($diff_rel < .8) {
            return true;
        }
    }

    protected static function getNormalizedRegionName($in) 
    {
        switch($in) {
        case 'OECD Americas':
        case 'OECD Asia Oceania':
        case 'OECD Europe':
        case 'OECD Total':
            return self::IGNORE_REGION;

        case 'IEA Total':
            return 'World';

        case "People's Republic of China":
            return 'China';
        }

        return parent::getNormalizedRegionName($in);
    }

    protected function getRows($row_offset = 0) 
    {
        if(!isset(self::$rows_static)) {
            self::$rows_static = parent::getRows($row_offset);
            self::$rows_static = array_filter(
                self::$rows_static, function ($row) {
                    return in_array($row[2], ['Net Electricity Production', 'Total Imports', 'Total Exports']);
                }
            );
        }
        return self::$rows_static;
    }

    protected function setRegions() 
    {
        $rows = $this->getRows($this->region_row_offset);
        foreach($rows as $i => $row) {
            $et = $row[3];
            if($et == 'Electricity') {
                $et = $row[2];
            }

            if($this->et && $this->et != $et) {
                continue;
            }

            list($month, $year) = explode(' ', $row[1]);
            $year = (int)$year;

            $month = (int)DateTime::createFromFormat('M', $month)->format('n');
            if($year == CURRENT_YEAR && $month > self::$current_year_months) {
                self::$current_year_months = $month;
            }

            if($year < 2019) {
                continue;
            }

            if(!in_array($year, $this->years)) {
                $this->years[] = $year;
            }

            $region_name = trim($row[0]);
            $value = $row[4];

            /*
            // Important to keep trade numbers even if null
            if(in_array($et, ['Total Imports', 'Total Exports']) && !$value) {
            if($value !== '0') {
            var_dump($value);
            exit;
            }
            $value = 0;
            }
            */

            $this->setValueByRegionName($region_name, $year, $value);
        }
        unset($this->regions['W']);
    }

    protected function setValueByRegionId($region_id, $year, $value) 
    {
        if(!isset($this->regions[$region_id][$year])) {
            parent::setValueByRegionId($region_id, $year, $value);
            return;
        }

        if(!$value) {
            return;
        }

        $value = $this->getNormalizedValue($value);

        $this->regions[$region_id][$year] += $value;
    }
}
