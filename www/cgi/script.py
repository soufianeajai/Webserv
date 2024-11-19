#!/usr/bin/env python3

import cgi
import os

# Set HTTP headers
print("Content-Type: text/html")
print()  # Blank line to indicate the end of the headers

# Start of the HTML body
print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI Example</title>
</head>
<body>
    <h1>Welcome to Python CGI!</h1>
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
    <pre>
""")

# Parse and print form data (GET or POST)
form = cgi.FieldStorage()
if form:
    for key in form.keys():
        print(f"{key}: {form.getvalue(key)}")
else:
    print("No form data received.")

print("""
    </pre>
</body>
</html>
""")

