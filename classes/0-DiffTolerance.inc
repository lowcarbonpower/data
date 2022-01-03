<?php

interface DiffTolerance
{

    function testTolerance($diff_rel, $diff_abs, $year, $region);
}
