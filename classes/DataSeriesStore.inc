<?php

class DataSeriesStore implements DiffTolerance
{

    private $members = [];

    public function __construct() 
    {
    }

    public function addCombination($name, $keys, $allowNegativeValues = false) 
    {
        $c = $this->getCombination($name, $keys, $allowNegativeValues);
        $key = $this->addMember($c);
        $this->requireThatCombinationsMatch($this, [$key], $keys, $allowNegativeValues);
        return $c;
    }

    public function addMember(DataSeries $data_series, $replace = false) 
    {
        $key = implode(' - ', [$data_series->getSource(), $data_series->getEnergyType()]);
        if($data_series->isEmpty()) {
            return;
        }
        if(isset($this->members[$key]) && !$replace) {
            trigger_error('Duplicate store key - ' . $key, E_USER_ERROR);
        }
        $this->members[$key] = $data_series;
        return $key;
    }

    public function getCombination($name, $keys, $allowNegativeValues = false) 
    {
        $c = new DataSeriesCombination($name, $allowNegativeValues);
        foreach($keys as $key) {
            $c->addMember($this->getMember($key));
        }
        return $c;
    }

    public function getMember($key) 
    {
        if(!isset($this->members[$key])) {
            print_r(array_keys($this->members));
            trigger_error('Unknown store key: [' . $key . ']', E_USER_ERROR);
        }
        return $this->members[$key];
    }

    public function getMembers() 
    {
        return $this->members;
    }

    public function getRegions() 
    {
        $regions = [];
        foreach($this->getMembers() as $data_series) {
            foreach(array_keys($data_series->getRegions()) as $region) {
                if(!in_array($region, $regions)) {
                    $regions[] = $region;
                }
            }
        }
        sort($regions);
        return $regions;
    }

    public function getYears() 
    {
        static $years;
        if(!isset($years)) {
            $years = [];
            foreach($this->getMembers() as $data_series) {
                foreach($data_series->getYears() as $year) {
                    if(!in_array($year, $years)) {
                        $years[] = $year;
                    }
                }
            }
            sort($years);
        }
        return $years;
    }

    public function print() 
    {
        foreach($this->getMembers() as $member) {
            echo $member->getEnergyType(), PHP_EOL;
        }
        exit;
    }

    public function requireThatCombinationsMatch(DiffTolerance $dt, $a, $b, $allowNegativeValues = false) 
    {
        $sets = [];
        foreach([$a, $b] as $i => $keys) {
            $key = implode('-', $keys);
            $c = $this->getCombination($i, $keys, $allowNegativeValues);
            $sets[$key] = $c->getRegions();
        }

        requireThatRegionsMatch($sets, $dt);
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        return false;
    }
}

