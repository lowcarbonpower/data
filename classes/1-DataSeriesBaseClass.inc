<?php

abstract class DataSeriesBaseClass implements DataSeries, DiffTolerance
{

    public $unset_regions_with_zero_sums = true;

    protected const IGNORE_REGION = 1;

    protected $allowNegativeValues = false;

    protected $region_map = [];

    protected $regions;

    protected $verifyWorldSums = true;

    private $multiplier;

    public function allowNegativeValues() 
    {
        $this->allowNegativeValues = true;
        unset($this->regions);
    }

    public final function getFirstYear() 
    {
        $years = $this->getYears();
        if($years) {
            return current($years);
        }
    }

    public final function getLastYear() 
    {
        $years = $this->getYears();
        if($years) {
            return end($years);
        }
    }

    public final function getRegions() 
    {
        $this->setRegionsOnce();
        return $this->regions;
    }

    public function getSource() 
    {
        return get_class($this);
    }

    public final function getValue($region, $year) 
    {
        $regions = $this->getRegions();
        if(isset($regions[$region]) && isset($regions[$region][$year])) {
            return $regions[$region][$year];
        }
        return 0;
    }

    public final function isEmpty() 
    {
        if(!$this->getEnergyType()) {
            return true;
        }
        $this->setRegionsOnce();
        if(!$this->getYears()) {
            return true;
        }
        return false;
    }

    public final function setMultiplier($multiplier) 
    {
        $this->multiplier = $multiplier;
    }

    public function testTolerance($diff_rel, $diff_abs, $year, $region) 
    {
        if($diff_rel < pow(10, -10)) {
            return true;
        }
    }

    protected static function getNormalizedRegionName($in) 
    {
        switch($in) {
        case 'China Hong Kong SAR':
            return 'Hong Kong';
        case 'Czech Republic':
            return 'Czechia';
        case 'Iran':
            return 'Iran (Islamic Republic of)';
        case 'South Korea':
            return 'Korea, Republic of';
        case 'Taiwan':
            return 'Taiwan, Province of China';
        case 'Total World':
            return 'World';
        case 'Trinidad & Tobago':
            return 'Trinidad and Tobago';
        case 'United Kingdom':
            return 'United Kingdom of Great Britain and Northern Ireland';

        case 'Unites States':
        case 'US':
            return 'United States of America';
        case 'Venezuela':
            return 'Venezuela (Bolivarian Republic of)';
        case 'Vietnam':
            return 'Viet Nam';

        case 'Bahamas, The':
            return 'Bahamas';

        case 'Bolivia':
            return 'Bolivia (Plurinational State of)';

        case 'Brunei':
            return 'Brunei Darussalam';

        case 'Burkina-Faso':
            return 'Burkina Faso';

        case 'Cape Verde':
            return 'Cabo Verde';

        case 'Czech Republic':
            return 'Czechia';

        case 'Congo, Dem. Rep.':
        case 'Congo-Kinshasa':
            return 'Congo, Democratic Republic of the';

        case 'Congo':
        case 'Congo, Rep.':
        case 'Congo-Brazzaville':
            return 'Congo';

        case "Cote d'Ivoire":
            return "Côte d'Ivoire";

        case 'Curacao':
            return 'Curaçao';

        case 'Egypt, Arab Rep.':
            return 'Egypt';

        case 'Faeroe Islands':
            return 'Faroe Islands';

        case 'Iran':
        case 'Iran, Islamic Rep.':
            return 'Iran (Islamic Republic of)';

        case 'Gambia, The':
            return 'Gambia';

        case 'Hong Kong SAR, China':
            return 'Hong Kong';

        case 'Iran':
            return 'Iran (Islamic Republic of)';

        case "Dem. People's Rep. Korea";
        case 'Korea, Dem. People’s Rep.':
            return "Korea (Democratic People's Republic of)";

        case 'Korea':
        case 'Korea, Rep.':
        case 'Korea, South':
            return 'Korea, Republic of';

        case 'Kyrgyz Republic':
            return 'Kyrgyzstan';

        case 'Lao PDR':
            return "Lao People's Democratic Republic";

        case 'Macao SAR, China':
            return 'Macao';

        case 'Micronesia, Fed. Sts.':
            return 'Micronesia (Federated States of)';

        case 'Moldova':
            return 'Moldova, Republic of';

        case 'Micronesia':
            return 'Micronesia (Federated States of)';

        case 'Burma':
            return 'Myanmar';

        case 'Reunion':
            return 'Réunion';

        case 'Russia':
            return 'Russian Federation';

        case 'Slovak Republic':
            return 'Slovakia';

        case 'St. Kitts and Nevis':
            return 'Saint Kitts and Nevis';

        case 'St. Lucia':
            return 'Saint Lucia';

        case 'St. Martin (French part)':
            return 'Saint Martin (French part)';

        case 'St. Vincent and the Grenadines':
            return 'Saint Vincent and the Grenadines';

        case 'Swaziland':
            return 'Eswatini';

        case 'Taiwan, China':
            return 'Taiwan, Province of China';

        case 'Tanzania':
            return 'Tanzania, United Republic of';

        case 'United Kingdom':
            return 'United Kingdom of Great Britain and Northern Ireland';

        case 'United States':
            return 'United States of America';

        case 'Venezuela':
        case 'Venezuela, RB':
            return 'Venezuela (Bolivarian Republic of)';

        case 'Vietnam':
            return 'Viet Nam';

        case 'Virgin Islands (U.S.)':
            return 'Virgin Islands (U.S.)';

        case 'British Virgin Islands':
            return 'Virgin Islands (British)';

        case 'Yemen, Rep.':
            return 'Yemen';

        case 'The Bahamas':
            return 'Bahamas';
        case 'Burma':
            return 'Myanmar';
        case 'Congo-Kinshasa':
            return 'Congo, Democratic Republic of the';
        case 'Congo-Brazzaville':
            return 'Congo';
        case 'Côte d’Ivoire':
            return "Côte d'Ivoire";
        case 'Falkland Islands':
            return 'Falkland Islands (Malvinas)';
        case 'Laos':
            return "Lao People's Democratic Republic";
        case 'Macau':
            return 'Macao';
        case 'North Korea':
            return "Korea (Democratic People's Republic of)";
        case 'Palestinian Territories':
            return 'Palestine, State of';
        case 'Saint Helena':
            return 'Saint Helena, Ascension and Tristan da Cunha';
        case 'Saint Vincent/Grenadines':
            return 'Saint Vincent and the Grenadines';
        case 'South Korea':
            return 'Korea, Republic of';
        case 'Syria':
            return 'Syrian Arab Republic';
        case 'Taiwan':
            return 'Taiwan, Province of China';
        case 'U.S. Virgin Islands':
            return 'Virgin Islands (U.S.)';
        }

        return $in;
    }

    protected final function getNormalizedValue($v) 
    {
        if($v === null) {
            return $v;
        }
        $v = (float)$v;

        if($this->multiplier) {
            $v = $v * $this->multiplier;
        }

        return $v;
    }

    protected final function getRegionId($region_name) 
    {
        if(!isset($this->region_map[$region_name])) {
            print_r($this->region_map);
            trigger_error('Missing region name [' . $region_name . ']', E_USER_ERROR);
        }
        return $this->region_map[$region_name];
    }

    protected function setValueByRegionId($region_id, $year, $value) 
    {
        if(!$this->allowNegativeValues && $value < 0) {
            $value = 0;
        }

        $value = $this->getNormalizedValue($value);
        if(isset($this->regions[$region_id][$year])) {
            if($value === $this->regions[$region_id][$year]) {
                return;
            }
            print_r($this->regions[$region_id]);
            trigger_error('Value already set in ' . get_class($this) . ' and energy type ' . $this->getEnergyType() . ' and region ' . $region_id . ' and year ' . $year . '. New value: ' . $value, E_USER_ERROR);
        }
        $this->regions[$region_id][$year] = $value;
    }

    protected function setValueByRegionName($region_name, $year, $value) 
    {
        $region_name = $this->getNormalizedRegionName($region_name);
        if($region_name === self::IGNORE_REGION) {
            return;
        }
        $region_id = $this->getRegionId($region_name);
        if(!isset($this->regions[$region_id])) {
            $this->regions[$region_id] = [];
        }
        $this->setValueByRegionId($region_id, $year, $value);
    }

    protected abstract function setRegions();

    private function setRegionsOnce() 
    {
        if(isset($this->regions)) {
            return;
        }

        $json = file_get_contents(dirname(__DIR__) . '/iso-3166.json');
        $data = json_decode($json);
        foreach($data as $item) {
            $this->region_map[$item->name] = $item->{'alpha-2'};
        }

        $this->region_map['World'] = 'W';

        $this->regions = [];
        $this->setRegions();

        foreach($this->regions as $region => $years) {
            if($region == 'W') {
                continue;
            }

            $sum = array_sum($years);

            if($this->unset_regions_with_zero_sums) {
                // Almost 0
                if(abs($sum) < pow(10, -5)) {
                    unset($this->regions[$region]);
                }
            }
        }

        if($this->verifyWorldSums) {
            if(!isset($this->regions['W'])) {
                ksort($this->regions);
                print_r(array_keys($this->regions));
                trigger_error('Missing world region key in ' . get_class($this) . ' with type ' . $this->getEnergyType(), E_USER_ERROR);
            }

            // Manually calculate world sums
            $world_from_sum = [];
            foreach($this->regions as $region => $years) {
                if($region == 'W') {
                    continue;
                }

                foreach($years as $year => $value) {
                    if(!isset($world_from_sum[$year])) {
                        $world_from_sum[$year] = 0;
                    }

                    $world_from_sum[$year] += $value;
                }
            }

            // Compare series to $regions - world using requireThatCombinationsMatch logic
            $tag = get_class($this) . '-' . $this->getEnergyType();
            requireThatRegionsMatch(
                [
                $tag . '-WorldFromSource' => [$this->regions['W']],
                $tag . '-WorldFromSum' => [$world_from_sum]
                ], $this
            );
        }
    }
}
