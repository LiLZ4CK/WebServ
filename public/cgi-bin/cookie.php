<?php

$name = $_GET["name"];

    setcookie("name", $name);

header("Content-Type: text/html");

echo "<html><head><title>Greetings</title></head><body>";
if ($name) {
    echo "<h1>Hello, $name!</h1>";
} else {
    echo "<h1>Please enter your name.</h1>";
}
$env_array =getenv();

echo '<form method="GET" action="cookie.php">';
echo 'Name: <input type="text" name="name">';
echo '<input type="submit" value="Submit">';
echo '</form>';
echo "</body></html>";
?>