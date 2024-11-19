#!/usr/bin/python3

import time

def main():
    # Start the infinite loop
    while True:
        print("Content-Type: text/html")
        print()  # This is required to send the HTTP header
        print("<html><body>")
        print("<h1>This page will run in an infinite loop</h1>")
        print("<p>The script is running indefinitely...</p>")
        print("</body></html>")
        
        # Sleep for a few seconds before continuing the loop
        time.sleep(5)

if __name__ == "__main__":
    main()
