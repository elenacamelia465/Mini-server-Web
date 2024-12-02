<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Failed</title>
    <script>
        // Redirecționare automată către login.php după 3 secunde
        setTimeout(function() {
            window.location.href = "login.php";
        }, 3000);
    </script>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin-top: 20%;
        }
    </style>
</head>
<body>
    <h1>Login Failed</h1>
    <p>Invalid username or password. You will be redirected back to the login page in 3 seconds...</p>
    <p>If the redirection does not happen, <a href="login.php">click here</a>.</p>
</body>
</html>
