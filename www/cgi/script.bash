#!/bin/bash

# Output HTML content
echo "<html>"
echo "<head><title>Bash CGI Script</title></head>"
echo "<body>"
echo "<h1>Hello from Bash CGI!</h1>"

# Output environment variables for debugging
echo "<h2>Environment Variables</h2>"
echo "<pre>"
env
echo "</pre>"

# Process form data (if any)
echo "<h2>Form Data</h2>"
if [ -z "$QUERY_STRING" ]; then
  echo "<p>No form data received.</p>"
else
  echo "<pre>"
  echo "Received form data: $QUERY_STRING"
  echo "</pre>"
fi

echo "</body>"
echo "</html>"
