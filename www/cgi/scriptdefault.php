#!/usr/bin/php
<?php
// Set the Content-Type header
header("Content-Type: text/html; charset=UTF-8");

// Add additional headers
header("X-Content-Type-Options: nosniff");

// End of headers, now separate headers and body with "\r\n\r\n"
echo "\r\n\r\n";
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

