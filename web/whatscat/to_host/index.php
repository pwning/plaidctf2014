<html>
  <head>
    <title>
      WhatsCat
    </title>
    <link rel="stylesheet" type="text/css" href="default.css" />
  </head>
  <br>
  <img class=logo src=logo.png>

<?php include "head.php"; ?>
<body>
  <div id=box>
  <div id=inbox>
<?php 
$allowed = array("main","login","logout","cat");
if (isset($_GET["page"]) && in_array($_GET["page"],$allowed)) {
  include $_GET["page"];
}
else {
  include "main";
}
?>
  </div>
  </div>
</body>
<br>

<?php include "foot.php"; ?>
</html>
