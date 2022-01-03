<?php

require_once '/usr/share/php/Services/JSON.php';

class EnerdataTrade extends DataSeriesBaseClass
{

    public function __construct() 
    {
        $this->unset_regions_with_zero_sums = false;
    }

    public function getEnergyType() 
    {
        return 'Trade';
    }

    public function getYears() 
    {
        static $years;
        if(!isset($years)) {
            $years = [];
            foreach($this->getItems() as $item) {
                foreach($item->chartData as $cd) {
                    $year = $cd->annee;
                    if(!isset($years[$year])) {
                        $years[$year] = 1;
                    }
                }
            }
            $years = array_keys($years);
            sort($years);
        }
        return $years;
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        return true;
    }

    protected function setRegions() 
    {
        foreach($this->getYears() as $year) {
            $this->setValueByRegionName('World', $year, 1);
        }

        foreach($this->getItems() as $item) {
            if(!$item->codeISOPaysChoisi) {
                continue;
            }
            $values = array_fill_keys($this->getYears(), null);
            foreach($item->chartData as $cd) {
                $cd = (array)$cd;
                $year = $cd['annee'];
                unset($cd['annee'], $cd['forceShow']);
                if(count($cd) > 0) {
                    $value = array_shift($cd);
                } else {
                    // Based on https://yearbook.enerdata.net/electricity/electricity-balance-trade.html, if a region array
                    // has no values at all, then they are displayed as 0 for all years, so we assume this is true.
                    // If mistaken, we hope that this will be caught when the values are compared to the IEA data source.
                    if(array_sum($values) == 0) {
                        $value = 0;
                    } else {
                        continue;
                    }
                }
                $value = $this->getNormalizedValue($value);
                $values[$year] = $value;
            }

            foreach($values as $year => $value) {
                $this->setValueByRegionId($item->codeISOPaysChoisi, $year, $value);
            }
        }
    }

    private function getItems() 
    {
        static $items;
        if(!isset($items)) {
            $items = [];
            $dir = dirname(__DIR__) . '/sources/enerdata/json';
            foreach(scandir($dir) as $file) {
                if($file[0] == '.') {
                    continue;
                }
                $path = $dir . '/' . $file;
                $json = file_get_contents($path);
                $data = json_decode($json);

                // No idea why this is here...
                if(in_array(
                    $data->countryTitle, [
                    'Kentucky',
                    'Maine',
                    'Mississippi',
                    'North Carolina',
                    'Virginia'
                    ]
                )
                ) {
                    continue;
                }

                $json_serv = new Services_JSON();
                $data->chartData = $json_serv->decode($data->chartData);
                $items[$file] = $data;
            }
        }
        return $items;
    }
}
