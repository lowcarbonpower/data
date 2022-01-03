<?php

class DataMerger
{

    private $data = [
    'ets' => [],
    'regions' => []
    ];

    private $energy_types = [];

    private $sources_by_preference = [
    'WorldBank',
    'IEA',
    'IEAYearly',
    'EIA',
    'BP',
    'Ember',
    'IEAMonthly'
    ];

    private $store;

    private $years = [];

    public function __construct(DataSeriesStore $store, $source_filter = false) 
    {
        $this->store = $store;

        $all_regions = [];

        if($source_filter && !is_array($source_filter)) {
            $source_filter = [$source_filter];
        }

        foreach($store->getMembers() as $member) {
            $energy_type = $this->getNormalizedEnergyType($member);
            if($energy_type) {
                if($source_filter) {
                    if(!in_array($member->getSource(), $source_filter)) {
                        continue;
                    }
                }

                if(!in_array($member->getSource(), $this->sources_by_preference)) {
                    trigger_error('Specify the preference for this source: ' . $member->getSource(), E_USER_ERROR);
                }

                if(!in_array($energy_type, $this->data['ets'])) {
                    if(!in_array($energy_type, ['total', 'primary'])) {
                        $this->data['ets'][] = $energy_type;
                    }
                }

                if(!isset($this->energy_types[$energy_type])) {
                    $this->energy_types[$energy_type] = [];
                }
                $this->energy_types[$energy_type][] = $member;

                if($energy_type != 'total') {
                    continue;
                }

                foreach($member->getYears() as $year) {
                    if(!in_array($year, $this->years)) {
                        if(gettype($year) != 'integer') {
                            var_dump($member->getYears());
                            trigger_error($member->getSource() . ': ' . gettype($year), E_USER_ERROR);
                        }
                        $this->years[] = $year;
                    }
                }

                foreach(array_keys($member->getRegions()) as $region) {
                    if(!in_array($region, $all_regions)) {
                        $all_regions[] = $region;
                    }
                }
            }
        }

        foreach($this->energy_types as $key => $ets) {

            foreach($ets as $et) {
                if(!is_a($et, 'DataSeriesCombination')) {
                    continue;
                }
                foreach($et->getMembers() as $member) {
                    // Exclude inverted members
                    if(is_a($member, 'DataSeriesInverted')) {
                        continue;
                    }

                    $met = $this->getNormalizedEnergyType($member);
                    if(!$met) {
                        continue;
                    }

                    // Temporary
                    if($met == 'renewables-including-hydro') {
                        continue;
                    }
                }
            }
        }

        sort($this->years);
        $this->data['years'] = $this->years;

        $this->data['diffs'] = [];

        try {
            foreach($all_regions as $region) {
                if($region[0] == '_') {
                    continue;
                }
                print "Adding $region\n";
                $this->add($region);
            }
        } catch(Exception $e) {
            trigger_error($e->getMessage(), E_USER_ERROR);
        }

        usort(
            $this->data['ets'], function ($a, $b) {
                return $this->etWeight($a) > $this->etWeight($b) ? 1 : -1;
            }
        );

        // unset Antarctica
        unset($this->data['regions']['AQ']);

        foreach($this->data['regions'] as $region => &$rd) {
            uksort(
                $rd['ets'], function ($a, $b) {
                    return $this->etWeight($a) - $this->etWeight($b);
                }
            );
        }

        // Keep ets actually in this data set but sort them in the same order as the universal ETS
        usort(
            $this->data['ets'], function ($a, $b) {
                return array_search($a, ETS) - array_search($b, ETS);
            }
        );

        foreach($this->data['regions'] as $region => &$rd) {
            $rd['leaves'] = [];
            foreach($this->data['years'] as $y => $year) {
                $rd['leaves'][$y] = $this->getLeafEts($region, $y);

                // Verify totals
                $et_total = 0;
                foreach($rd['leaves'][$y] as $et) {
                    $et_total += $rd['ets'][$et][$y];
                }
                $diff_abs = abs($et_total - $rd['ets']['total'][$y]);
                $avg_sum = ($et_total + $rd['ets']['total'][$y]) / 2;
                $diff_rel = $avg_sum ? $diff_abs / $avg_sum : 0;
                if($diff_rel > .07 && $diff_abs > 1) {
                    echo $diff_rel, PHP_EOL;
                    echo $diff_abs, PHP_EOL;
                    print_r($rd['sources']);
                    print_r($rd['leaves'][$y]);
                    trigger_error('data total ' . $rd['ets']['total'][$y] . ' differs too much from calculated total ' . $et_total . ' for region ' . $region . ' and year ' . $year . ' (' . $y . ')', E_USER_ERROR);
                }

                // Replace with et key references to make file smaller
                if($rd['leaves'][$y]) {
                    $rd['leaves'][$y] = array_map(
                        function ($et) {
                            $k = array_search($et, $this->data['ets']);
                            return $k;
                        }, $rd['leaves'][$y]
                    );
                }
            }
        }
    }

    public function getMergedData() 
    {
        // Add emissions
        $emissions = new Emissions($this->data);
        foreach($this->data['regions'] as $region => &$rd) {
            $rd['emissions'] = $emissions->get($region);
        }

        $this->setRankings();

        // If years include current years
        if(array_search(CURRENT_YEAR, $this->data['years']) !== false) {
            $this->data['current_year_months'] = IEAMonthly::$current_year_months;
        }

        return $this->data;
    }

    public function getSources() 
    {
        $sources = [];
        foreach($this->store->getMembers() as $m) {
            if(!in_array($m->getSource(), $sources)) {
                $sources[] = $m->getSource();
            }
        }
        return $sources;
    }

    public function setData($data) 
    {
        $this->data = $data;
    }

    private function energyTypeContainsChildrenWithValues($et, $ets, $rd, $y) 
    {
        if(isset(ETSS[$et])) {
            foreach(ETSS[$et] as $child) {
                if(in_array($child, $ets)) {
                    $value = $rd['ets'][$child][$y];
                    if($value) {
                        return true;
                    }
                }
                if($this->energyTypeContainsChildrenWithValues($child, $ets, $rd, $y)) {
                    return true;
                }
            }
        }
        return false;
    }

    private function getLeafEts($region, $y) 
    {
        $rd = $this->data['regions'][$region];
        $ets = array_keys($rd['ets']);

        // Remove energy types with 0 values for this year
        $ets = array_filter(
            $ets, function ($et) use ($rd, $y) {
                return $rd['ets'][$et][$y] <> 0;
            }
        );

        // Remove parent energy types if they have children with values for this year
        $ets = array_filter(
            $ets, function ($et) use ($ets, $rd, $y) {
                return !$this->energyTypeContainsChildrenWithValues($et, $ets, $rd, $y);
            }
        );

        // Remove primary energy and total
        $ets = array_filter(
            $ets, function ($et) {
                return $et != 'primary' && $et != 'total';
            }
        );
        
        // Sort these last, if available
        foreach(['other', 'trade'] as $key) {
            if(in_array($key, $ets)) {
                $ets = array_flip($ets);
                unset($ets[$key]);
                $ets = array_flip($ets);
                $ets[] = $key;
            }
        }

        $ets = array_values($ets);

        return $ets;
    }

    private function add($region) 
    {
        $this->data['regions'][$region] = [];
        $this->data['regions'][$region]['acid'] = $region;
        $this->data['regions'][$region]['ets'] = [];
        $this->data['regions'][$region]['sources'] = [];
        foreach(array_keys($this->energy_types) as $energy_type) {
            $this->data['regions'][$region]['ets'][$energy_type] = [];
        }

        foreach($this->years as $y => $year) {
            $values_by_source = [];
            foreach($this->energy_types as $energy_type => $dss) {
                foreach($dss as $ds) {
                    $regions = $ds->getRegions();
                    if(!isset($regions[$region])) {
                        if($energy_type == 'total') {
                            // This means the region only has a single source, perhaps we should record this somewhere
                            //trigger_error('missing total: ' . get_class($ds) . ', ' . $region . ', ' . $year, E_USER_ERROR);
                        }
                        continue;
                    }
                    $region_data = $regions[$region];

                    if(isset($region_data[$year])) {
                        $v = $region_data[$year];
                        if($v !== null) {
                            $values_by_source[$ds->getSource()][$energy_type] = $v;
                        }
                    }
                }
            }

            // Require "total" values
            $values_by_source = array_filter(
                $values_by_source, function ($v) {
                    return isset($v['total']) && $v['total'] > 0;
                }
            );

            foreach(array_keys($this->energy_types) as $energy_type) {
                $this->data['regions'][$region]['ets'][$energy_type][$y] = null;
            }

            $count = count($values_by_source);

            if($count == 0) {
                continue;
            }

            if($count > 1) {
                $all_ets = [];

                foreach($values_by_source as $ets) {
                    foreach(array_keys($ets) as $et) {
                        if(!in_array($et, $all_ets)) {
                            $all_ets[] = $et;
                        }
                    }
                }
                foreach($all_ets as $et) {
                    /*
                    if(in_array($et, [
                    'primary',
                    'renewables-except-hydro',
                    'unspecified-fossil-fuels'
                    ])) {
                    continue;
                    }
                    */

                    $et_values_by_source = [];
                    foreach($values_by_source as $source => $values) {
                        if(isset($values[$et])) {
                            $et_values_by_source[$source] = $values[$et];
                        }
                    }

                    // Calculate diffs
                    if(count($et_values_by_source) > 1) {
                        $divisor = (function ($et) {
                            switch($et) {
                            case 'primary':
                            case 'unspecified-fossil-fuels':
                                return false;
                            case 'total':
                                return 'primary';
                            }
                            return 'total';
                        })($et);
                        if($divisor) {
                            $et_values_by_source_pc = [];
                            foreach($et_values_by_source as $source => $value) {
                                if(isset($values_by_source[$source][$divisor]) && $values_by_source[$source][$divisor] > 0) {
                                    $et_values_by_source_pc[$source] = $value / $values_by_source[$source][$divisor];
                                }
                            }

                            if(count($et_values_by_source_pc) > 0) {
                                $avg = array_sum($et_values_by_source_pc) / count($et_values_by_source_pc);
                                if($avg == 0) {
                                    continue;
                                }
                                $min = min($et_values_by_source_pc);
                                $diff_abs = $avg - $min;
                                $diff_rel = abs($diff_abs / $avg);
                                $this->data['diffs'][] = [
                                $diff_rel,
                                $diff_abs,
                                $region,
                                $et,
                                $year,
                                $et_values_by_source,
                                $et_values_by_source_pc
                                ];
                            }
                        }
                    }
                }

                $valuable_values = array_map(
                    function ($values) {
                        return array_filter(
                            $values, function ($k) use ($values) {
                                if(isset(ETSS[$k])) {
                                    return false;
                                }
                                if(in_array($k, ETSS['fossil'])) {
                                    return false;
                                }
                                if(in_array($k, ['other', 'primary', 'unspecified-renewables'])) {
                                    return false;
                                }
                                return $values[$k] > 0;
                            }, ARRAY_FILTER_USE_KEY
                        );
                    }, $values_by_source
                );

                uksort(
                    $values_by_source, function ($a, $b) use ($valuable_values) {
                        $diff = count($valuable_values[$b]) - count($valuable_values[$a]);

                        // Same number of energy types - manually prefer certain sources
                        if($diff == 0) {
                            $diff = array_search($a, $this->sources_by_preference) - array_search($b, $this->sources_by_preference);
                        }

                        return $diff;
                    }
                );
            }

            if(0 && $region == 'W' && $year == 2020) {
                print_r($valuable_values);
                print_r($values_by_source);
                exit;
            }

            $chosen_source = array_keys($values_by_source)[0];
            $values = $values_by_source[$chosen_source];

            // Add missing energy types from other sources if available / needed
            if(count($values_by_source) > 1) {

                // If the chosen source does not include primary energy, get this value from the next available source
                if(!isset($values['primary'])) {
                    foreach($values_by_source as $source => $values2) {
                        if(isset($values2['primary'])) {
                            $values['primary'] = $values2['primary'];
                            break;
                        }
                    }
                }

                // If we have unspecified-fossil-fuels, look for alternative source with no unspecified-fossil-fuels
                if(isset($values['unspecified-fossil-fuels']) && $values['unspecified-fossil-fuels'] > 0) {
                    foreach($values_by_source as $source => $values2) {
                        if(!isset($values2['unspecified-fossil-fuels'])) {
                            $values3 = [];
                            foreach(ETSS['fossil'] as $et) {
                                if(isset($values2[$et])) {
                                    $values3[$et] = ($values2[$et] / $values2['fossil']) * $values['fossil'];
                                } else {
                                    if($et == 'unspecified-fossil-fuels') {
                                        continue;
                                    }

                                    // Original values has this energy type - datasets are too different, so ignore this one
                                    if(isset($values[$et])) {
                                        continue 2;
                                    }
                                }
                            }
                            if(array_sum($values3) > 0) {
                                foreach($values3 as $k => $v) {
                                    $values[$k] = $v;
                                }
                                unset($values['unspecified-fossil-fuels']);
                                break;
                            }
                        }
                    }
                }
            }

            $this->data['regions'][$region]['sources'][$y] = $chosen_source;

            foreach($values as $energy_type => $value) {
                $this->data['regions'][$region]['ets'][$energy_type][$y] = $value;
            }

            if(isset($values['total']) && $values['total']) {

                // fywd: first year with data
                if(!isset($this->data['regions'][$region]['fywd'])) {
                    $this->data['regions'][$region]['fywd'] = $y;
                }

                // lywd: last year with data
                $this->data['regions'][$region]['lywd'] = $y;
            }
        }

        foreach(array_reverse(ETSS) as $parent => $children) {
            $sum = null;
            foreach($children as $et) {
                if(!isset($this->data['regions'][$region]['ets'][$et])) {
                    continue;
                }

                if($sum === null) {
                    $sum = $this->data['regions'][$region]['ets'][$et];
                    continue;
                }

                foreach($this->data['regions'][$region]['ets'][$et] as $y => $v) {
                    if($v === null) {
                        continue;
                    }
                    $sum[$y] += $v;
                }
            }
            if($sum) {
                if(!isset($this->data['regions'][$region]['ets'][$parent])) {
                    $this->data['regions'][$region]['ets'][$parent] = $sum;
                    if(!in_array($parent, $this->data['ets'])) {
                        $this->data['ets'][] = $parent;
                    }
                } else {
                    foreach($sum as $y => $v) {
                        if($this->data['regions'][$region]['ets'][$parent][$y] === null) {
                            $this->data['regions'][$region]['ets'][$parent][$y] = $v;
                        }
                    }
                }
            }
        }

        foreach($this->data['regions'][$region]['ets'] as $energy_type => $years) {
            if(array_sum($years) == 0) {

                // Keep lowcarbon even if 0 (looking at you, Gibraltar)
                if($energy_type == 'lowcarbon') {
                    continue;
                }

                // Keep trade even if 0 - important to know we have trade data
                if($energy_type == 'trade') {
                    continue;
                }

                unset($this->data['regions'][$region]['ets'][$energy_type]);
            }
        }

        $total_sum = array_sum($this->data['regions'][$region]['ets']['total']);
        if(!$total_sum) {
            print "$region\n";
            print_r(array_keys($this->data['regions']));
            print_r($this->data['regions'][$region]);
            throw new Exception('Empty data for region ' . $region);
        }

        // For production
        unset($this->data['diffs']);
    }

    private function etWeight($et) 
    {
        return array_search($et, ETS);
    }

    private function getNormalizedEnergyType(DataSeries $ds) 
    {
        switch($ds->getEnergyType()) {
         // Template for quicker copy/paste
        case '':
        case '':
            return '';

        case 'Bioenergy':
        case 'Biofuels':
        case 'Biomass and waste':
        case 'Combustible Renewables':
            return 'biofuels';

        case 'Coal':
        case 'COALpc - Converted':
        case 'Coal, Peat and Manufactured Gases':
            return 'coal';

        case 'Fossil':
        case 'Fossil Fuels':
        case 'Fossil fuels':
            return 'fossil';

        case 'Gas':
        case 'Natural gas':
        case 'Natural Gas':
        case 'NGASpc - Converted':
            return 'gas';

        case 'Geothermal':
            return 'geothermal';

        case 'Geothermal And Biofuels':
        case 'Geo Biomass Other':
            return 'geothermal-and-biofuels';

        case 'Hydro':
        case 'HYROpc - Converted':
            return 'hydro';

        case 'Low-carbon':
        case 'Non-Fossil Sources':
            return 'lowcarbon';

        case 'Net imports':
        case 'Trade':
            return 'trade';

        case 'Nuclear':
        case 'NUCLpc - Converted':
            return 'nuclear';

        case 'Oil':
        case 'Oil and Petroleum Products':
        case 'PETRpc - Converted':
            return 'oil';

        case 'Other':
        case 'Not Specified':
            return 'other';

        case 'Other fossil':
        case 'Other Combustible Non-Renewables':
        case 'Unspecified Fossil Fuels':
            return 'unspecified-fossil-fuels';

        case 'Other renewables':
        case 'Other Renewables':
        case 'Unspecified renewables':
            return 'unspecified-renewables';

        case 'Primary':
        case 'Primary Energy Consumption':
        case 'Total energy consumption':
            return 'primary';

        case 'Renewable':
        case 'Renewables':
        case 'Renewables Including Hydro':
            return 'renewables-including-hydro';

        case 'Non-hydro renewable':
        case 'Renewables Except Hydro':
        case 'RNWXpc - Converted':
            return 'renewables-except-hydro';

        case 'Solar':
        case 'Solar PV':
            return 'solar';

        case 'Electricity':
            return 'total';

        case 'Wind':
            return 'wind';
        }
    }

    private function regionWeight($rd, $y) 
    {
        if(!isset($rd['ets']['total'][$y])) {
            return null;
        }
        return $rd['ets']['lowcarbon'][$y] / $rd['ets']['total'][$y];
    }

    private function setRankings() 
    {
        uasort(
            $this->data['regions'], function ($a, $b) {
                return $this->regionWeight($a, $a['lywd']) > $this->regionWeight($b, $b['lywd']) ? -1 : 1;
            }
        );
        $ranking = 1;
        foreach($this->data['regions'] as $region => &$rd) {
            $rd['rankings'] = array_fill_keys(array_keys($this->data['years']), null);
            if($region == 'W' || isset($rd['subRegions'])) {
                $rd['ranking'] = null;
                continue;
            }
            $rd['ranking'] = $ranking;
            $ranking++;
        }

        foreach($this->data['years'] as $y => $year) {
            uasort(
                $this->data['regions'], function ($a, $b) use ($y) {
                    return $this->regionWeight($a, $y) > $this->regionWeight($b, $y) ? -1 : 1;
                }
            );

            $ranking = 1;
            foreach($this->data['regions'] as $region => &$rd) {
                if($region == 'W') {
                    continue;
                }
                if(!isset($rd['ets']['total'][$y])) {
                    continue;
                }
                $rd['rankings'][$y] = $ranking;
                $ranking++;
            }
        }
    }
}
