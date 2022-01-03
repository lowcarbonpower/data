<?php

abstract class DataSeriesFromCsvFile extends DataSeriesBaseClass
{

    protected $region_row_offset = 1;

    protected $source_dir, $source_file;

    private $rows;

    public function __construct($source_dir, $source_file) 
    {
        $this->source_dir = $source_dir;
        $this->source_file = $source_file;
        $this->rows = null;
    }

    public function getSourceDir() 
    {
        return $this->source_dir;
    }

    public function getSourceFile() 
    {
        return $this->source_file;
    }

    public function getSourcePath() 
    {
        return $this->source_dir . '/' . $this->source_file;
    }

    protected function getFieldValue($row, $column) 
    {
        $rows = $this->getRows();
        return $rows[$row][$column];
    }

    protected function getRows($row_offset = 0) 
    {
        if($this->rows == null) {
            $this->rows = [];
            if(is_file($this->getSourcePath())) {
                if(($handle = fopen($this->getSourcePath(), 'r')) !== false) {
                    while(($row = fgetcsv($handle)) !== false) {
                        $this->rows[] = $row;
                    }
                }
            } else {
                trigger_error('File not found: ' . $this->getSourcePath());
            }
        }
        return array_slice($this->rows, $row_offset);
    }

    protected function setRegions() 
    {
        $years = $this->getYears();
        $rows = $this->getRows($this->region_row_offset);
        foreach($rows as $i => $row) {
            $values = [];
            $region_name = $row[0];
            foreach($years as $k => $year) {
                $this->setValueByRegionName($region_name, $year, $row[$k]);
            }
        }
    }
}
