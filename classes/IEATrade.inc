<?php

class IEATrade extends IEABaseClass
{

    private $max_year;

    public function __construct($max_year = null) 
    {
        $this->max_year = $max_year;

        // ktoe -> TWh
        $this->setMultiplier(0.01163);

        $this->unset_regions_with_zero_sums = false;

        parent::__construct('Trade');
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        return true;
    }

    public function getSource() 
    {
        return 'IEAYearly';
    }

    protected function getRawData() 
    {
        $ets = [];
        $years = [];
        $path = dirname(__DIR__) . '/sources/iea/json/trade.json';
        $json = file_get_contents($path);
        $items = json_decode($json);
        foreach($items as $i => $item) {
            $year = (int)$item->year;

            if(!in_array($year, $years)) {
                $years[] = $year;
            }
            if(!isset($ets[$this->energy_type])) {
                $ets[$this->energy_type] = [];
            }

            if(!isset($ets[$this->energy_type][$item->country])) {
                $ets[$this->energy_type][$item->country] = [];
            }
            if(!isset($ets[$this->energy_type][$item->country][$year])) {
                $ets[$this->energy_type][$item->country][$year] = 0;
            }

            $ets[$this->energy_type][$item->country][$item->year] += $item->value;
        }
        return compact('ets', 'years');
    }

    protected function setValueByRegionName($region_name, $year, $value) 
    {
        if($this->max_year && $year > $this->max_year) {
            return;
        }
        parent::setValueByRegionName($region_name, $year, $value);
    }
}
