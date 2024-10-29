#pragma once
#include "HttpMessage.hpp"

enum State {
    INITIALIZED,              // Parser just initialized or reset

    // Request Line States
    METHOD_START,             // Start parsing method
    METHOD_PARSING,           // Parsing method characters
    FIRST_SP,              // End of method, expect space
    URI_START,               // Start parsing URI 
    URI_PATH_PARSING,        // Parsing raw URI path characters (no query handling)
    SECOND_SP,                 // End of URI, expect space
    VERSION_H,               // Expect 'H' of HTTP/
    VERSION_T1,              // Expect first 'T' of HTTP/
    VERSION_T2,              // Expect second 'T' of HTTP/
    VERSION_P,               // Expect 'P' of HTTP/
    VERSION_SLASH,           // Expect '/' after HTTP
    VERSION_MAJOR,           // Parsing major version number
    VERSION_DOT,             // Expect '.' between major and minor
    VERSION_MINOR,           // Parsing minor version number
    REQUEST_LINE_CR,         // Expect CR
    REQUEST_LINE_LF,         // Expect LF

    // Header Parsing States
    HEADER_START,            // Start a new header
    HEADER_NAME,             // Parsing header name
    HEADER_COLON,            // Expect colon after header name
    HEADER_SPACE_AFTER_COLON, // Optional space after colon
    HEADER_VALUE,            // Parsing header value
    HEADER_CR,               // Expect CR after header value
    HEADER_LF,               // Expect LF after CR
    HEADERS_END_CR,          // Expect CR (end of headers)
    HEADERS_END_LF,          // Expect LF (end of headers)
    
    // Body Parsing States with Binary Support
    BODY_START,              // Start parsing body
    
    // Content-Length Based Body
    BODY_CONTENT_LENGTH,     // Reading exact number of bytes
    
    // Chunked Transfer States
    CHUNK_SIZE_START,        // Start of chunk size line
    CHUNK_SIZE,              // Reading chunk size (hex)
    CHUNK_SIZE_CR,           // Expect CR after chunk size
    CHUNK_SIZE_LF,          // Expect LF after chunk size CR
    CHUNK_DATA,             // Reading chunk data (binary safe)
    CHUNK_DATA_CR,          // Expect CR after chunk data
    CHUNK_DATA_LF,          // Expect LF after chunk data
    CHUNK_TRAILER_START,    // Start of optional trailers
    CHUNK_TRAILER_CR,       // Expect CR in trailer
    CHUNK_TRAILER_LF,       // Expect LF in trailer
    
    // Multipart Body States (for binary file uploads)
    BODY_BOUNDARY_START,    // Start of boundary
    BODY_BOUNDARY_PARSING,  // Parsing boundary
    BODY_BOUNDARY_CR,       // Expect CR after boundary
    BODY_BOUNDARY_LF,       // Expect LF after boundary
    BODY_PART_HEADER_START, // Start of part headers
    BODY_PART_HEADER,       // Parsing part header
    BODY_PART_HEADER_CR,    // Expect CR after part header
    BODY_PART_HEADER_LF,    // Expect LF after part header
    BODY_PART_DATA,         // Reading part data (binary safe)
    BODY_PART_END,          // End of current part
    
    // Final States
    MESSAGE_COMPLETE,       // Parsing completed successfully
    
    // Error States
    ERROR_BAD_REQUEST,
    ERROR_INVALID_METHOD,   // Invalid or unsupported method (not GET/POST/DELETE)
    ERROR_INVALID_URI,      // Invalid URI format
    REQUEST_URI_TOO_LONG,    // URI too long
    ERROR_INVALID_VERSION,  // Invalid HTTP version
    ERROR_INVALID_HEADER,   // Malformed header
    ERROR_CONTENT_LENGTH,   // Invalid Content-Length
    ERROR_CHUNK_SIZE,       // Invalid chunk size
    ERROR_BOUNDARY,         // Invalid boundary in multipart
    ERROR_INCOMPLETE,       // Unexpected end of input
    ERROR_BUFFER_OVERFLOW,  // Input exceeds buffer capacity
    ERROR_BINARY_DATA      // Error processing binary data
};

class HttpRequest :  public HttpMessage{
private:
    std::string method;
    std::string uri;
    typedef void (HttpRequest::*StateHandler)(uint8_t);
    StateHandler currentHandler;
    State currentState;
    int    statusCode;
    std::string holder;
    static const std::map<State, StateHandler> stateHandlers;
    static const std::map<State, int> errorState;
    static const int  MAX_URI_LENGTH = 2048;
    // void parseRequestLine(std::string& RequestLine);
    // void parseHeaders(const std::string& headersLine);
    // void parseBody(const std::vector<uint8_t>& bufferBody);
    bool isValidPathChar(uint8_t byte);
    bool    uriBehindRoot();
public:
    HttpRequest();
    void parse(uint8_t *buffer, int readSize);
    void processRequest();
    void setMethod(const std::string methodStr);
    std::string getMethod() const;
    void setUri(const std::string uri);
    std::string getUri() const ;
    void reset();
    bool parsingCompleted() const;
    bool errorOccured() const;

protected:
//Start line
    void    handleInitialized(uint8_t byte);
    void    handleMethodStart(uint8_t byte);
    void    handleMethodParsing(uint8_t byte);
    void    handleFirstSP(uint8_t byte);
    void    handleURIStart(uint8_t byte);
    void    handleURIPathParsing(uint8_t byte);
    void    handleSecondSP(uint8_t byte);
    void    handleVersionH(uint8_t byte);
    void    handleVersionT1(uint8_t byte);
    void    handleVersionT2(uint8_t byte);
    void    handleVersionP(uint8_t byte);
    void    handleVersionSlash(uint8_t byte);
    void    handleVersionMajor(uint8_t byte);
    void    handleVersionDot(uint8_t byte);
    void    handleVersionMinor(uint8_t byte);
    void    handleRequestLineCR(uint8_t byte);
    void    handleRequestLineLF(uint8_t byte);
    
    // Header handling methods
    void    handleHeaderStart(uint8_t byte);
    void    handleHeaderName(uint8_t byte);
    void    handleHeaderColon(uint8_t byte);
    void    handleHeaderSpaceAfterColon(uint8_t byte);
    void    handleHeaderValue(uint8_t byte);
    void    handleHeaderCR(uint8_t byte);
    void    handleHeaderLF(uint8_t byte);
    void    handleHeadersEndCR(uint8_t byte);
    void    handleHeadersEndLF(uint8_t byte);
    
    // Body handling methods
    void    handleBodyStart(uint8_t byte);
    void    handleBodyContentLength(uint8_t byte);
    
    // Chunked transfer handling methods
    void    handleChunkSizeStart(uint8_t byte);
    void    handleChunkSize(uint8_t byte);
    void    handleChunkSizeCR(uint8_t byte);
    void    handleChunkSizeLF(uint8_t byte);
    void    handleChunkData(uint8_t byte);
    void    handleChunkDataCR(uint8_t byte);
    void    handleChunkDataLF(uint8_t byte);
    void    handleChunkTrailerStart(uint8_t byte);
    void    handleChunkTrailerCR(uint8_t byte);
    void    handleChunkTrailerLF(uint8_t byte);
    
    // Multipart body handling methods
    void    handleBodyBoundaryStart(uint8_t byte);
    void    handleBodyBoundaryParsing(uint8_t byte);
    void    handleBodyBoundaryCR(uint8_t byte);
    void    handleBodyBoundaryLF(uint8_t byte);
    void    handleBodyPartHeaderStart(uint8_t byte);
    void    handleBodyPartHeader(uint8_t byte);
    void    handleBodyPartHeaderCR(uint8_t byte);
    void    handleBodyPartHeaderLF(uint8_t byte);
    void    handleBodyPartData(uint8_t byte);
    void    handleBodyPartEnd(uint8_t byte);

    // Utility methods for data handling
    void storeBodyByte(uint8_t byte);
    void processChunkSize(uint8_t byte);
    bool isValidBoundary() const;
    void processBoundary(uint8_t byte);
    bool isEndBoundary() const;
};
