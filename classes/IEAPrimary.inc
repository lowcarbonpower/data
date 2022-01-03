<?php

class IEAPrimary extends IEABaseClass
{

    public function __construct() 
    {
        parent::__construct('Primary');
    }

    public function getSource() 
    {
        return 'IEA';
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        // Data incomplete for 2019
        if($year == 2019) {
            return true;
        }

        if($diff_rel < 0.06) {
            return true;
        }
    }

    protected function getRawData() 
    {
        $this->setMultiplier(11.63 / 1000);

        $dir = dirname(__DIR__) . '/sources/iea/json/';
        $file = 'tpes.json';
        $json = file_get_contents($dir . '/' . $file);
        $data = json_decode($json);
        $years = [];
        $regions = [];
        foreach($data as $i => $item) {
            $year = (int)$item->year;
            if(!in_array($year, $years)) {
                $years[] = $year;
            }

            $region = $item->country;

            $value = $item->value;

            if(!isset($regions[$region])) {
                $regions[$region] = [];
            }
            if(!isset($regions[$region][$year])) {
                $regions[$region][$year] = 0;
            }

            $regions[$region][$year] += $value;
        }
        $ets = [
        'Primary' => $regions
        ];
        return compact('ets', 'years');

        foreach($regions as $region => $years) {
            foreach($years as $year => $value) {
                $this->setValueByRegionName($region, $year, $value);
            }
        }
    }
}
