<?php

// TODO:
// - Simlify same way as DataMerger
class TradeDataMerger
{

    private $data = [
    ];

    private $store;

    private $years = [];

    const DIFF_TOLERANCE_ABS = 4;
    const DIFF_TOLERANCE_REL = 0.01;

    // New data strucute, so let's get rid of $regions_with_single_source and $region_alt_names - will simplify this a lot
    public function __construct(DataSeriesStore $store) 
    {
        $this->store = $store;

        $all_regions = [];

        foreach($store->getMembers() as $member) {
            foreach($member->getYears() as $year) {
                if(!in_array($year, $this->years)) {
                    $this->years[] = $year;
                }
            }

            foreach(array_keys($member->getRegions()) as $region) {
                if(!in_array($region, $all_regions)) {
                    $all_regions[] = $region;
                }
            }
        }

        sort($this->years);
        $this->data['years'] = $this->years;

        try {
            foreach($all_regions as $region) {
                print "Adding $region\n";
                $this->add($region);
            }
        } catch(Exception $e) {
            trigger_error($e->getMessage());
        }
    }

    public function getData() 
    {
        return $this->data;
    }

    private function add($region) 
    {
        $this->data['regions'][$region] = [];
        $this->data['regions'][$region]['values'] = [];
        $this->data['regions'][$region]['sources'] = [];

        foreach($this->years as $y => $year) {
            $values_by_source = [];
            foreach($this->store->getMembers() as $member) {
                $regions = $member->getRegions();
                if(!isset($regions[$region])) {
                    continue;
                }
                $region_data = $regions[$region];

                if(isset($region_data[$year])) {
                    $v = $region_data[$year];
                    $values_by_source[$member->getSource()] = $v;
                }
            }

            $count = count($values_by_source);

            if($count == 0) {
                continue;
            }

            // If we have multiple sources which include Ember, remove Ember.
            // I suspect their data is less correct, plus they are just aggregating other sources anyway,
            // better to use an original source.
            if($count > 1 && isset($values_by_source['Ember'])) {
                unset($values_by_source['Ember']);
            }
            $count = count($values_by_source);

            // If we have multiple sources which include IEAMonthly, remove IEAMonthly.
            // I suspect their data is less correct. For example, they report 0 trade for Brazila and Argentina.
            if($count > 1 && isset($values_by_source['IEAMonthly'])) {
                unset($values_by_source['IEAMonthly']);
            }
            $count = count($values_by_source);

            if($count > 1) {
                if($region == 'W') {
                    // World trade totals are not important
                } else {
                    $avg = array_sum($values_by_source) / count($values_by_source);
                    if($avg) {
                        foreach($values_by_source as $source => $value) {
                            $diff = abs($value - $avg);
                            if($diff > self::DIFF_TOLERANCE_ABS) {
                                $diff_relative = $diff / $avg;
                                if($diff_relative > self::DIFF_TOLERANCE_REL) {
                                    print_r($values_by_source);
                                    trigger_error('Diff ' . $diff . ' greater than DIFF_TOLERANCE_ABS ' . self::DIFF_TOLERANCE_ABS . ' for region ' . $region . ' and year ' . $year);
                                    trigger_error('Relative diff ' . $diff_relative . ' greater than DIFF_TOLERANCE_REL ' . self::DIFF_TOLERANCE_REL . ' for region ' . $region . ' and year ' . $year);
                                    $handle = fopen('php://stdin', 'r');
                                    $line = fgets($handle);
                                }
                            }
                        }
                    }
                }

                // I guess this just prefers more "exact" numbers
                uasort(
                    $values_by_source, function ($a, $b) {
                        return strlen($a) > strlen($b) ? -1 : 1;
                    }
                );
            }

            $chosen_source = array_keys($values_by_source)[0];
            $value = $values_by_source[$chosen_source];

            $this->data['regions'][$region]['sources'][$y] = $chosen_source;
            $this->data['regions'][$region]['values'][$y] = $value;
        }
    }
}
