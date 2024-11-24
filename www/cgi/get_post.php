#!/usr/bin/php
<?php
// Set the Content-Type header for HTML output
header("Content-Type: text/html; charset=UTF-8");

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GET and POST Data Handler</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f7fc;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            flex-direction: column;
        }
        .container {
            background-color: #fff;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            width: 90%;
            max-width: 600px;
            padding: 60px;
            text-align: center;
            margin-bottom: 50px;
            
        }
        h1 {
            color: #333;
            margin-bottom: 20px;
        }
        form {
            margin-bottom: 20px;
            text-align: left;
        }
        label {
            display: block;
            margin: 5px 0;
            font-weight: bold;
        }
        input[type="text"] {
            padding: 8px;
            margin: 10px 0;
            width: 100%;
            border-radius: 4px;
            border: 1px solid #ccc;
        }
        input[type="submit"] {
            padding: 10px 20px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            margin-top: 10px;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
        .result {
            background-color: #e7f7e7;
            border: 1px solid #4CAF50;
            border-radius: 4px;
            padding: 15px;
            margin-top: 20px;
            text-align: left;
        }
        .result p {
            margin: 5px 0;
        }
        .result h2 {
            color: #4CAF50;
            font-size: 1.2em;
        }
    </style>
</head>
<body>
    <!-- GET Data Handler Section -->
    <div class="container">
        <h1>GET Data Handler</h1>
        <?php
        if ($_SERVER['REQUEST_METHOD'] == 'GET') {
            echo '<div class="result">';
            if (!empty($_GET)) {
                echo '<h2>Received GET Data:</h2>';
                foreach ($_GET as $key => $value) {
                    $escaped_value = htmlspecialchars($value);
                    echo "<p><strong>$key:</strong> $escaped_value</p>";
                }
            } else {
                echo "<p>No query parameters received.</p>";
            }
            echo '</div>';
        }
        ?>
    </div>

    <!-- POST Data Handler Section -->
    <div class="container">
        <h1>POST Data Handler</h1>
        <?php
        if ($_SERVER['REQUEST_METHOD'] == 'POST') {
            echo '<div class="result">';
            $dataFile = $_SERVER['DB_PATH'];
            if (file_exists($dataFile)) {
                $data = file_get_contents($dataFile);
                parse_str($data, $parsedData);
                $param1 = isset($parsedData['param1']) ? htmlspecialchars($parsedData['param1']) : 'Not set';
                $param2 = isset($parsedData['param2']) ? htmlspecialchars($parsedData['param2']) : 'Not set';
                echo '<h2>Received POST Data:</h2>';
                echo "<p><strong>Param1:</strong> $param1</p>";
                echo "<p><strong>Param2:</strong> $param2</p>";
            } else {
                echo "<p>Error: POST data file not found.</p>";
            }
            echo '</div>';
        } else {
            ?>
            <!-- Display POST form if not submitted -->
            <form action="/script/get_post.php" method="POST">
                <label for="param1">Param1:</label>
                <input type="text" id="param1" name="param1" placeholder="Enter value for Param1">

                <label for="param2">Param2:</label>
                <input type="text" id="param2" name="param2" placeholder="Enter value for Param2">

                <input type="submit" value="Submit">
            </form>
            <?php
        }
        ?>
    </div>
</body>
</html>
