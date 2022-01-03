<?php

function getAmchartsIds() 
{
    static $ids;
    if(!isset($ids)) {
        $url = 'https://www.amcharts.com/lib/4/geodata/worldLow.js';
        $cache_file = '/tmp/' . md5($url);
        if(!file_exists($cache_file)) {
            $contents = file_get_contents($url);
            file_put_contents($cache_file, $contents);
        }
        $lines = file($cache_file);
        $prefix = 'am4internal_webpackJsonp(["fcaa"],';
        foreach($lines as $line) {
            if(substr($line, 0, strlen($prefix)) == $prefix) {
                $json = preg_replace('/^.+?var r=/', '', $line);
                $json = preg_replace('/};.+?$/', '}', $json);

                include_once '/usr/share/php/Services/JSON.php';
                $json_serv = new Services_JSON();
                $data = $json_serv->decode($json);

                $ids = [];
                foreach($data->features as $feature) {
                    $name = $feature->properties->name;
                    $id = $feature->properties->id;
                    $ids[$name] = $id;
                }
            }
        }
    }
    return $ids;
}

// Pretty Energy Type
function pet($et, $lid = LANG) 
{
    if($et == 'lowcarbon') {
        $et = 'low carbon sources';
    }
    if($et == 'fossil') {
        $et = 'fossil fuels';
    }
    if($et == 'hydro') {
        $et = 'hydropower';
    }
    if($et == 'trade') {
        $et = 'net imports';
    }
    $et = str_replace('-', ' ', $et);
    $et = ucwords($et);
    $et = t($et, false, $lid);
    return $et;
}

// Pretty Region Name
function prn($id, $flip = false, $lid = LANG) 
{
    static $maps = [];
    if(!isset($maps[$lid])) {
        $lid2 = $lid;
        if($lid2 == 'zht') {
            $lid2 = 'zh_Hant';
        }
        $path = dirname(__DIR__) . '/country-list/data/' . $lid2 . '/country.json';
        $json = file_get_contents($path);
        $map = json_decode($json);
        $map = (array)$map;

        $map['CN'] = t("People's Republic of China", false, $lid);
        $map['TW'] = t('Republic of China (Taiwan)', false, $lid);

        // Groups
        $map['EU'] = t('EU', false, $lid);
        $map['SSA'] = t('Sub-Saharan Africa', false, $lid);

        // Missing from some
        $map['XK'] = t('Kosovo', false, $lid);

        $map['W'] = t('World', false, $lid);

        global $data;
        $regions = is_object($data) ? $data->regions : $data['regions'];
        foreach(array_keys((array)$regions) as $region) {
            if(strlen($region) > 3) {
                $region_name = $region;
                
                foreach(ETS as $et) {
                    $region_name = str_replace($et, pet($et, $lid), $region_name);
                }

                $region_name = preg_replace_callback(
                    '/0\.\d+/', function ($x) {
                        return $x[0] * 100 . '%';
                    }, $region_name
                );

                $map[$region] = $region_name;
            }
        }

        $maps[$lid] = $map;
    }

    $map = $maps[$lid];

    if($flip) {
        $map = array_flip($map);
    }

    if(isset($map[$id])) {
        return $map[$id];
    }
}

function requireThatRegionsMatch($sets, DiffTolerance $dt) 
{
    // To access first and second set without knowing key name
    $sets_values = array_values($sets);

    // Make an array of all regions in either set
    $regions = [];
    foreach($sets as $k => $set) {
        foreach($set as $region => $years) {
            if(!in_array($region, $regions)) {
                $regions[] = $region;
            }

            // Sort the array by year
            ksort($years);
            $sets[$k][$region] = $years;
        }
    }

    try {
        foreach($regions as $region) {

            // Make an array of all years for this region in either set
            $years = [];

            foreach($sets as $set) {

                // Make sure region is present in both sets
                if(!isset($set[$region])) {
                    $E_REGION = $region;
                    throw new Exception($region . ' missing from set');
                }

                foreach(array_keys($set[$region]) as $year) {
                    if(!in_array($year, $years)) {
                        $years[] = $year;
                    }
                }
            }

            sort($years);

            foreach($years as $year) {
                $value1 = isset($sets_values[0][$region][$year]) ? $sets_values[0][$region][$year] : null;
                $value2 = isset($sets_values[1][$region][$year]) ? $sets_values[1][$region][$year] : null;

                if($value1 === $value2) {
                    continue;
                }

                $diff_abs = abs($value1 - $value2);
                $min = (float)min(abs($value1), abs($value2));
                $max = (float)max(abs($value1), abs($value2));
                if($min === (float)0 && $max > 0) {
                    $diff_rel = 1;
                } elseif($max === (float)0) {
                    $diff_rel = 0;
                } else {
                    $diff_rel = $diff_abs / $max;
                }
                if(!$dt->testTolerance($diff_rel, $diff_abs, $year, $region)) {
                    $E_REGION = $region;
                    throw new Exception('Diff too high for region ' . $region . ' and year ' . $year . '. Diff abs: ' . $diff_abs . ' / diff rel: ' . $diff_rel);
                }
            }
        }
    } catch(Exception $e) {
        $filenames = [];
        foreach($sets as $key => &$set) {
            $key = str_replace([' ', '/', '&', chr(0)], '-', $key);
            if(isset($E_REGION)) {
                $set = $set[$E_REGION];
            }
            $filename = tempnam('/tmp', $key . '_');
            $filenames[] = '"' . $filename . '"';
            file_put_contents($filename, print_r($set, true));
        }
        print_r($sets);
        trigger_error($e->getMessage());
        $cmd = 'meld ' . implode(' ', $filenames);
        exec($cmd);
        $handle = fopen('php://stdin', 'r');
        $line = fgets($handle);
    }
}
