<?php
    $username = $_POST['username'];
    $password = $_POST['password'];

    if ($username === 'admin' && $password === 'admin') {
        echo "<h1>Bun venit, $username!</h1>";
        echo "<p>Login reușit. Acesta este un exemplu de procesare a datelor în PHP.</p>";
    } else {
        echo "<h1>Acces refuzat!</h1>";
        echo "<p>Username sau parola incorecte.</p>";
    }
?>
