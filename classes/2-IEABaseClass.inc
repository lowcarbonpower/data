<?php

abstract class IEABaseClass extends DataSeriesBaseClass
{

    protected $energy_type;
    protected static $raw_data;

    public function __construct($energy_type) 
    {
        $this->energy_type = $energy_type;
    }

    public final function getEnergyType() 
    {
        return $this->energy_type;
    }

    public function getYears() 
    {
        $raw_data = $this->getRawDataForThisClass();
        if(!isset($raw_data['years'])) {
            trigger_error('Missing years', E_USER_ERROR);
        }
        return $raw_data['years'];
    }

    protected final function setRegions() 
    {
        $this->region_map['Kosovo'] = '_K';

        $raw_data = $this->getRawDataForThisClass();
        $ets = $raw_data['ets'];
        if(!isset($ets[$this->energy_type])) {
            print_r(array_keys($ets));
            trigger_error('Missing energy type: ' . $this->energy_type, E_USER_ERROR);
        }
        foreach($ets[$this->energy_type] as $region_name => $years) {
            foreach($years as $year => $value) {
                $this->setValueByRegionName($region_name, $year, $value);
            }
        }
    }

    protected abstract function getRawData();

    protected final static function getNormalizedRegionName($in) 
    {
        switch($in) {
        case 'AFRICA':
        case 'ASIA':
        case 'CHINAREG':
        case 'EU28':
        case 'EURASIA':
        case 'LATINAMERI':
        case 'MASEAN':
        case 'MIDDLEEAST':
            return self::IGNORE_REGION;
        }

        switch(substr($in, 0, 3)) {
        case 'IEA':
        case 'NON':
        case 'OEC':
        case 'WEO':
            return self::IGNORE_REGION;
        }

        switch($in) {
        case 'UNK':
            return 'Kosovo';
        case 'WORLD':
            return 'World';
        }

        static $alpha3map;
        if(!isset($alpha3map)) {
            $json = file_get_contents(dirname(__DIR__) . '/iso-3166.json');
            $data = json_decode($json);
            foreach($data as $item) {
                $alpha3map[$item->{'alpha-3'}] = $item->name;
            }
        }

        if(!isset($alpha3map[$in])) {
            trigger_error('Missing region ' . $in, E_USER_ERROR);
        }
        return $alpha3map[$in];
    }

    private function getRawDataForThisClass() 
    {
        $class = get_class($this);
        if(!isset(self::$raw_data[$class])) {
            self::$raw_data[$class] = $this->getRawData();
        }
        return self::$raw_data[$class];
    }
}
