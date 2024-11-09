#!/usr/bin/php
<?php
header("Content-Type: text/html");

// Capture query parameters
$name = isset($_GET['name']) ? htmlspecialchars($_GET['name']) : "Guest";

echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>GET Request</title>
    <style>
        body { background-color: #f4f4f9; color: #333; font-family: Arial, sans-serif; }
        .container { text-align: center; padding: 50px; }
        .greeting { color: #4CAF50; font-size: 2em; }
    </style>
</head>
<body>
    <div class="container">
        <p class="greeting">Hello, {$name}!</p>
        <p>Welcome to our CGI example for GET requests.</p>
    </div>
</body>
</html>
HTML;
?>

