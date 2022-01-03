<?php

require 'functions.inc';
require 'vars.inc';
define('LANG', 'en');

// No translations needed in data sripts
function t($str) 
{
    return $str;
}

set_error_handler(
    function ($errno, $errstr, $errfile, $errline) {
        $args = func_get_args();
        $args = array_slice($args, 0, 4);
        if($args[2] == '/usr/share/php/Services/JSON.php') {
            return;
        }
        echo "ERROR:\n";
        print_r($args);
        exit(1);
    }
);

function scandir_($dir) 
{
    $contents = [];
    foreach(scandir($dir) as $x) {
        if($x[0] == '.') {
            continue;
        }
        $contents[] = $x;
    }
    return $contents;
}

foreach(scandir_('classes') as $file) {
    include 'classes/' . $file;
}

// Regular electricity data store
$store = new DataSeriesStore();

/*
    IEA Monthly
*/
$IEAM = new IEAMonthly;
foreach($IEAM->getEnergyTypes() as $et) {
    $ieam = new IEAMonthly($et);
    $store->addMember($ieam);
}

$dt = new class implements DiffTolerance {
    function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        if($diff_abs < 0.4) {
            return true;
        }

        if($diff_rel < 0.05) {
            return true;
        }
    }
};

$store->requireThatCombinationsMatch(
    $dt, ['IEAMonthly - Total Renewables (Geo, Solar, Wind, Other)'], [
    'IEAMonthly - Combustible Renewables',
    'IEAMonthly - Hydro',
    'IEAMonthly - Wind',
    'IEAMonthly - Solar',
    'IEAMonthly - Geothermal',
    'IEAMonthly - Other Renewables'
    ]
);

$store->addCombination(
    'Non-Fossil Sources', [
    'IEAMonthly - Nuclear', 
    'IEAMonthly - Total Renewables (Geo, Solar, Wind, Other)'
    ]
);
$store->addCombination(
    'Fossil Fuels', [
    'IEAMonthly - Coal, Peat and Manufactured Gases',
    'IEAMonthly - Oil and Petroleum Products',
    'IEAMonthly - Natural Gas',
    'IEAMonthly - Other Combustible Non-Renewables'
    ]
);

$store->requireThatCombinationsMatch(
    $dt, ['IEAMonthly - Net Electricity Production'], [
    'IEAMonthly - Non-Fossil Sources',
    'IEAMonthly - Fossil Fuels',
    'IEAMonthly - Not Specified'
    ]
);

$ieam_exports_inverted = new DataSeriesInverted($store->getMember('IEAMonthly - Total Exports'));
$ieam_trade = new DataSeriesCombination('Trade');
$ieam_trade->addMember($ieam_exports_inverted);
$ieam_trade->addMember($store->getMember('IEAMonthly - Total Imports'));
$store->addMember($ieam_trade);
$store->addCombination('Electricity', ['IEAMonthly - Net Electricity Production', 'IEAMonthly - Trade']);

/*
    Ember
*/
$E = new Ember;
foreach($E->getEnergyTypes() as $et) {
    $e = new Ember($et);
    $e->getRegions();
    $store->addMember($e);
}

$ember_dt = new class implements DiffTolerance {
    function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        if($diff_rel < pow(10, -10)) {
            return true;
        }

        // Data is worse for 2020
        if($year == 2020 && $diff_rel < pow(10, -3)) {
            return true;
        }
    }
};
$store->requireThatCombinationsMatch($ember_dt, ['Ember - Fossil'], ['Ember - Coal', 'Ember - Gas', 'Ember - Other fossil']);
$store->requireThatCombinationsMatch($ember_dt, ['Ember - Renewables'], ['Ember - Hydro', 'Ember - Wind', 'Ember - Solar', 'Ember - Bioenergy', 'Ember - Other renewables']);
$store->addCombination('Non-Fossil Sources', ['Ember - Nuclear', 'Ember - Renewables']);
$store->requireThatCombinationsMatch($ember_dt, ['Ember - Production'], ['Ember - Non-Fossil Sources', 'Ember - Fossil']);

$total = new class('Electricity') extends DataSeriesCombination {
    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        return true;
    }
};
$total->addMember($store->getMember('Ember - Production'));
$total->addMember($store->getMember('Ember - Net imports'));
$store->addMember($total);

/*
    EIA
*/
$eiap = new EIA('Total energy consumption', 'sources/eia/primary');
$store->addMember($eiap);

$eia0 = new EIA;
foreach($eia0->getEnergyTypes() as $et) {
    $eia = new EIA($et);
    $store->addMember($eia);
}

$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        public function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_abs <= 1.5) {
                return true;
            }
        }
    }, ['EIA - Generation'], ['EIA - Fossil fuels', 'EIA - Nuclear', 'EIA - Renewable', 'EIA - Hydroelectric pumped storage']
);

$eia_pumped_storage_inverted = new class extends DataSeriesInverted {
    public function __construct() 
    {
        global $store;
        parent::__construct($store->getMember('EIA - Hydroelectric pumped storage'));
    }
};

$eia_total = new class extends DataSeriesCombination {
    public function __construct() 
    {
        parent::__construct('Electricity');
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        global $eia0;
        return $eia0->testTolerance($diff_rel, $diff_abs, $year, $region);
    }
};

$eia_total->addMember($store->getMember('EIA - Generation'));
$eia_total->addMember($eia_pumped_storage_inverted);
$store->addMember($eia_total);

$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        public function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_abs <= 1.5) {
                return true;
            }
        }
    }, ['EIA - Electricity'], ['EIA - Fossil fuels', 'EIA - Nuclear', 'EIA - Renewable']
);

$eia_h = $store->getMember('EIA - Hydro');
$eia_hi = new class extends DataSeriesInverted {
    public function __construct() 
    {
        global $eia_h;
        parent::__construct($eia_h);
    }
};
$eia_nhr = $store->getMember('EIA - Non-hydro renewable');
$eia_nhri = new class extends DataSeriesInverted {
    public function __construct() 
    {
        global $eia_nhr;
        parent::__construct($eia_nhr);
    }
};
$eia_ur = new class extends DataSeriesCombination {
    public function __construct() 
    {
        parent::__construct('Unspecified renewables');
        $this->allowNegativeValues = true;
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        if($diff_abs < 9) {
            return true;
        }
        global $eia0;
        return $eia0->testTolerance($diff_rel, $diff_abs, $year, $region);
    }
};
$eia_ur->addMember($eia_hi);
$eia_ur->addMember($eia_nhri);
$eia_ur->addMember($store->getMember('EIA - Renewable'));
$store->addMember($eia_ur);
$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        public function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_abs <= 0) {
                return true;
            }
            global $eia0;
            return $eia0->testTolerance($diff_rel, $diff_abs, $year, $region);
        }
    }, ['EIA - Renewable'], ['EIA - Hydro', 'EIA - Non-hydro renewable', 'EIA - Unspecified renewables']
);
$store->addCombination('Non-Fossil Sources', ['EIA - Nuclear', 'EIA - Renewable']);
$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_rel < 0.03) {
                return true;
            }
            global $eia0;
            return $eia0->testTolerance($diff_rel, $diff_abs, $year, $region);
        }
    }, ['EIA - Generation'], ['EIA - Non-Fossil Sources', 'EIA - Fossil fuels']
);

// Just make a copy of fossil fuels to "Unspecified Fossil Fuels" so that DataMerger will include this data.
$store->addCombination('Unspecified Fossil Fuels', ['EIA - Fossil fuels']);

/*
    IEA Yearly
*/
$iea = new IEAYearly('Total production');
$store->addMember($iea);

$store->addMember(new IEAPrimary());

// Compare production and consumption. Very high diff tolerance (should be due to trade).
$store->addMember(new IEAYearly('Total final consumption'));
$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_rel < .4) {
                return true;
            }
            if($diff_abs < 100) {
                return true;
            }
            global $iea;
            return $iea->testTolerance($diff_rel, $diff_abs, $year, $region);
        }
    }, ['IEAYearly - Total production'], ['IEAYearly - Total final consumption']
);

$groups = [
    'Fossil Fuels' => [
        'Coal',
        'Oil',
        'Natural gas'
    ],
    'Renewables Except Hydro' => [
        'Biofuels',
        'Geothermal',
        'Solar PV',
        'Wind'
    ],
    'Non-Fossil Sources' => [
        'Nuclear',
        'Hydro'
    ],
    'Other' => [
        'Other sources'
    ]
];
foreach($groups as $group => $types) {
    $comb = new DataSeriesCombination($group);
    foreach($types as $type) {
        $iea = new IEAYearly($type);
        $iea->getRegions();
        $store->addMember($iea);
        $comb->addMember($iea);
    }
    $store->addMember($comb);
}

$store->getMember('IEAYearly - Non-Fossil Sources')->addMember(
    $store->getMember('IEAYearly - Renewables Except Hydro')
);

$store->addCombination('Geothermal And Biofuels', ['IEAYearly - Biofuels', 'IEAYearly - Geothermal']);
$store->addCombination('Renewables Including Hydro', ['IEAYearly - Renewables Except Hydro', 'IEAYearly - Hydro']);
$store->addCombination('Our Calculated Total', ['IEAYearly - Fossil Fuels', 'IEAYearly - Non-Fossil Sources', 'IEAYearly - Other']);

$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_rel < .01) {
                return true;
            }
            if($diff_abs < 2) {
                return true;
            }
            global $iea;
            return $iea->testTolerance($diff_rel, $diff_abs, $year, $region);
        }
    }, ['IEAYearly - Our Calculated Total'], ['IEAYearly - Total production']
);

$ieat = new IEATrade();
$store->addMember($ieat);
$store->addCombination('Electricity', ['IEAYearly - Total production', 'IEAYearly - Trade']);

/*
    BP
*/
$dir = 'sources/bp/csv';
foreach(scandir_($dir) as $file) {
    $bp = new BP($dir, $file);
    $store->addMember($bp);
}

// Just for verification
$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_rel < pow(10, -10)) {
                return true;
            }
        }
    }, ['BP - Renewables'], [
    'BP - Geo Biomass Other',
    'BP - Solar',
    'BP - Wind'
    ]
);

$store->addCombination(
    'Renewables Except Hydro', [
    'BP - Geo Biomass Other',
    'BP - Solar',
    'BP - Wind'
    ]
);

$bp_coal_gas_oil = $store->addCombination('Coal / Gas / Oil', ['BP - Coal', 'BP - Gas', 'BP - Oil']);
$bp_non_fossil_sources = $store->addCombination('Non-Fossil Sources', ['BP - Hydro', 'BP - Nuclear', 'BP - Renewables']);
$bp_non_fossil_plus_other = $store->addCombination('Non-Fossil Sources / Other', ['BP - Non-Fossil Sources', 'BP - Other'], true); // Other can be negative!?

$bpu = new BPUnspecifiedFossilFuels($store->getMember('BP - Electricity'), $bp_coal_gas_oil, $bp_non_fossil_plus_other);
$store->addMember($bpu);

$store->addCombination('Renewables Including Hydro', ['BP - Renewables', 'BP - Hydro']);

foreach($bp_coal_gas_oil->getMembers() as $member) {
    $store->addMember(new BPFossilFuelMinusUnspecifiedFossilFuels($member, $bpu), true);
}

$bp_fossil_fuels = $store->addCombination('Fossil Fuels', ['BP - Coal', 'BP - Gas', 'BP - Oil', 'BP - Unspecified Fossil Fuels'], true); // Negative values in here somewhere!?

$store->addCombination('Our Calculated Total', ['BP - Fossil Fuels', 'BP - Non-Fossil Sources', 'BP - Other']);

$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_rel < pow(10, -7)) {
                return true;
            }
        }
    }, ['BP - Our Calculated Total'], ['BP - Electricity']
);

/*
    World Bank
*/
$wb_total = new WorldBankTotals($store->getMember('BP - Electricity'));

$wb_percentages = [];
$dir1 = 'sources/worldbank/csv';
foreach(scandir_($dir1) as $dir2) {
    foreach(scandir_($dir1 . '/' . $dir2) as $dir3) {
        $wb = new WorldBank($dir1 . '/' . $dir2 . '/' . $dir3, 'data.csv');

        if(!$wb->validate()) {
            continue;
        }

        if($dir3 == 'ZS') {
            $wb->setIsPc();
            $wb_percentages[$dir2] = $wb;
        } else {
            // kWh => TWh
            $wb->setMultiplier(pow(10, -9));
        }

        $wb_total->addMember($dir2, $dir3, $wb);
        $store->addMember($wb);
    }
}

foreach($wb_percentages as $key => $wb_pc) {
    $wb_abs = new WorldBankFromTotalAndPercentages($key, $wb_total, $wb_pc);
    $store->addMember($wb_abs);
}

$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_rel < pow(10, -12)) {
                return true;
            }
        }
    }, ['WorldBank - FOSLpc'], ['WorldBank - COALpc', 'WorldBank - NGASpc', 'WorldBank - PETRpc']
);

$store->addMember($wb_total);

$store->addCombination('Fossil Fuels', ['WorldBank - COALpc - Converted', 'WorldBank - NGASpc - Converted', 'WorldBank - PETRpc - Converted']);

$store->addCombination('Non-Fossil Sources', ['WorldBank - HYROpc - Converted', 'WorldBank - RNWXpc - Converted', 'WorldBank - NUCLpc - Converted']);
$total = $store->addCombination('Electricity', ['WorldBank - Fossil Fuels', 'WorldBank - Non-Fossil Sources']);

$store->addCombination('Renewables Including Hydro', ['WorldBank - HYROpc - Converted', 'WorldBank - RNWXpc - Converted']);

$store->requireThatCombinationsMatch(
    new class implements DiffTolerance {
        function testTolerance($diff_rel, $diff_abs, $year, $region) 
        {
            if($diff_rel < .2) {
                return true;
            }
        }
    }, ['WorldBank - Total'], ['WorldBank - Electricity']
);

foreach($store->getMembers() as $data_series) {
    print implode(
        "\t", [
        $data_series->getSource(),
        get_class($data_series),
        $data_series->getEnergyType(), 
        $data_series->getFirstYear(), 
        $data_series->getLastYear(), 
        is_subclass_of($data_series, 'DataSeriesFromCsvFile') ? $data_series->getSourcePath() : ''
        ]
    );
    print "\n";
}

// Get merged data
$dm = new DataMerger($store);
$data = $dm->getMergedData();

// Get years (except current year, if not complete)
$years = $data['years'];
define('CURRENT_Y', array_search(CURRENT_YEAR, $data['years'], true));
if(isset($years[CURRENT_Y]) && $data['current_year_months'] < 12) {
    unset($years[CURRENT_Y]);
}

// Create energy storage data for the same years
$storage = [];
$s_power = new SandiaGlobalEnergyStorage($years, 'power');
foreach($s_power->getRegions() as $region => $values) {
    if(isset($data['regions'][$region])) {
        //$data['regions'][$region][$s_power->getEnergyType()] = array_values($values);
        if(!isset($storage[$region])) {
            $storage[$region] = [];
        }
        $storage[$region][$s_power->getEnergyType()] = array_values($values);
    }
}
$s_energy = new SandiaGlobalEnergyStorage($years, 'energy');
foreach($s_energy->getRegions() as $region => $values) {
    if(isset($data['regions'][$region])) {
        //$data['regions'][$region][$s_energy->getEnergyType()] = array_values($values);
        if(!isset($storage[$region])) {
            $storage[$region] = [];
        }
        $storage[$region][$s_energy->getEnergyType()] = array_values($values);
    }
}
file_put_contents('storage.json', json_encode($storage));

function csv_file_put_contents($filename, $data) 
{
    $rows = [];

    $row = array_values($data['years']);
    array_unshift($row, '');
    array_unshift($row, '');
    array_unshift($row, '');
    $rows[] = $row;

    foreach($data['regions'] as $region => $rd) {
        foreach($rd['ets'] as $et => $years) {
            $vals = [];
            $vals[] = $region;
            $vals[] = prn($region);
            $vals[] = $et;
            foreach($years as $v) {
                $vals[] = $v;
            }
            $rows[] = $vals;
        }
    }

    $fp = fopen($filename, 'w');
    foreach($rows as $row) {
        fputcsv($fp, $row);
    }
    fclose($fp);
}

// Region groups
$REGION_GROUPS = REGION_GROUPS;

// Dynamic region groups based on energy type thresholds
foreach([.12, .24] as $threshold) {
    foreach([
    [
    ['nuclear'],
    ['wind-and-solar']
    ],
    [
    ['wind-and-solar'],
    ['nuclear']
    ],
    [
    ['nuclear', 'wind-and-solar']
    ]
    ] as $conditions) {
        $conditions_text = array_map(
            function ($conditon) use ($threshold) {
                return $conditon . '>' . $threshold;
            }, $conditions[0]
        );
        $key = implode($conditions_text, ' and ');

        if(isset($conditions[1])) {
            $threshold2 = $threshold / 2;
            $conditions_text = array_map(
                function ($conditon) use ($threshold2) {
                    return $conditon . '<' . $threshold2;
                }, $conditions[1]
            );
            $key .= ', ' . implode($conditions_text, ' and ');
        }

        $sub_regions = array_filter(
            array_keys($data['regions']), function ($region) use ($data, $conditions, $threshold) {
                if($region == 'W') {
                    return false;
                }

                $rd = $data['regions'][$region];
                $lywd = $rd['lywd'];

                // Must not have these
                if(isset($conditions[1])) {
                    foreach($conditions[1] as $et) {
                        if(isset($rd['ets'][$et])) {
                            if($rd['ets'][$et][$lywd] / $rd['ets']['total'][$lywd] > $threshold / 2) {
                                return false;
                            }
                        }
                    }
                }

                // Must have these
                foreach($conditions[0] as $et) {
                    if(isset($rd['ets'][$et])) {
                        if($rd['ets'][$et][$lywd] / $rd['ets']['total'][$lywd] > $threshold) {
                            continue;
                        }
                    }

                    return false;
                }

                // If no recent data, and total generation for this region is very low, then ignore
                if($lywd < CURRENT_Y) {
                    if($rd['ets']['total'][$lywd] < 134) {
                        return false;
                    }
                }

                // Manually exclude these because they lack recent trade data
                if(in_array($region, ['MA'])) {
                    return false;
                }

                return true;
            }
        );

        if($sub_regions) {
            $REGION_GROUPS[$key] = $sub_regions;
        }
    }
}

foreach($REGION_GROUPS as $parent => $children) {
    $rd = null;
    foreach($children as $child) {
        if(!isset($data['regions'][$child])) {
            trigger_error('missing region child: ' . $child, E_USER_ERROR);
        }
        if(!$rd) {
            $rd = $data['regions'][$child];
        } else {
            $rd2 = $data['regions'][$child];

            if($rd2['fywd'] > $rd['fywd']) {
                $rd['fywd'] = $rd2['fywd'];
            }
            if($rd2['lywd'] < $rd['lywd']) {
                $rd['lywd'] = $rd2['lywd'];
            }

            foreach($rd2['ets'] as $et => $years) {
                if(!isset($rd['ets'][$et])) {
                    $rd['ets'][$et] = array_fill_keys(array_keys($data['years']), null);
                }

                for($y = $rd2['fywd']; $y <= $rd2['lywd']; $y++) {
                    $v = $years[$y];

                    // require primary energy values for all years, or set to null
                    if($et == 'primary') {
                        if($v === null) {
                            $rd['ets'][$et][$y] = null;
                        }

                        if($rd['ets'][$et][$y] === null) {
                            continue;
                        }
                    }

                    if(!$v) {
                        continue;
                    }
                    if($rd['ets'][$et][$y] === null) {
                        $rd['ets'][$et][$y] = 0;
                    }
                    $rd['ets'][$et][$y] += $v;
                }
            }
            foreach($data['regions'][$child]['leaves'] as $y => $leaves) {
                foreach($leaves as $leaf) {
                    if(!in_array($leaf, $rd['leaves'][$y])) {
                        $rd['leaves'][$y][] = $leaf;
                    }
                }
                sort($rd['leaves'][$y]);
                unset($leaves);
            }
        }
    }
    unset($v);

    foreach(array_keys($data['years']) as $y) {
        if($y >= $rd['fywd'] && $y <= $rd['lywd']) {
            continue;
        }

        // all years outside of fywd -> lywd, set values to null
        foreach($rd['ets'] as &$years) {
            $years[$y] = null;
        }
        unset($years);
        $rd['leaves'][$y] = [];
    }

    $rd['ranking'] = null;
    unset($rd['emissions']);
    unset($rd['rankings']);
    unset($rd['sources']);

    // storage?

    $rd['acid'] = $parent;
    $rd['subRegions'] = $children;
    $data['regions'][$parent] = $rd;
}
unset($rd);

// Make a copy of data so we can resume with the original version after we removed trade data
$data_copy = $data;

// Remove trade
$trade_e = array_search('trade', $data['ets']);
foreach($data['regions'] as &$rd) {
    $ys_with_trade = [];
    if(isset($rd['ets']['trade'])) {
        foreach($rd['ets']['trade'] as $y => $value) {
            $leaf_index = array_search($trade_e, $rd['leaves'][$y]);
            if($leaf_index !== false) {
                $rd['ets']['total'][$y] -= $value;
                unset($rd['leaves'][$y][$leaf_index]);
                $rd['leaves'][$y] = array_values($rd['leaves'][$y]);

                // Unusual - if trade was the sole energy type, then set the total to null. Palestine in 2000 may be only example.
                if(count($rd['leaves'][$y]) == 0) {
                    $rd['ets']['total'][$y] = null;
                }

                if(!in_array($y, $ys_with_trade)) {
                    $ys_with_trade[] = $y;
                }
            }
        }
        unset($rd['ets']['trade']);
    }
}
unset($rd);
unset($data['ets'][$trade_e]);

csv_file_put_contents('data-excluding-net-imports.csv', $data);
$json = json_encode($data);
file_put_contents('data-excluding-net-imports.json', $json);

// Restore data
$data_no_trade = $data;
$data = $data_copy;

// For group regions, keep using the no-trade versions
foreach($data['regions'] as $region => $rd) {
    if($region == 'W' || isset($rd['subRegions'])) {
        $data['regions'][$region] = $data_no_trade['regions'][$region];
    }
}

function storeData($source_filter, $basename) 
{
    global $store;
    $dms = new DataMerger($store, $source_filter);
    $data = $dms->getMergedData();
    csv_file_put_contents($basename . '.csv', $data);
    $json = json_encode($data);
    file_put_contents($basename . '.json', $json);
}

foreach($dm->getSources() as $source) {
    echo 'Creating data for source ', $source, PHP_EOL;
    $basename = 'data-' . strtolower($source);
    storeData($source, $basename);
}

// Merge IEA Yearly and Monthly
storeData(['IEAYearly', 'IEAMonthly'], 'data-iea');

// Add trade data
$trade_store = new DataSeriesStore();
$trade_store->addMember(new EnerdataTrade());
$trade_store->addMember(new IEATrade(2017)); // IEA after 2018 doesn't seem accurate

$ember_trade = $store->getMember('IEAMonthly - Trade');
$ember_trade->allowNegativeValues();
$trade_store->addMember($ember_trade);

$ember_trade = $store->getMember('Ember - Net imports');
$ember_trade->allowNegativeValues();
$trade_store->addMember($ember_trade);

// For manual verification only. They don't match exactly but pretty close. The Enerdata data is more complete so we just go ahead with that, no need to merge the two datasets.
//$trade_store->requireThatCombinationsMatch(pow(10, -3), ['IEATrade - Trade'], ['EnerdataTrade - Trade']);

// Replace totals with totals including net imports (if positive)
$tdm = new TradeDataMerger($trade_store);
$trade_data = $tdm->getData();
$regions_without_trade_data = file('regions_without_trade_data', FILE_IGNORE_NEW_LINES);
if(!in_array('trade', $data['ets'])) {
    $data['ets'][] = 'trade';
}
foreach($data['regions'] as $region => &$rd) {

    // Who is the world going to trade with? :)
    if($region == 'W') {
        continue;
    }

    $trade_rds = [];

    if(isset($rd['subRegions'])) {
        $regions = $rd['subRegions'];
    } else {
        $regions = [];
        $regions[] = $region;
    }

    foreach($regions as $r) {
        if(!isset($trade_data['regions'][$r])) {
            if(in_array($r, $regions_without_trade_data)) {
                continue;
            }
            ksort($trade_data['regions']);
            print_r(array_keys($trade_data['regions']));
            trigger_error('Region ' . $r . ' not found in trade data', E_USER_ERROR);
        }
        $trade_rds[] = $trade_data['regions'][$r];
    }

    // If it does not already contain trade data
    if(!isset($rd['ets']['trade'])) {
        $rd['ets']['trade'] = array_fill_keys(array_keys($data['years']), null);
    }

    $rd['fywd'] = null;
    $rd['lywd'] = null;
    foreach($data['years'] as $y => $year) {
        // If we already have trade data for this year
        if($rd['ets']['trade'][$y] !== null) {
            if($rd['fywd'] === null) {
                $rd['fywd'] = $y;
            }
            $rd['lywd'] = $y;
            continue;
        }

        $total2_v = $rd['ets']['total'][$y];
        $rd['ets']['total'][$y] = null;
        if($total2_v === null) {
            continue;
        }
        $trade_y = array_search($year, $trade_data['years']);
        if($trade_y !== false) {
            $trade_vs = [];
            foreach($trade_rds as $trade_rd) {
                if(isset($trade_rd['values'][$trade_y])) {
                    $trade_v = $trade_rd['values'][$trade_y];
                    if($trade_v === null) {
                        continue;
                    }
                    $trade_vs[] = $trade_v;
                }
            }

            // Missing values for at least one region
            if(count($trade_vs) != count($trade_rds)) {
                continue;
            }

            $trade_v = array_sum($trade_vs);
            if($trade_v >= 0) {
                $rd['ets']['trade'][$y] = $trade_v;
            }
            if($trade_v > 0) {
                $total2_v += $trade_v;
                $rd['leaves'][$y][] = array_search('trade', $data['ets']);
            }
            $rd['ets']['total'][$y] = $total2_v;

            if($rd['fywd'] === null) {
                $rd['fywd'] = $y;
            }
            $rd['lywd'] = $y;
        }
    }
}
unset($rd);

$dm->setData($data);
$data = $dm->getMergedData();

csv_file_put_contents('data-including-net-imports.csv', $data);
$json = json_encode($data);
file_put_contents('data-including-net-imports.json', $json);

echo 'done', PHP_EOL;
