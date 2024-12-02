<?php

echo "<p>DEBUG: Full URL: http://" . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'] . "</p>";
echo "<p>DEBUG: Query String: " . $_SERVER['QUERY_STRING'] . "</p>";
echo "<p>DEBUG: Token in $_GET: " . (isset($_GET['token']) ? $_GET['token'] : 'NOT SET') . "</p>";

function getUsernameFromToken($token) {
    if (!is_readable('tokens.txt')) {
        echo "<p>DEBUG: tokens.txt is not readable. Check file permissions.</p>";
        return null;
    }

    $file = fopen('tokens.txt', 'r'); 
    if ($file) {
        while (($line = fgets($file)) !== false) {
            echo "<p>DEBUG: Line read from tokens.txt: $line</p>";
            list($savedToken, $savedUsername) = explode(',', trim($line));
            echo "<p>DEBUG: Saved token: $savedToken, Saved username: $savedUsername</p>";
            if ($token === $savedToken) { 
                fclose($file);
                return $savedUsername; 
            }
        }
        fclose($file);
    } else {
        echo "<p>DEBUG: Failed to open tokens.txt</p>";
    }
    return null; 
}

$token = isset($_GET['token']) ? $_GET['token'] : '';
echo "<p>DEBUG: Token received from URL: $token</p>";

$username = getUsernameFromToken($token);

if ($username) {
    echo "<h1>Bun venit, $username!</h1>";
    echo "<p>Te-ai logat cu succes.</p>";
} else {
    echo "<h1>Acces refuzat!</h1>";
    echo "<p>Token invalid sau expirat.</p>";
    echo "<p>DEBUG: Token validation failed.</p>";
}
?>
