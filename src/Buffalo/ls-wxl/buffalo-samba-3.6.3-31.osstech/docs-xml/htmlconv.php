<?php

$file = fopen("php://stdin","r");
$str = "";

while( !feof($file) ){
  $line = fgets($file);
  $line = preg_replace('/(; charset=)ISO-8859-1/','${1}UTF-8',$line);
  $line = mb_convert_encoding($line,"UTF-8","HTML-ENTITIES");
  $line = preg_replace('/\n/','',$line);
  $str .= $line;
}
fclose($file);

echo($str);
