<?php

class Emissions
{

    private $data;

    public function __construct($data) 
    {
        $this->data = $data;
    }

    public function get($region) 
    {
        $rd = $this->data['regions'][$region];
        $emissions = [];

        $ets = array_keys($rd['ets']);

        foreach($rd['ets']['total'] as $y => $v) {

            // Default value
            $emissions[$y] = null;

            // No total for this year, skip
            if(!$v) {
                continue;
            }

            // Low and high estimates
            $emissions_low = [];
            $emissions_high = [];

            foreach($rd['leaves'][$y] as $e) {

                $et = $this->data['ets'][$e];

                if(!isset($rd['ets'][$et][$y])) {
                    echo $et, PHP_EOL;
                    echo $y, PHP_EOL;
                    print_r($rd['ets']);
                    print_r($rd['leaves']);
                    trigger_error('Missing et: ' . $et, E_USER_ERROR);
                }
                $value = $rd['ets'][$et][$y];
                if(!($value > 0)) {
                    continue;
                }

                if(!isset(EMISSIONS[$et])) {
                    echo $region, PHP_EOL;
                    echo $et, PHP_EOL;
                    echo $y, PHP_EOL;
                    trigger_error('Missing emissions data for et: ' . $et, E_USER_ERROR);
                }

                if(is_array(EMISSIONS[$et])) {
                    $emissions_low[$et] = $value * 1000000000 * EMISSIONS[$et][0];
                    $emissions_high[$et] = $value * 1000000000 * EMISSIONS[$et][1];
                } else {
                    $emissions_low[$et] = $value * 1000000000 * EMISSIONS[$et];
                    $emissions_high[$et] = $value * 1000000000 * EMISSIONS[$et];
                }
            }

            $emissions_low_sum = array_sum($emissions_low);
            $emissions_high_sum = array_sum($emissions_high);
            $emissions_avg_sum = ($emissions_low_sum + $emissions_high_sum) / 2;

            if($emissions_low_sum != $emissions_high_sum) {
                $emissions[$y] = [
                $emissions_avg_sum,
                $emissions_low_sum,
                $emissions_high_sum
                ];
            } else {
                $emissions[$y] = [$emissions_avg_sum];
            }
            /*
            if($region == 'FI' && $y == 48) {
            echo $region, PHP_EOL;
            echo $y, PHP_EOL;
            echo $emissions_high_sum, PHP_EOL;
            print_r($emissions_high);
            echo $emissions_low_sum, PHP_EOL;
            print_r($emissions_low);
            print_r($this->data->etss->{$et});
            print_r($rd->ets);
            exit;
            }
            */
        }

        return $emissions;
    }
}
