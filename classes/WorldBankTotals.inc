<?php

// TODO: Compare totals from each energy source as well as total from BP, per year, to decide what factors make data more reliable (such as energy type, or percentage value, ie if percentage is very low it may be less reliable)
class WorldBankTotals extends WorldBank
{

    private $bp_total_regions;
    private $diff_tolerance;
    private $member_regions = [];
    private $percentage_totals;
    private $source;
    private $years = [];

    public function __construct($bp_total) 
    {
        $this->bp_total_regions = $bp_total->getRegions();
        $this->diff_tolerance = .08;
        $this->percentage_totals = new DataSeriesCombination('ZS');
    }

    public function addMember($type, $unit, DataSeries $data_series) 
    {
        if(!isset($this->member_regions[$type])) {
            $this->member_regions[$type] = [];
        }
        $this->member_regions[$type][$unit] = $data_series->getRegions();

        $this->source = $data_series->getSource();

        if($type != 'FOSL' && $type != 'RNEW') {
            if($unit == 'ZS') {
                $this->percentage_totals->addMember($data_series);
            }
        }

        // force regions to be recalculated
        unset($this->regions);
    }

    public function getEnergyType() 
    {
        return 'Total';
    }

    public function getSource() 
    {
        return $this->source;
    }

    public function getYears() 
    {
        $this->setRegions();
        sort($this->years);
        return $this->years;
    }

    protected function getWorldSumDiffTolerance() 
    {
        return .6;
    }

    protected function setRegions() 
    {
        $regions = [];

        $pc_regions = $this->percentage_totals->getRegions();
        foreach($pc_regions as $region => $years) {
            foreach($years as $year => $value) {

                // Only include data where percentage totals add up to 100 (+/- 2), also manually include World even if it doesn't add up
                if(abs($value - 100) < 2 || $region == 'W') {
                    foreach($this->member_regions as $type => $members) {
                        if($type != 'RNWX') {
                            continue;
                        }
                        if(!isset($members['KH'])) {
                            continue;
                        }
                        if(!isset($members['ZS'])) {
                            continue;
                        }
                        $absolute_regions = $members['KH'];
                        $percentage_regions = $members['ZS'];

                        // Check if World total exists for this year, otherwise skip it (for now at least)
                        if(!$percentage_regions['W'][$year]) {
                            continue;
                        }

                        if(isset($absolute_regions[$region]) && isset($percentage_regions[$region])) {
                            $absolute = $absolute_regions[$region][$year];
                            $percentage = $percentage_regions[$region][$year];
                            if($absolute && $percentage) {
                                $total = $absolute / ($percentage / 100);
                                if(!isset($regions[$region])) {
                                    $regions[$region] = [];
                                }
                                $regions[$region][$year] = $total;

                                if(!in_array($year, $this->years)) {
                                    $this->years[] = $year;
                                }
                            }
                        }
                    }
                } else {
                    //trigger_error('Skipping region ' . $region . ' with pc total ' . $value);
                }
            }
        }

        $this->regions = $regions;
    }
}
