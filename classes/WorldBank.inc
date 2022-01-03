<?php

class WorldBank extends DataSeriesFromCsvFile
{

    protected $region_row_offset = 5;

    private $is_pc = false;

    public function getEnergyType() 
    {
        $energy_type = $this->getSourceDirPart(3);
        if($this->is_pc) {
            $energy_type .= 'pc';
        }
        return $energy_type;
    }

    public function getYears() 
    {
        $years = [];
        $rows = $this->getRows();
        if(isset($rows[4])) {
            foreach($rows[4] as $k => $value) {
                if(is_numeric($value)) {
                    $years[$k] = $value;
                }
            }
        }
        return $years;
    }

    public function setIsPc() 
    {
        $this->is_pc = true;
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        if($this->is_pc) {
            return true;
        }
        if($year >= 2010) {
            return true;
        }
        if($diff_rel < 0.6) {
            return true;
        }
    }

    public function validate() 
    {
        if(!is_file($this->getSourcePath())) {
            return false;
        }

        return true;

        // What does the below do?

        $all_regions = [];
        foreach($this->getRows() as $k => $row) {
            if($k < $this->region_row_offset) {
                continue;
            }
            $region = $row[0];
            $region = $this->getNormalizedRegionName($region);
            $all_regions[] = $region;
        }
        sort($all_regions);

        $regions_and_children = [];
        foreach(file(dirname(__DIR__) . '/sources/worldbank/regions_and_children', FILE_IGNORE_NEW_LINES) as $line) {
            if(!isset($parent)) {
                $parent = $this->getNormalizedRegionName($line);
                $regions_and_children[$parent] = [];
            } elseif($line) {
                $regions_and_children[$parent][] = $this->getNormalizedRegionName($line);
            } else {
                unset($parent);
            }
        }

        $ignored_region_keys = array_fill_keys($all_regions, 0);
        unset($ignored_region_keys[self::WORLD_REGION_KEY]);

        foreach($regions_and_children as $parent => $children) {
            if(!in_array($parent, $all_regions)) {
                print_r($all_regions);
                trigger_error('Missing region parent: ' . $parent . ', source dir: ' . $source_dir, E_USER_ERROR);
            }
            foreach($children as $child) {
                if(!in_array($child, $all_regions)) {
                    print_r($all_regions);
                    trigger_error('Missing region child: ' . $child, E_USER_ERROR);
                }
                unset($ignored_region_keys[$child]);
            }
        }
        $ignored_region_keys = array_keys($ignored_region_keys);
        $this->ignored_region_keys = $ignored_region_keys;

        return true;
    }

    protected static function getNormalizedRegionName($in) 
    {
        switch($in) {
        case 'Arab League states':
        case 'Arab World':
        case 'Caribbean small states';
        case 'Central Europe and the Baltics':
        case 'China and India';
        case 'Czechoslovakia';
        case 'Early-demographic dividend';
        case 'East Asia and the Pacific (IFC classification)';
        case 'East Asia & Pacific';
        case 'East Asia & Pacific (all income levels)':
        case 'East Asia & Pacific (excluding high income)':
        case 'East Asia & Pacific (IDA & IBRD)':
        case 'East Asia & Pacific (IDA & IBRD countries)':
        case 'EASTERN EUROPE';
        case 'Euro area';
        case 'European Community';
        case 'Europe and Central Asia (IFC classification)':
        case 'European Union':
        case 'Europe & Central Asia';
        case 'Europe & Central Asia (all income levels)':
        case 'Europe & Central Asia (excluding high income)':
        case 'Europe & Central Asia (IDA & IBRD)':
        case 'Europe & Central Asia (IDA & IBRD countries)':
        case 'Fragile and conflict affected situations':
        case 'Heavily indebted poor countries (HIPC)':
        case 'High income':
        case 'High income: nonOECD':
        case 'High income: OECD':
        case 'IBRD only':
        case 'IDA blend':
        case 'IDA & IBRD total':
        case 'IDA only':
        case 'IDA total':
        case 'Late-demographic dividend':
        case 'Latin America and the Caribbean (IFC classification)';
        case 'Latin America & Caribbean':
        case 'Latin America & Caribbean (all income levels)':
        case 'Latin America & Caribbean (excluding high income)':
        case 'Latin America & Caribbean (IDA & IBRD)':
        case 'Latin America & the Caribbean (IDA & IBRD countries)':
        case 'Least developed countries: UN classification':
        case 'Lower middle income':
        case 'Low income':
        case 'Low-income Africa':
        case 'Low income, excluding China and India':
        case 'Low & middle income':
        case 'Middle East and North Africa (IFC classification)';
        case 'Middle East (developing only)':
        case 'Middle East & North Africa':
        case 'Middle East & North Africa (all income levels)':
        case 'Middle East & North Africa (excluding high income)':
        case 'Middle East & North Africa (IDA & IBRD)':
        case 'Middle East & North Africa (IDA & IBRD countries)':
        case 'Middle income':
        case 'North America':
        case 'Not classified':
        case 'OECD members':
        case 'Other low-income':
        case 'Other small states':
        case 'Pacific island small states':
        case 'Post-demographic dividend':
        case 'Pre-demographic dividend':
        case 'Severely Indebted':
        case 'Serbia and Montenegro':
        case 'Small states':
        case 'South Asia':
        case 'South Asia (IDA & IBRD)':
        case 'South Asia (IFC classification)';
        case 'Sub-Saharan Africa':
        case 'Sub-Saharan Africa (all income levels)':
        case 'SUB-SAHARAN AFRICA (excl. Nigeria)':
        case 'Sub-Saharan Africa (excluding high income)':
        case 'Sub-Saharan Africa (IDA & IBRD)':
        case 'Sub-Saharan Africa (IDA & IBRD countries)':
        case 'Sub-Saharan Africa (IFC classification)':
        case 'Upper middle income':
        case 'Virgin Islands':
        case 'Yemen, PDR':
            return self::IGNORE_REGION;
        }

        return parent::getNormalizedRegionName($in);
    }

    protected function setRegions() 
    {
        $this->region_map['Channel Islands'] = '_CI';
        $this->region_map['Kosovo'] = '_K';
        $this->region_map['Netherlands Antilles'] = '_NA';
        $this->region_map['West Bank and Gaza'] = '_WBG';

        parent::setRegions();
    }

    private function getSourceDirPart($n) 
    {
        $parts = explode('/', $this->source_dir);
        return $parts[$n];
    }
}

