#pragma once
#include "../HttpMessage/HttpMessage.hpp"

enum State {
    // Request Line States
    METHOD_START,             // Start parsing method
    METHOD_PARSING,           // Parsing method characters
    FIRST_SP,              // End of method, expect space
    URI_START,               // Start parsing URI 
    URI_PATH_PARSING,        // Parsing raw URI path characters (no query handling)
    DECODE_URI,              // decode chars in the URI like %20 is a ' '
    URI_SKIP_QUERY_OR_FRAGMENT, // skip all chars after ? or #
    SECOND_SP,                 // End of URI, expect space
    VERSION_HTTP,               // Expect  HTTP/1.1
    REQUEST_LINE_CR,         // Expect CR
    REQUEST_LINE_LF,         // Expect LF

    // Header Parsing States
    HEADER_START,            // Start a new header
    HEADER_NAME,             // Parsing header name
    HEADER_COLON,            // Expect colon after header name
    HEADER_SPACE_AFTER_COLON, // Optional space after colon
    HEADER_VALUE_START,
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
    BODY_PART_HEADERLF2,
    BODY_PART_HEADERCR2,
    BODY_PART_HEADER_START, // Start of part headers
    BODY_PART_HEADER,       // Parsing part header
    BODY_PART_HEADER_CR,    // Expect CR after part header
    BODY_PART_HEADER_LF,    // Expect LF after part header
    BODY_PART_HEADER_CR2,    // Expect CR after part header
    BODY_PART_HEADER_LF2,    // Expect LF after part header
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
    std::string currentHeaderName;
    std::string currentHeaderValue;
    bool isChunked; // init to false
    bool isMultipart; // init to false
    int contentLength; // init to -1
    std::string boundary;
    int chunkSize;  // init to 0;
    int chunkbytesread; // init to 0
    std::map<State, StateHandler> stateHandlers;
    std::map<State, int> errorState;
    std::map<std::string, std::string> formFields;
    std::string fieldName;
    static const int  MAX_URI_LENGTH = 2048;
    static bool isValidPathChar(uint8_t byte);
    bool    uriBehindRoot();
    static bool isValidHeaderNameChar(uint8_t byte);
    void addCurrentHeader();
    void handleTransfer();
    bool isValidMultipart(std::string content);
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
    // void    handleInitialized(uint8_t byte);
    void    handleMethodStart(uint8_t byte);
    void    handleMethodParsing(uint8_t byte);
    void    handleFirstSP(uint8_t byte);
    void    handleURIStart(uint8_t byte);
    void    handleURIPathParsing(uint8_t byte);
    void    handleDecodeURI(uint8_t byte);
    void    handleSkipQF(uint8_t byte);
//    void    handleSecondSP(uint8_t byte);
    void    handleVersionHTTP(uint8_t byte);
    void    handleRequestLineCR(uint8_t byte);
    void    handleRequestLineLF(uint8_t byte);
    
    // Header handling methods
    void    handleHeaderStart(uint8_t byte);
    void    handleHeaderName(uint8_t byte);
//    void    handleHeaderColon(uint8_t byte);
    void    handleHeaderValueStart(uint8_t byte);
    void    handleHeaderValue(uint8_t byte);
//    void    handleHeaderCR(uint8_t byte);
    void    handleHeaderLF(uint8_t byte);
//    void    handleHeadersEndCR(uint8_t byte);
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
//    void    handleChunkDataCR(uint8_t byte);
    void    handleChunkDataLF(uint8_t byte);
//    void    handleChunkTrailerStart(uint8_t byte);
    void    handleChunkTrailerCR(uint8_t byte);
    void    handleChunkTrailerLF(uint8_t byte);
    
    // Multipart body handling methods
    void    handleBodyBoundaryStart(uint8_t byte);
    void    handleBodyBoundaryParsing(uint8_t byte);
//    void    handleBodyBoundaryCR(uint8_t byte);
    void    handleBodyBoundaryLF(uint8_t byte);
    void    handleBodyBoundaryCR2(uint8_t byte);
    void    handleBodyBoundaryLF2(uint8_t byte);
//    void    handleBodyPartHeaderStart(uint8_t byte);
    void    handleBodyPartHeader(uint8_t byte);
//    void    handleBodyPartHeaderCR(uint8_t byte);
    void    handleBodyPartHeaderLF(uint8_t byte);
    void    handleBodyPartHeaderCR2(uint8_t byte);
    void    handleBodyPartHeaderLF2(uint8_t byte);
    void    handleBodyPartData(uint8_t byte);
    void    handleBodyPartEnd(uint8_t byte);

    // Utility methods for data handling
    void storeBodyByte(uint8_t byte);
    void processChunkSize(uint8_t byte);
    bool isValidBoundary() const;
    void processBoundary(uint8_t byte);
    bool isEndBoundary() const;
};
