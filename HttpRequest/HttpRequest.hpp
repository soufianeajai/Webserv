#pragma once
#include "../HttpMessage/HttpMessage.hpp"
#include "../Route/Route.hpp"

enum Status {INITIAL, READING_PARSING, PROCESSING, GENARATE_RESPONSE, SENDING_RESPONSE, DONE};
enum State {
// FIRST LINE STATES
    METHOD_START,
    METHOD_PARSING,
    FIRST_SP,
    URI_START,
    URI_PATH_PARSING,
    DECODE_URI,
    URI_HANDLE_QUERY,
    VERSION_HTTP,
    REQUEST_LINE_CR,
    REQUEST_LINE_LF,

// HEADERS STATES
    HEADER_START,
    HEADER_NAME,
    HEADER_VALUE_START,
    HEADER_VALUE,
    HEADER_LF,
    HEADERS_END_LF,
    
    BODY_START,
    BODY_CONTENT_LENGTH,
    
// CUNKED BODY STATES
    CHUNK_SIZE_START,
    CHUNK_SIZE,
    CHUNK_SIZE_LF,
    CHUNK_DATA,
    CHUNK_DATA_LF,
    CHUNK_TRAILER_CR,
    CHUNK_TRAILER_LF,
    
// MULTIPART/FORM-DATA BODY STATE HANDLERS
    BODY_BOUNDARY_START,
    BODY_BOUNDARY_PARSING,
    BODY_BOUNDARY_LF,
    BODY_PART_HEADER_NAME,
    BODY_PART_HEADER_VALUE,
    BODY_PART_HEADER_LF,
    BODY_PART_HEADER_LF2,
    BODY_PART_DATA,
    BODY_PART_END,
    
    MESSAGE_COMPLETE,
    
// ERRORS STATES
    ERROR_BAD_REQUEST,
    ERROR_FORBIDDEN,
    ERROR_INVALID_METHOD,
    ERROR_INVALID_URI,
    ERROR_INVALID_VERSION,
    ERROR_INVALID_HEADER,
    ERROR_CONTENT_LENGTH,
    ERROR_CHUNK_SIZE,
    ERROR_BOUNDARY,
    ERROR_METHOD_NOT_ALLOWED,
    ERROR_INTERNAL_ERROR,
    ERROR_NOT_FOUND,
    ERROR_FILE_TOO_LARGE,

// PROCESS REQUEST
    PROCESS_URI,
    PROCESS_GET,
    PROCESS_DELETE,
    PROCESS_POST,
    PROCESS_POST_DATA,
    PROCESS_CHUNKED_BODY,
    PROCESS_MULTIPART_FORM_DATA,
    PROCESS_DONE
};

typedef struct s_boundaryPart
{
    std::string name;
    std::string value;
    std::string fileName;
    bool        isFile;
    std::vector<uint8_t> fileBody;
    std::map<std::string, std::string> boundaryHeader;
} boundaryPart;

class HttpRequest :  public HttpMessage{
private:
    typedef void (HttpRequest::*StateHandler)(uint8_t);
    std::map<State, StateHandler> stateHandlers;
    std::map<State, int>          errorState;
    std::vector<boundaryPart>     parts;
    Route           CurrentRoute;
    State           currentState;
    std::string     method;
    std::string     uri;
    int             statusCode;
    std::string     holder;
    std::string     currentHeaderName;
    std::string     currentHeaderValue;
    bool            isChunked;
    bool            isMultipart;
    size_t          contentLength;
    std::string     boundary;
    int             chunkSize;
    int             chunkbytesread;
    StateHandler    currentHandler;
    std::string     query;
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
    void    handleProcessUri_Method(std::map<std::string, Route>& routes);
    void    handleProcessDelete();
    void    handleProcessPost();
    void    handleProcessPostData();
    void    handleProcessMultipart();
    void    saveDataToFile(std::string name, std::vector<uint8_t>& body);
// PARSER UTILS 
    bool    isValidPathChar(uint8_t byte);
    bool    uriBehindRoot();
    bool    isValidHeaderNameChar(uint8_t byte);
    void    addCurrentHeader();
    void    handleTransfer();
    bool    isValidMultipart(std::string content);
public:
    HttpRequest();
    Route& getCurrentRoute();
    std::vector<uint8_t>& GetBody();
    std::map<std::string, std::string>& getheaders();
    void    parse(uint8_t *buffer, int readSize, size_t limitBodySize);
    void    process(std::map<std::string, Route>& routes);
    void    setMethod(const std::string methodStr);
    void    setUri(const std::string uri);
    bool    parsingCompleted() const;
    bool    errorOccured() const;
    std::string& getMethod() ;
    std::string& getUri()  ;
    int GetStatusCode() const;
    void SetStatusCode(int status);
    std::string getQuery() const;
    State getcurrentState() const;
    std::string& getHeader(std::string key);
    std::map<State, int>& getErrorState() ;
    
};
