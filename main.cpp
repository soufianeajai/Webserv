#include <iostream>
#include <string>
#include <cctype>
#include <sstream>

std::string decodePercentEncoding(const std::string& input) {
    std::ostringstream decoded;
    
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '%') {
            // Check for valid percent-encoded format
            if (i + 2 < input.length() && std::isxdigit(input[i + 1]) && std::isxdigit(input[i + 2])) {
                // Convert the next two characters from hex to char
                std::string hexStr = input.substr(i + 1, 2);
                char decodedChar = static_cast<char>(std::stoi(hexStr, nullptr, 16));
                decoded << decodedChar;
                i += 2; // Skip the next two characters
            } else {
                // If not valid, just append the '%' and continue
                decoded << '%';
            }
        } else {
            decoded << input[i];
        }
    }
    
    return decoded.str();
}

int main() {
    std::string encodedUrl = "/search?q=hello%20world&lang=en";
    std::string decodedUrl = decodePercentEncoding(encodedUrl);
    std::cout << "Decoded URL: " << decodedUrl << std::endl; // Outputs: /search?q=hello world&lang=en
    return 0;
}
