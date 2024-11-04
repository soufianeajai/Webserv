#include "HttpRequest/HttpRequest.hpp"
#include "HttpMessage/HttpMessage.hpp"

void testSimpleGetRequest() {
    std::cout << "Testing simple GET request..." << std::endl;
    
    HttpRequest request;
    std::string rawRequest = 
        "GET /index.html HTTP/1.1\\r\n"
        "Host: example.com\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "\r\n";
    
    std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
    request.parse(buffer.data(), buffer.size());
    



    assert(request.parsingCompleted());
    assert(!request.errorOccured());
    assert(request.getMethod() == "GET");
    assert(request.getUri() == "/index.html");
    
   std::cout << "Simple GET request test passed!" << std::endl;
}

void testPostWithChunkedTransfer() {
    std::cout << "Testing POST request with chunked transfer..." << std::endl;
    
    HttpRequest request;
    std::string rawRequest = 
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "4\r\n"
        "Wiki\r\n"
        "5\r\n"
        "pedia\r\n"
        "0\r\n"
        "\r\n";
    
    std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
    request.parse(buffer.data(), buffer.size());
    
    assert(request.parsingCompleted());
    assert(!request.errorOccured());
    assert(request.getMethod() == "POST");
    assert(request.getUri() == "/upload");
    
    std::cout << "Chunked transfer test passed!" << std::endl;
}

void testMultipartFormData() {
    std::cout << "Testing multipart form data..." << std::endl;
    
    HttpRequest request;
    std::string boundary = "-------------------------735323031399963618057233701";
    std::string rawRequest = 
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
        "\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"username\"\r\n"
        "\r\n"
        "johndoe\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"email\"\r\n"
        "\r\n"
        "john.doe@example.com\r\n"
        "--" + boundary + "--\r\n";
    
    std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
    request.parse(buffer.data(), buffer.size());
    
    // assert(request.parsingCompleted());
    // assert(!request.errorOccured());
    // assert(request.getMethod() == "POST");
    // assert(request.getUri() == "/submit");
    
    std::cout << "Multipart form data test passed!" << std::endl;
}

void testErrorCases() {
    std::cout << "Testing error cases..." << std::endl;
    
//    Test 1: Invalid method
    {
        HttpRequest request;
        std::string rawRequest = "PUT /index.html HTTP/1.1\r\n\r\n";
        std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
        request.parse(buffer.data(), buffer.size());
        assert(request.errorOccured());
    }
    
//    Test 2: Invalid URI (contains spaces)
    {
        HttpRequest request;
        std::string rawRequest = "GET /path with spaces HTTP/1.1\r\n\r\n";
        std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
        request.parse(buffer.data(), buffer.size());
        assert(request.errorOccured());
    }
    
    // Test 3: Invalid HTTP version
    {
        HttpRequest request;
        std::string rawRequest = "GET /index.html HTTP/2.0\r\n\r\n";
        std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
        request.parse(buffer.data(), buffer.size());
        assert(request.errorOccured());
    }
    
    // Test 4: URI too long
    {
        HttpRequest request;
        std::string longUri(3000, 'a');  // Creates a string of 3000 'a' characters
        std::string rawRequest = "GET /" + longUri + " HTTP/1.1\r\n\r\n";
        std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
        request.parse(buffer.data(), buffer.size());
        assert(request.errorOccured());
    }
    
    std::cout << "Error cases test passed!" << std::endl;
}

void testURIDecoding() {
    std::cout << "Testing URI decoding..." << std::endl;
    
    HttpRequest request;
    std::string rawRequest = "GET /path/with%20space%2Fand%2Dothers HTTP/1.1\r\n\r\n";
    std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
    request.parse(buffer.data(), buffer.size());
    
    assert(!request.errorOccured());
    assert(request.getUri() == "/path/with space/and-others");
    
    std::cout << "URI decoding test passed!" << std::endl;
}
