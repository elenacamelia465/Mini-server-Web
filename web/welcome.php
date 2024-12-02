<?php


function getUsernameFromToken($token) {
    if (!is_readable('tokens.txt')) {
        
        return null;
    }

    $file = fopen('tokens.txt', 'r'); 
    if ($file) {
        while (($line = fgets($file)) !== false) {
           
            list($savedToken, $savedUsername) = explode(',', trim($line));
           
            if ($token === $savedToken) { 
                fclose($file);
                return $savedUsername; 
            }
        }
        fclose($file);
    } else {
       
    }
    return null; 
}

$token = isset($_GET['token']) ? $_GET['token'] : '';

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
