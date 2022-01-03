<?php

class BP extends DataSeriesFromCsvFile
{

    protected $allowNegativeValues = true;

    public function getEnergyType() 
    {
        $unit = $this->getFieldValue(0, 0);
        if($unit != 'Terawatt-hours') {

            // Primary_Energy_Consumption unit is Exajoules
            if($this->source_file == 'Primary_Energy_Consumption.csv') {
                $this->setMultiplier(277.777778);
            }
            
            // Ignore others
            else {
                return;
            }
        }

        $energy_type = basename($this->source_file, '.csv');
        $energy_type = str_replace('_', ' ', $energy_type);

        // Actual error in source!?
        $energy_type = str_replace('Wind Generation - EJ', 'Wind Generation', $energy_type);

        $energy_type = str_replace('Elec Gen from ', '', $energy_type);
        $energy_type = str_replace(' Generation', '', $energy_type);
        $energy_type = str_replace(' Power', '', $energy_type);
        $energy_type = str_replace(' power', '', $energy_type);
        $energy_type = str_replace(' - TWh', ' ', $energy_type);
        $energy_type = str_replace(' - Twh', ' ', $energy_type);
        $energy_type = trim($energy_type);

        // Ignore these types
        switch($energy_type) {
        case 'Elec Gen by fuel':
        case 'Renewables by source':
            return;
        }

        return $energy_type;
    }

    public function getYears() 
    {
        $years = [];
        foreach($this->getRows()[0] as $k => $value) {
            if(is_numeric($value)) {

                // Ignore years before 1985 for BP data since it's incomplete
                if($value >= 1985) {
                    $years[$k] = (int)$value;
                }
            }
        }
        return $years;
    }

    protected static function getNormalizedRegionName($in) 
    {
        switch($in) {
        case 'Total World':
            return 'World';
        case 'USSR':
            return self::IGNORE_REGION;
        }

        if(substr($in, 0, 6) == 'Total ') {
            return self::IGNORE_REGION;
        }

        $out = parent::getNormalizedRegionName($in);
        return $out;
    }

    protected function setRegions() 
    {
        $this->region_map['Central America'] = '_CAM';
        $this->region_map['Eastern Africa'] = '_EAF';
        $this->region_map['Middle Africa'] = '_MAF';
        $this->region_map['Other Africa'] = '_OAF';
        $this->region_map['Other Asia Pacific'] = '_OAP';
        $this->region_map['Other Caribbean'] = '_OCA';
        $this->region_map['Other CIS'] = '_OCIS';
        $this->region_map['Other Europe'] = '_OE';
        $this->region_map['Other Middle East'] = '_OME';
        $this->region_map['Other North Africa'] = '_ONAF1';
        $this->region_map['Other Northern Africa'] = '_ONAF2';
        $this->region_map['Other South America'] = '_OSAM';
        $this->region_map['Other S. & Cent. America'] = '_OSCAM';
        $this->region_map['Other Southern Africa'] = '_OSAF';
        $this->region_map['Western Africa'] = '_WAF';

        parent::setRegions();
    }
}

