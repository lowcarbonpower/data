<?php

function force_set($object, $property, $value) 
{
    call_user_func(
        \Closure::bind(
            function () use ($object, $property, $value) {
                $object->{$property} = $value;
            },
            null,
            $object
        )
    );
}

function force_get($object, $property) 
{
    return call_user_func(
        \Closure::bind(
            function () use ($object, $property) {
                return $object->{$property};
            },
            null,
            $object
        )
    );
}

class DataSeriesClone extends DataSeriesBaseClass
{

    private $clone;

    public function __construct(DataSeries $source) 
    {
        $this->clone = clone $source;

        $r = new ReflectionObject($source);

        // Don't include static properties
        foreach($r->getProperties() as $property) {
            if($property->isStatic()) {
                continue;
            }
            $k = $property->name;
            $v = force_get($source, $k);
            force_set($this, $k, $v);
        }
    }

    public function getEnergyType() 
    {
        return $this->clone->getEnergyType();
    }

    public function getSource() 
    {
        return $this->clone->getSource();
    }

    public function getYears() 
    {
        return $this->clone->getYears();
    }

    public function __call($function, $args) 
    {
        return call_user_func_array([$this->clone, $function], $args);
    }

    protected function setRegions() 
    {
        return $this->clone->setRegions();
    }
}

