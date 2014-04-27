<ul id=links />
<li><a href="index.php?page=main">home</a></li>
<?php
session_start();
if (isset($_SESSION['uid'])) {
echo '<li><a href="index.php?page=msgs">messages</a></li>';
echo '<li><a href="index.php?page=logout">logout</a></li>';
}
else {
echo '<li><a href="index.php?page=login">login</a></li>';
}
?>
</ul>
