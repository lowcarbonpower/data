<?php

class WorldBankFromTotalAndPercentages extends WorldBank
{

    private $total, $pc;
    private $years = [];

    public function __construct($key, $total, $pc) 
    {
        $this->total = $total;
        $this->pc = $pc;
    }

    public function getEnergyType() 
    {
        return $this->pc->getEnergyType() . ' - Converted';
    }

    public function getSource() 
    {
        return $this->total->getSource();
    }

    public function getYears() 
    {
        return $this->years;
    }

    protected function getWorldSumDiffTolerance() 
    {
        return .7;
    }

    // For each region/year, if both total and pc value are available, calculate absolute values
    protected function setRegions() 
    {
        $total_regions = $this->total->getRegions();
        $pc_regions = $this->pc->getRegions();
        foreach($pc_regions as $region => $years) {
            foreach($years as $year => $pc_value) {
                if($pc_value) {
                    $total_value = $this->total->getValue($region, $year);
                    if($total_value) {
                        if(!isset($this->regions[$region])) {
                            $this->regions[$region] = [];
                        }
                        if(!in_array($year, $this->years)) {
                            $this->years[] = $year;
                        }
                        $this->regions[$region][$year] = ($pc_value / 100) * $total_value;
                    }
                }
            }
        }
        sort($this->years);
    }
}
