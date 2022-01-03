<?php

class IEAYearly extends IEABaseClass
{

    public function __construct($et) 
    {
        $this->setMultiplier(1 / 1000);
        parent::__construct($et);
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        // Data incomplete for 2019
        if($year == 2019) {
            return true;
        }

        if($diff_rel < 0.05) {
            return true;
        }
    }

    protected function getRawData() 
    {
        $ets = [];
        $years = [];
        $dir = dirname(__DIR__) . '/sources/iea/json/';
        $files = scandir($dir);
        foreach($files as $file) {
            if($file[0] == '.') {
                continue;
            }

            $json = file_get_contents($dir . '/' . $file);
            $data = json_decode($json);
            foreach($data as $i => $item) {
                $year = (int)$item->year;
                if(!in_array($year, $years)) {
                    $years[] = $year;
                }

                if($item->product != 'ELECTR') {
                    continue;
                }

                if(!isset($ets[$item->flowLabel])) {
                    $ets[$item->flowLabel] = [];
                }

                if(!isset($ets[$item->flowLabel][$item->country])) {
                    $ets[$item->flowLabel][$item->country] = [];
                }

                $value = $item->value;
                switch($item->units) {
                case 'GWh':
                    break;
                case 'ktoe':
                    $value = $value * 11.63;
                    break;
                default:
                    trigger_error('Unknown unit: ' . $item->units, E_USER_ERROR);
                }

                $ets[$item->flowLabel][$item->country][$item->year] = $value;
            }
        }
        return compact('ets', 'years');
    }
}
