<?php

class DataSeriesCombination extends DataSeriesBaseClass
{

    private $energy_type;

    private $members = [];

    private $years;

    public function __construct($energy_type, $allowNegativeValues = false) 
    {
        $this->energy_type = $energy_type;
        $this->allowNegativeValues = $allowNegativeValues;
    }

    public function addMember(DataSeries $data_series) 
    {
        $this->members[$data_series->getEnergyType()] = $data_series;

        if($data_series->unset_regions_with_zero_sums == false) {
            $this->unset_regions_with_zero_sums = false;
        }

        if($data_series->verifyWorldSums == false) {
            $this->verifyWorldSums = false;
        }

        // force regions to be recalculated
        unset($this->regions);
    }

    public function getEnergyType() 
    {
        return $this->energy_type;
    }

    public function getMembers() 
    {
        return $this->members;
    }

    public function getPrimaryMember($region, $year) 
    {
        foreach($this->getMembers() as $member) {
            if(!isset($pm) || $member->getValue($region, $year) > $pm->getValue($region, $year)) {
                $pm = $member;
            }
        }
        if(!isset($pm)) {
            throw new Exception('No members?');
        }
        return $pm;
    }

    public function getSource() 
    {
        return current($this->members)->getSource();
    }

    public function getYears() 
    {
        if(!isset($this->years)) {
            $this->years = [];
            foreach($this->getMembers() as $data_series) {
                foreach($data_series->getYears() as $year) {
                    if(!in_array($year, $this->years)) {
                        $this->years[] = $year;
                    }
                }
            }
            sort($this->years);
        }
        return $this->years;
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        foreach($this->getMembers() as $member) {
            if(!$member->testTolerance($diff_rel, $diff_abs, $year, $region)) {
                return false;
            }
        }
        return true;
    }

    protected function setRegions() 
    {
        $regions = [];
        foreach($this->getMembers() as $data_series) {
            foreach($data_series->getRegions() as $region => $years) {
                if(!isset($regions[$region])) {
                    $regions[$region] = array_fill_keys($this->getYears(), null);
                }
                foreach($this->getYears() as $year) {
                    if(isset($years[$year])) {
                        $value = $years[$year];
                        if($value !== null) {
                            if($regions[$region][$year] === null) {
                                $regions[$region][$year] = 0;
                            }
                            $regions[$region][$year] += $value;
                        }
                    }
                }
            }
        }

        if(!$this->allowNegativeValues) {
            foreach($regions as &$years) {
                foreach($years as &$value) {
                    if($value < 0) {
                        $value = 0;
                    }
                }
            }
        }

        ksort($regions);
        $this->regions = $regions;
    }
}

