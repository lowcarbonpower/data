<?php

class SandiaGlobalEnergyStorage extends DataSeriesFromCsvFile
{

    private $years, $type;

    public function __construct($years, $type) 
    {
        parent::__construct('sources/sandia/csv', 'All_Projects.csv');
        $this->years = $years;
        $this->type = $type;
        $this->setMultiplier(1 / 1000 / 1000);
    }

    public function getEnergyType() 
    {
        return 'storage' . ucwords($this->type);
    }

    public function getYears() 
    {
        return $this->years;

        // We know from the code below that the min year is 1905.
        $rows = $this->getRows(1);
        foreach($rows as $row) {
            if(isset($row[19]) && $row[19]) {
                $commisioned = strtotime($row[19]);
                if($commisioned) {
                    $year = date('Y', $commisioned);
                    if(!isset($min_year) || $year < $min_year) {
                        $min_year = $year;
                    }
                }
            }
        }
    }

    protected function setRegions() 
    {
        $years = $this->getYears();
        $rows = $this->getRows(1);
        $median_duration = $this->getMedianDuration();
        $regions = [];
        $regions['World'] = array_fill_keys($years, 0);
        foreach($rows as $row) {
            $status = $row[4];
            if($status == 'Operational' || $status == 'De-Commissioned') {
            } else {
                // Other statuses include "Offline/Under Repair".
                // We don't know when these plants went offline, so we exclude them altogether.
                // This is a problem with the data.
                continue;
            }

            $value = (int)$row[6];
            if(!$value) {
                continue;
            }
            if($this->type == 'energy') {
                $duration = (float)$row[7];

                // If missing, use median value
                if(!$duration) {
                    $duration = $median_duration;
                }

                // There are two values that seem incredibly large, replace these with medians as well
                if($duration > ($median_duration * 200)) {
                    $duration = $median_duration;
                }

                $value = $value * $duration;

                // MVh -> TWh
                $value = $value / 1000;
            }
                
            $commisioned = strtotime($row[19]);
            if($commisioned) {
                $start = date('Y', $commisioned);
                $decomissioned = strtotime($row[21]);
                $end = null;
                if($decomissioned) {
                    $end = date('Y', $decomissioned);
                }
                $row_years = array_filter(
                    $years, function ($year) use ($start, $end) {
                        if($year < $start) {
                            return false;
                        }
                        if($end && $year > $end) {
                            return false;
                        }
                        return true;
                    }
                );
                $region = trim($row[16]);
                $region = $this->getNormalizedRegionName($region);
                if(!isset($regions[$region])) {
                    $regions[$region] = array_fill_keys($years, 0);
                }
                foreach($row_years as $year) {
                    $regions[$region][$year] += $value;
                    $regions['World'][$year] += $value;
                }
            }
        }

        foreach($regions as $region => $values) {
            foreach($values as $year => $value) {
                $this->setValueByRegionName($region, $year, $value);
            }
            if($region == 'Unites States') {
                exit;
            }
        }
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        if($this->type == 'energy') {
            return $diff_rel < pow(10, -4);
        }

        return parent::testTolerance($diff_rel, $diff_abs, $year, $region);
    }

    private function getMedianDuration() 
    {
        $durations = [];
        $rows = $this->getRows(1);
        foreach($rows as $row) {
            $duration = (float)$row[7];
            if($duration) {
                $durations[] = $duration;
            }
        }
        sort($durations);
        $middle = round((count($durations) - 1) / 2);
        $median = $durations[$middle];
        return $median;
    }
}
