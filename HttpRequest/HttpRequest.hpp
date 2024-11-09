#pragma once
#include "../HttpMessage/HttpMessage.hpp"
#include "../Route/Route.hpp"

enum State {
// FIRST LINE STATES
    METHOD_START,               // Start parsing method
    METHOD_PARSING,             // Parsing method characters
    FIRST_SP,                   // End of method, expect space
    URI_START,                  // Start parsing URI 
    URI_PATH_PARSING,           // Parsing raw URI path characters (no query handling)
    DECODE_URI,                 // decode chars in the URI like %20 is a ' '
    URI_HANDLE_QUERY,           // skip all chars after ? or #
    SECOND_SP,                  // End of URI, expect space
    VERSION_HTTP,               // Expect  HTTP/1.1
    REQUEST_LINE_CR,            // Expect CR
    REQUEST_LINE_LF,            // Expect LF

// HEADERS STATES
    HEADER_START,               // Start a new header
    HEADER_NAME,                // Parsing header name
    HEADER_COLON,               // Expect colon after header name
    HEADER_SPACE_AFTER_COLON,   // Optional space after colon
    HEADER_VALUE_START,         // Getting the first char in the value
    HEADER_VALUE,               // Parsing header value
    HEADER_CR,                  // Expect CR after header value
    HEADER_LF,                  // Expect LF after CR
    HEADERS_END_CR,             // Expect CR (end of headers)
    HEADERS_END_LF,             // Expect LF (end of headers)
    
// BODY STATES
    BODY_START,                 // Start parsing body
// BODY WITH CONTENT LENGTH STATE
    BODY_CONTENT_LENGTH,        // Reading exact number of bytes
    
// CUNKED BODY STATES
    CHUNK_SIZE_START,           // Start of chunk size line
    CHUNK_SIZE,                 // Reading chunk size (hex)
    CHUNK_SIZE_CR,              // Expect CR after chunk size
    CHUNK_SIZE_LF,              // Expect LF after chunk size CR
    CHUNK_DATA,                 // Reading chunk data (binary safe)
    CHUNK_DATA_CR,              // Expect CR after chunk data
    CHUNK_DATA_LF,              // Expect LF after chunk data
    CHUNK_TRAILER_START,        // Start of optional trailers
    CHUNK_TRAILER_CR,           // Expect CR in trailer
    CHUNK_TRAILER_LF,           // Expect LF in trailer
    
// MULTIPART/FORM-DATA BODY STATE HANDLERS
    BODY_BOUNDARY_START,        // Start of boundary
    BODY_BOUNDARY_PARSING,      // Parsing boundary
    BODY_BOUNDARY_CR,           // Expect CR after boundary
    BODY_BOUNDARY_LF,           // Expect LF after boundary
    BODY_PART_HEADER_NAME,
    BODY_PART_HEADER_VALUE,
    BODY_PART_HEADERLF2,        // Expect a second CR after boundary
    BODY_PART_HEADERCR2,        // Expect a second LF after boundary
    BODY_PART_HEADER_START,     // Start of part headers
    BODY_PART_HEADER,           // Parsing part header
    BODY_PART_HEADER_CR,        // Expect CR after part header
    BODY_PART_HEADER_LF,        // Expect LF after part header
    BODY_PART_HEADER_CR2,       // Expect CR after part header
    BODY_PART_HEADER_LF2,       // Expect LF after part header
    BODY_PART_DATA,             // Reading part data (binary safe)
    BODY_PART_END,              // End of current part
    
// FINAL STATE
    MESSAGE_COMPLETE,           // Parsing completed successfully
    
// ERRORS STATES
    ERROR_BAD_REQUEST,          // Bad request
    ERROR_INVALID_METHOD,       // Invalid or unsupported method (not GET/POST/DELETE)
    ERROR_INVALID_URI,          // Invalid URI format
    REQUEST_URI_TOO_LONG,       // URI too long
    ERROR_INVALID_VERSION,      // Invalid HTTP version
    ERROR_INVALID_HEADER,       // Malformed header
    ERROR_CONTENT_LENGTH,       // Invalid Content-Length
    ERROR_CHUNK_SIZE,           // Invalid chunk size
    ERROR_BOUNDARY,             // Invalid boundary in multipart
    ERROR_INCOMPLETE,           // Unexpected end of input
    ERROR_BUFFER_OVERFLOW,      // Input exceeds buffer capacity
    ERROR_BINARY_DATA,           // Error processing binary data
// PROCESS REQUEST
    PROCESS_URI,
    PROCESS_GET,
    PROCESS_DELETE,
    PROCESS_POST,
    PROCESS_BODY,
    PROCESS_CHUNKED_BODY,
    PROCESS_MULTIPART_FORM_DATA,
    PROCESS_DONE
};


typedef struct s_boundaryPart{
    std::string name;
    std::string value;
    std::string fileName;
    bool        isFile;
    std::vector<uint8_t> fileBody;
    std::map<std::string, std::string> boundaryHeader;
} boundaryPart;

class HttpRequest :  public HttpMessage{
private:
    static const int  MAX_URI_LENGTH = 2048;
    typedef void (HttpRequest::*StateHandler)(uint8_t);
    StateHandler    currentHandler;
    State           currentState;
    std::string     method;
    std::string     uri;
    std::string     holder;
    std::string     currentHeaderName;
    std::string     currentHeaderValue;
    std::string     boundary;
    std::string     fieldName;
    std::string     query;
    int             statusCode;
    int             contentLength;
    int             chunkSize;
    int             chunkbytesread;
    bool            isChunked;
    bool            isMultipart;
    std::map<State, StateHandler> stateHandlers;
    std::map<State, int> errorState;
    std::map<std::string, std::string> formFields;
    std::vector<boundaryPart> parts;

public:
    HttpRequest();
    void    parse(uint8_t *buffer, int readSize);
     std::map<std::string, std::string>    process(std::map<std::string, Route>& routes, size_t clientMaxBodySize);
    void    setMethod(const std::string methodStr);
    void    setUri(const std::string uri);
    void    reset();
    bool    parsingCompleted() const;
    bool    errorOccured() const;
    std::string getMethod() const;
    std::string getUri() const ;
private:
// STATE HANDLERS
    void    handleMethodStart(uint8_t byte);
    void    handleMethodParsing(uint8_t byte);
    void    handleFirstSP(uint8_t byte);
    void    handleURIStart(uint8_t byte);
    void    handleURIPathParsing(uint8_t byte);
    void    handleDecodeURI(uint8_t byte);
    void    handleQuery(uint8_t byte);
    void    handleVersionHTTP(uint8_t byte);
    void    handleRequestLineCR(uint8_t byte);
    void    handleRequestLineLF(uint8_t byte);
    void    handleHeaderStart(uint8_t byte);
    void    handleHeaderName(uint8_t byte);
    void    handleHeaderValueStart(uint8_t byte);
    void    handleHeaderValue(uint8_t byte);
    void    handleHeaderLF(uint8_t byte);
    void    handleHeadersEndLF(uint8_t byte);
    void    handleBodyStart(uint8_t byte);
    void    handleBodyContentLength(uint8_t byte);
    void    handleChunkSizeStart(uint8_t byte);
    void    handleChunkSize(uint8_t byte);
    void    handleChunkSizeCR(uint8_t byte);
    void    handleChunkSizeLF(uint8_t byte);
    void    handleChunkData(uint8_t byte);
    void    handleChunkDataLF(uint8_t byte);
    void    handleChunkTrailerCR(uint8_t byte);
    void    handleChunkTrailerLF(uint8_t byte);
    void    handleBodyBoundaryStart(uint8_t byte);
    void    handleBodyBoundaryParsing(uint8_t byte);
    void    handleBodyBoundaryLF(uint8_t byte);
    void    handleBodyPartHeaderName(uint8_t byte);
    void    handleBodyPartHeaderValue(uint8_t byte);
    void    handleBodyPartHeaderLF(uint8_t byte);
    void    handleBodyPartHeaderLF2(uint8_t byte);
    void    handleBodyPartData(uint8_t byte);
    void    handleBodyPartEnd(uint8_t byte);
// PROCESS HANDLERS
    void    handleProcessUri_Method(std::map<std::string, Route>& routes, Route& myRoute);
    void    handleProcessDelete(Route& myRoute);
    void    handleProcessChunkedBody(std::string root);
    void    handleProcessPost();
    void    handleProcessFileUpload();
    void    handleProcessMultipart(std::string root);
// PARSER UTILS 
    bool    isValidPathChar(uint8_t byte);
    bool    uriBehindRoot();
    bool    isValidHeaderNameChar(uint8_t byte);
    void    addCurrentHeader();
    void    handleTransfer();
    bool    isValidMultipart(std::string content);
};
