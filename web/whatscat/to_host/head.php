<ul id=links />
<li><a href="index.php?page=main">home</a></li>
<li><a href="index.php?page=cat">cats</a></li>
<?php
session_start();
if (isset($_SESSION['uid'])) {
echo '<li><a href="index.php?page=logout">logout</a></li>';
}
else {
echo '<li><a href="index.php?page=login">login</a></li>';
}
?>
</ul>
