<?php
  dob(234);
  echo "\n";
  dob(0);
  echo "\n";
  dob(90);
  echo "\n";
  dob(180);
  echo "\n";
  dob(2);
  echo "\n";

  function dob($bearing) {
    $dirs = array("N","E","S","W");

    $rounded = round($bearing / 22.5) % 16;
    if (($rounded % 4) == 0) {
      echo $dirs[$rounded / 4] . "\n";
      echo $rounded / 4;
    } // if
    else {
      echo $dirs[2 * floor(((floor($rounded / 4) + 1) % 4) / 2)] . "\n";
      echo $dirs[1 + 2 * floor($rounded / 8)] . "\n";
      echo 2 * floor(((floor($rounded / 4) + 1) % 4) / 2) . "\n";
      echo 1 + 2 * floor($rounded / 8) . "\n";
    }  // if
  } // do

?>
