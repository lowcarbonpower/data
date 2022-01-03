<?php

class BPFossilFuelMinusUnspecifiedFossilFuels extends DataSeriesClone
{

    private $bpu;

    function __construct($source, $bpu) 
    {
        parent::__construct($source);
        $this->bpu = $bpu;
        $regions = $this->getRegions();
        foreach($this->bpu->getRegions() as $region => $years) {
            // Skip World region. We are deducting all other region values from the current world total below.
            if($region == 'W') {
                continue;
            }
            foreach($years as $year => $value) {
                if($value && isset($regions[$region][$year])) {
                    $regions['W'][$year] -= $regions[$region][$year];
                    $regions[$region][$year] = 0;
                }
            }
        }
        $this->regions = $regions;
    }
}
