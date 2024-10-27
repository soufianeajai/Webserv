#pragma once
#include "HttpMessage.hpp"
#include "Buffer.hpp"
enum HttpMethod {
    GET,
    POST,
    DELETE,
    UNKNOWN
};
enum State {
    INITIALIZED,              // Parser just initialized or reset
    // Request Line States
    METHOD_START,             // Start parsing method
    METHOD_PARSING,           // Parsing method characters
    METHOD_END,               // End of method, expect space
    URI_START,                // Start parsing URI
    URI_PARSING,              // Parsing URI characters
    URI_END,                  // End of URI, expect space
    VERSION_START,            // Start parsing version after "HTTP/"
    VERSION_PARSE,            // Parsing "HTTP/1.1" as a whole
    VERSION_MAJOR,            // Parsing major version number
    VERSION_MINOR,            // Parsing minor version number
    REQUEST_LINE_END,         // End of the request line, expect CRLF
    // Header Parsing States
    HEADER_START,             // Start a new header
    HEADER_NAME,              // Parsing header name
    HEADER_COLON,             // Expect colon after header name
    HEADER_VALUE_START,       // Start parsing header value
    HEADER_VALUE,             // Parsing header value characters
    HEADER_END,               // End of each header, expect CRLF
    HEADERS_COMPLETE,         // Final CRLF after headers
    // Body Parsing States
    BODY_START,               // Start parsing body
    BODY_PARSING,             // Parsing body content
    BODY_CONTENT_LENGTH,      // Parsing body with Content-Length
    CHUNK_SIZE_START,         // Start parsing chunk size
    CHUNK_SIZE,               // Parsing chunk size
    CHUNK_EXTENSION,          // Parsing chunk extension (optional)
    CHUNK_DATA,               // Parsing chunk data
    CHUNK_DATA_END,           // End of chunk data, expect CRLF
    CHUNK_END,                // End of a chunk
    BODY_END,                 // End of body
    // END OR ERRORS
    MESSAGE_COMPLETE,         // Entire message parsed successfully
    ERROR_INVALID_METHOD,     // Invalid HTTP method detected
    ERROR_INVALID_URI,        // Invalid URI format
    ERROR_INVALID_VERSION,    // HTTP version format error
    ERROR_INVALID_HEADER,     // Invalid header format
    ERROR_INVALID_BODY,       // Body parsing error
    ERROR_INCOMPLETE_MESSAGE  // Unexpected end of input
};

class HttpRequest :  public HttpMessage{
private:
    HttpMethod method;
    std::string url;
    State currentState;
    void parseRequestLine(std::string& RequestLine);
    void parseHeaders(const std::string& headersLine);
    void parseBody(const std::vector<uint8_t>& body);
public:
    HttpRequest();

    void setMethod(const std::string& methodStr);

    HttpMethod getMethod() const;

    void setUrl(const std::string& url);

    std::string getUrl() const ;
    std::string toString() const;
    void parse(Buffer& requestBuffer);
};

/* rules of parsing in the RFC 7230
request-line   = method SP request-target SP HTTP-version CRLF

*/