<?php
session_start();
unset($_SESSION["username"]);
http_redirect("/");
