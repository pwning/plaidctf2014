<?php
session_start(); 
// TODO: Perhaps check REMOTE_IP here too?
$username = $_SESSION['username'];
$logged_in = $username === 'ebleford';
?>
<?php if ($logged_in): ?>
<!doctype html>
<html>
  <head>
    <title>
      eXtreme Secure Solutions Internal Portal
    </title>
  </head>
  <link rel="stylesheet" type="text/css" charset="utf-8" media="all" href="/style.css">
  <body>
    <div id="box">
        <h1>eXtreme Secure Solutions Internal Portal</h1>
        <div id="notice">
            Flag #1: xss_problem_is_web_problem<br>
            This challenge has one more flag.  Break into the internal server to capture it!
        </div>
        <p>
            You are logged in as <?= htmlspecialchars($username) ?>.
        </p>
        <p>
            Reminder: For security reasons, all internet traffic is blocked on the Bigson.
        </p>
        <p>
<script>
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://bigson.essolutions.largestctf.com/status', false);
    xhr.send(null);
    if (xhr.responseJSON["status"] == "ok") {
        document.write('The Bigson is up!');
    } else {
        document.write('The Bigson is down!');
    }
</script>
        </p>
        <ul id="menu">
            <li><a href="http://bigson.essolutions.largestctf.com/index?file=index.html">The Bigson</a></li>
            <li><a href="/logout.php">Logout</a></li>
        </ul>
    </div>
  </body>
</html>
<?php else: ?>
<!doctype html>
<html>
  <head>
    <title>
      eXtreme Secure Solutions Internal Login
    </title>
  </head>
  <link rel="stylesheet" type="text/css" charset="utf-8" media="all" href="/style.css">
  <body>
    <div id="box">
      <h1>eXtreme Secure Solutions Internal Login</h1>
      <form method="POST" action="/login.php">
      <table id="login">
        <tr>
          <td><label for="username">Username:</label></td>
          <td><input type="text" id="username" name="username"></td>
        </tr>
        <tr>
          <td><label for="password">Password:</label></td>
          <td><input type="password" id="password" name="password"></td>
        </tr>
        <tr>
          <td><label for="otp">OTP:</label></td>
          <td><input type="text" id="otp" name="otp"></td>
        </tr>
        <tr>
          <td colspan="2"><input type="submit" value="Login"></td>
        </tr>
      </table>
      </form>
    </div>
  </body>
</html>
<?php endif; ?>
