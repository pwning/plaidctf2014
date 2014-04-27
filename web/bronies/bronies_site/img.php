<?php
// File and rotation

mysql_connect("localhost","root","123456");
mysql_select_db("bronies");

$files = glob('data/imgs/*.png');
$filename = $files[array_rand($files)];

$filename2 = preg_replace('/.png/', '', basename($filename));
mysql_query("insert into cs (ans) values ('$filename2')");

session_start();

$_SESSION['cs'] = mysql_insert_id();

$degrees = rand(-30,30);

// Content type
header('Content-type: image/png');

// Load
$source = imagecreatefrompng($filename);

// Rotate
$rotate = imagerotate($source, $degrees, imagecolorallocatealpha($source, 255, 55, 255, 127));
imagesavealpha($rotate, true);

// Output
imagepng($rotate);

?>

