<?php
$username = $_POST["username"];
$password = $_POST["password"];
$otp = $_POST["otp"];
if (!$username || !$password || !$otp) {
    header("Location: /");
}
?>
<body>
<h1>Login status</h1>
<ul>
<?php
$descriptorspec = array(
    0 => array("pipe", "r"),
    1 => array("pipe", "w"),
);

$p = proc_open("./checkotp 2>&1",  $descriptorspec, $pipes);
fwrite($pipes[0], $username . "\n");
fwrite($pipes[0], $password . "\n");
fwrite($pipes[0], $otp . "\n");
fclose($pipes[0]);
echo stream_get_contents($pipes[1]);
fclose($pipes[1]);
$otp_correct = proc_close($p) == 0;

$login_correct = false;
if ($username !== "ebleford" || md5($password) !== "86d0b09363e0346792f72cdfe3559537") {
    echo '<li style="color: red">Username or password incorrect!</li>';
} else {
    echo '<li>Login correct...</li>';
    $login_correct = true;
}
?>
</ul>
<?php
if ($login_correct && $otp_correct) {
    session_start();
    $_SESSION['username'] = $username;
    //echo "<script>window.location='/';</script>";
    echo "Logged in!";
}
?>
</body>
