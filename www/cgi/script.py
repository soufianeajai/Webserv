#!/usr/bin/env python3

import cgi
import os

# Set HTTP headers
# print("Content-Type: text/html")
# print()  # Blank line to indicate the end of the headers

# Start of the HTML body
print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI Example</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f9f9f9;
            color: #333;
        }
        header {
            background-color: #0078d7;
            color: white;
            padding: 20px;
            text-align: center;
        }
        h1, h2 {
            color: #0078d7;
        }
        pre {
            background-color: #e8e8e8;
            padding: 10px;
            border-radius: 5px;
            overflow-x: auto;
        }
        .container {
            max-width: 800px;
            margin: 20px auto;
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }
        th, td {
            text-align: left;
            padding: 10px;
            border: 1px solid #ddd;
        }
        th {
            background-color: #f4f4f4;
        }
        .no-data {
            text-align: center;
            color: #666;
            font-style: italic;
        }
    </style>
</head>
<body>
    <header>
        <h1>Welcome to Python CGI!</h1>
    </header>
    <div class="container">
        <p>This is an example of a Python CGI script.</p>
        <h2>Environment Variables</h2>
        <pre>
""")

# Print environment variables for debugging purposes
for key, value in os.environ.items():
    print(f"{key}={value}")

print("""
        </pre>
        <h2>Form Data</h2>
""")

# Parse and display form data (GET or POST) in a styled table
form = cgi.FieldStorage()
if form:
    print("<table>")
    print("<tr><th>Key</th><th>Value</th></tr>")
    for key in form.keys():
        print(f"<tr><td>{key}</td><td>{form.getvalue(key)}</td></tr>")
    print("</table>")
else:
    print('<p class="no-data">No form data received.</p>')

print("""
    </div>
</body>
</html>
""")
