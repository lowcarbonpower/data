<?php

class DataSeriesInverted extends DataSeriesClone
{

    public function __construct(DataSeries $ds) 
    {
        parent::__construct($ds);
        foreach($this->regions as $region => &$years) {
            foreach($years as &$value) {
                $value = -1 * $value;
            }
        }
    }
}
