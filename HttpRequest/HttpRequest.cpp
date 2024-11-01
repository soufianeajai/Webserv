#include "HttpRequest.hpp"
#include "Connection.hpp"

const std::map<State, HttpRequest::StateHandler> HttpRequest::stateHandlers = {
    {State::INITIALIZED, &HttpRequest::handleInitialized},
//Sart line
    {State::METHOD_START, &HttpRequest::handleMethodStart},
    {State::METHOD_PARSING, &HttpRequest::handleMethodParsing},
    {State::FIRST_SP, &HttpRequest::handleFirstSP},
    {State::URI_START, &HttpRequest::handleURIStart},
    {State::URI_PATH_PARSING, &HttpRequest::handleURIPathParsing},
    {State::DECODE_URI, &HttpRequest::handleDecodeURI},
    {State::URI_SKIP_QUERY_OR_FRAGMENT, &HttpRequest::handleSkipQF},
    {State::SECOND_SP, &HttpRequest::handleSecondSP},
    {State::VERSION_HTTP, &HttpRequest::handleVersionHTTP},
    {State::REQUEST_LINE_CR, &HttpRequest::handleRequestLineCR},
    {State::REQUEST_LINE_LF, &HttpRequest::handleRequestLineLF},
// header
    {State::HEADER_START, &HttpRequest::handleHeaderStart},
    {State::HEADER_NAME, &HttpRequest::handleHeaderName},
    {State::HEADER_COLON, &HttpRequest::handleHeaderColon},
    {State::HEADER_SPACE_AFTER_COLON, &HttpRequest::handleHeaderValueStart},
    {State::HEADER_VALUE, &HttpRequest::handleHeaderValue},
    {State::HEADER_CR, &HttpRequest::handleHeaderCR},
    {State::HEADER_LF, &HttpRequest::handleHeaderLF},
    {State::HEADERS_END_CR, &HttpRequest::handleHeadersEndCR},
    {State::HEADERS_END_LF, &HttpRequest::handleHeadersEndLF},
// body
    {State::BODY_START, &HttpRequest::handleBodyStart},
    {State::BODY_CONTENT_LENGTH, &HttpRequest::handleBodyContentLength},
//chunked body
    {State::CHUNK_SIZE_START, &HttpRequest::handleChunkSizeStart},
    {State::CHUNK_SIZE, &HttpRequest::handleChunkSize},
    {State::CHUNK_SIZE_CR, &HttpRequest::handleChunkSizeCR},
    {State::CHUNK_SIZE_LF, &HttpRequest::handleChunkSizeLF},
    {State::CHUNK_DATA, &HttpRequest::handleChunkData},
    {State::CHUNK_DATA_CR, &HttpRequest::handleChunkDataCR},
    {State::CHUNK_DATA_LF, &HttpRequest::handleChunkDataLF},
    {State::CHUNK_TRAILER_START, &HttpRequest::handleChunkTrailerStart},
    {State::CHUNK_TRAILER_CR, &HttpRequest::handleChunkTrailerCR},
    {State::CHUNK_TRAILER_LF, &HttpRequest::handleChunkTrailerLF},
// multipart body
    {State::BODY_BOUNDARY_START, &HttpRequest::handleBodyBoundaryStart},
    {State::BODY_BOUNDARY_PARSING, &HttpRequest::handleBodyBoundaryParsing},
    {State::BODY_BOUNDARY_CR, &HttpRequest::handleBodyBoundaryCR},
    {State::BODY_BOUNDARY_LF, &HttpRequest::handleBodyBoundaryLF},
    {State::BODY_PART_HEADER_START, &HttpRequest::handleBodyPartHeaderStart},
    {State::BODY_PART_HEADER, &HttpRequest::handleBodyPartHeader},
    {State::BODY_PART_HEADER_CR, &HttpRequest::handleBodyPartHeaderCR},
    {State::BODY_PART_HEADER_LF, &HttpRequest::handleBodyPartHeaderLF},
    {State::BODY_PART_DATA, &HttpRequest::handleBodyPartData},
    {State::BODY_PART_END, &HttpRequest::handleBodyPartEnd}
};
const std::map<State, int> HttpRequest::errorState{
    {State::ERROR_BAD_REQUEST, 400},
    {State::ERROR_INVALID_METHOD, 501},
    {State::ERROR_INVALID_URI, 400},
    {State::ERROR_INVALID_VERSION, 505},
    {State::ERROR_INVALID_HEADER, 400},
    {State::ERROR_CONTENT_LENGTH, 411},
    {State::ERROR_CHUNK_SIZE, 400},
    {State::ERROR_BOUNDARY, 400},
    {State::ERROR_INCOMPLETE, 400},
    {State::ERROR_BUFFER_OVERFLOW, 400},
    {State::ERROR_BINARY_DATA, 415},
    {State::REQUEST_URI_TOO_LONG, 414}
};
HttpRequest::HttpRequest():HttpMessage(), method(""), uri(""), currentState(INITIALIZED), statusCode(200){};

void HttpRequest::handleInitialized(uint8_t byte){
    currentState = METHOD_START;
}

void HttpRequest::handleMethodStart(uint8_t byte) {
    if (byte == 'G' || byte == 'P' || byte == 'D') {
        method = byte;
        currentState = State::METHOD_PARSING;
    }
    else
        currentState =  State::ERROR_INVALID_METHOD;
}
void HttpRequest::handleMethodParsing(uint8_t byte) {
    std::string expectedMethod;
    if (method[0] == 'G')
        expectedMethod = "GET";
    else if (method[0] == 'P')
        expectedMethod = "POST";
    else
        expectedMethod = "DELETE";
    size_t methodNextByte = method.length();
    if (methodNextByte < expectedMethod.length()) {
        if (byte == expectedMethod[methodNextByte]) {
            method += byte; 
            if (method == expectedMethod)
                currentState = State::FIRST_SP;
            return;
        }
    }    
    currentState = State::ERROR_INVALID_METHOD;
}

void HttpRequest::handleFirstSP(uint8_t byte) {
    if (byte == ' ')
        currentState = State::URI_START;
    else
        currentState = State::ERROR_BAD_REQUEST;
}
void HttpRequest::handleURIStart(uint8_t byte) {
    if (byte == '/') {
        currentState = State::URI_PATH_PARSING;
        uri += byte;
    }
    else
        currentState = State::ERROR_INVALID_URI;
}
void HttpRequest::handleURIPathParsing(uint8_t byte) {
// Check for end of URI (space before HTTP version)
    if (byte == ' ')
        currentState = State::VERSION_HTTP;
// Check max length or invalid char
    else if (uri.length() >= HttpRequest::MAX_URI_LENGTH ||!isValidPathChar(byte) )
        currentState = State::ERROR_INVALID_URI;
// check for consecutive // in the uri if so ... do nothing 
    else if (byte == '/' && !uri.empty() && uri.back() == '/')
        currentState = State::URI_PATH_PARSING;
    else if (byte == '%')
    {
        holder.clear();
        currentState = State::DECODE_URI;
    }
    else if (byte == '?' || byte == '#') {
        currentState = State::URI_SKIP_QUERY_OR_FRAGMENT;
    }
    else
        uri += byte;
}

void HttpRequest::handleSkipQF(uint8_t byte) {
    if (byte == ' ')
        currentState = State::VERSION_HTTP; 
}
void HttpRequest::handleDecodeURI(uint8_t byte) {
    if (holder.length() < 2 && std::isxdigit(byte)) {
        holder += byte;
        if (holder.length() == 2)
        {
            int value = std::stoi(holder, NULL, 16); 
            uri += static_cast<char>(value);
            holder.clear();
            currentState = State::URI_PATH_PARSING;
        }
    }
    else if (!std::isxdigit(byte))
        currentState = State::ERROR_INVALID_URI;
}

void HttpRequest::handleVersionHTTP(uint8_t byte) {
    std::string expectedHttpVersion = "HTTP/1.1";
    size_t versiondNextByte = version.length();
    if (uriBehindRoot())
        currentState = State::ERROR_BAD_REQUEST;
    else if (versiondNextByte < expectedHttpVersion.length()) {
        if (byte == expectedHttpVersion[versiondNextByte]) {
            version += byte; 
            if (version == expectedHttpVersion)
                currentState = State::REQUEST_LINE_CR;
            return;
        }
    }    
    currentState = State::ERROR_INVALID_VERSION;
}

void HttpRequest::handleRequestLineCR(uint8_t byte) {
    if (byte == '\r')
        currentState = State::REQUEST_LINE_LF;
    else
        currentState = State::ERROR_BAD_REQUEST;
}
void HttpRequest::handleRequestLineLF(uint8_t byte) {
    if (byte == '\n')
        currentState = State::HEADER_START;
    else
        currentState = State::ERROR_BAD_REQUEST;
}

void HttpRequest::handleHeaderStart(uint8_t byte) {
    if (!isValidHeaderNameChar(byte))
        State::ERROR_INVALID_HEADER;
    else if (byte == '\r')
        State::HEADERS_END_LF;
    else {
        currentHeaderName += byte;
        State::HEADER_NAME;
    }

}

void HttpRequest::handleHeaderName(uint8_t byte) {
    if (byte == ':')
        currentState = State::HEADER_VALUE_START;
    else if (!isValidHeaderNameChar(byte))
        currentState = State::ERROR_BAD_REQUEST;
    else
        currentHeaderName += byte;
}


void HttpRequest::handleHeaderValueStart(uint8_t byte) {
    if (byte == '\r')
        currentState = State::HEADER_LF;
    else {
        currentHeaderValue += byte;
        currentState = State::HEADER_VALUE;
    }    
}

void HttpRequest::handleHeaderValue(uint8_t byte) {
    if (byte == '\r')
        currentState = State::HEADER_LF;
    else
        currentHeaderValue += byte;
}

void HttpRequest::handleHeaderLF(uint8_t byte) {
    if (byte == '\n') {
        addCurrentHeader();
        currentState = State::HEADER_START;
    }
    else {
        currentState = State::ERROR_BAD_REQUEST;
    }    
}
void HttpRequest::handleHeadersEndLF(uint8_t byte) {
    if (byte == '\n' && (headers.find("Content-Length") == headers.end() && headers.find("Transfer-Encoding") == headers.end()))
        currentState = State::MESSAGE_COMPLETE;
    else if (byte == '\n')
        currentState = State::BODY_START;
    else
        currentState = State::ERROR_BAD_REQUEST;
}
void HttpRequest::handleTransfer(){
    if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked")
        isChunked = true;
    if (headers.find("Content-Length") != headers.end()){
        char c = 0;
        std::istringstream nbr(headers["Content-Length"]);
        nbr >> contentLength >> c;
        if (contentLength <= 0 || c)
            contentLength = -1;            
    }
    if (headers.find("Content-Type") != headers.end() && isValidMultipart(headers["Content-Type"])){
        isMultipart = true;
    }
    
}
    /*
    4\r\n                  # Chunk size (4 bytes)
    Wiki\r\n               # Chunk data
    5\r\n                  # Chunk size (5 bytes)
    pedia\r\n             # Chunk data
    0\r\n                 # Final chunk (size 0)
    X-Custom: value\r\n   # Optional trailer
    \r\n                  # End of trailers
    */

/*
-----------------------------735323031399963618057233701
Content-Disposition: form-data; name="username"

johndoe
-----------------------------735323031399963618057233701
Content-Disposition: form-data; name="email"

john.doe@example.com
-----------------------------735323031399963618057233701
*/
void HttpRequest::handleBodyStart(uint8_t byte) {
    handleTransfer();
    if (isChunked)
        currentState = State::CHUNK_SIZE_START;
    else if (isMultipart)
        currentState = State::BODY_BOUNDARY_START;
    else if (contentLength > 0)
        currentState = State::BODY_CONTENT_LENGTH;
    else
        currentState = State::MESSAGE_COMPLETE;
}
void HttpRequest::handleBodyBoundaryStart(uint8_t byte) {
    if (byte == '-') {
        holder += byte;
        currentState = State::BODY_BOUNDARY_PARSING;
    }
    else
        currentState = State::ERROR_BOUNDARY;
}

void HttpRequest::handleBodyBoundaryParsing(uint8_t byte) {
    if (byte == '\r') {
        std::string expectedBoundary = "--" + boundary;
        if (holder == expectedBoundary) {
            currentState = State::BODY_BOUNDARY_LF;
        }
        else if (holder == expectedBoundary + "--")
            currentState = State::MESSAGE_COMPLETE;
        else
            currentState = State::ERROR_BOUNDARY;
    }
    else {
        holder += byte;
        if (holder.length() > boundary.length() + 4)
            currentState = State::ERROR_BOUNDARY;
    }   
}

void HttpRequest::handleBodyBoundaryLF(uint8_t byte) {
    if (byte == '\n') {
        currentState = State::BODY_PART_HEADER_START;
        holder.clear();
    }
    else
        currentState = State::ERROR_BOUNDARY;
}


void HttpRequest::handleBodyPartHeader(uint8_t byte) {
    if (byte == '\r') {
        currentState = State::BODY_PART_HEADER_LF;
    }
    else if (isValidHeaderNameChar(byte)) {
        holder = byte;
        currentState = State::BODY_PART_HEADER;
    }
    else
        currentState = State::ERROR_BOUNDARY;
}

void HttpRequest::handleBodyPartHeaderLF(uint8_t byte) {
    std::size_t namePos = holder.find("name=\"");
    if (namePos != std::string::npos) {
        namePos += 6;
        std::size_t endPos = holder.find("\"", namePos);
        if (endPos != std::string::npos)
            fieldName = holder.substr(namePos, endPos - namePos);
        else {
            currentState = State::ERROR_BOUNDARY;
            return;
        }
    }
    if (byte == '\n')
        currentState = State::BODY_PART_DATA;
    else
        currentState = State::ERROR_BOUNDARY;
}

void HttpRequest::handleBodyPartData(uint8_t byte) {
    if (byte == '\r')
        currentState = State::BODY_PART_END;
    else if (!holder.empty())
        formFields[holder] += byte;
    else
        currentState = State::ERROR_BOUNDARY;
}

void HttpRequest::handleBodyPartEnd(uint8_t byte) {
    if (byte == '\n') {
        holder.clear();
        currentState = State::BODY_BOUNDARY_START;
    }
    else
        currentState = State::ERROR_BOUNDARY;
}




void HttpRequest::handleChunkSizeStart(uint8_t byte) {
    chunkbytesread = 0;
    chunkSize = 0;
    holder.clear();
    if (std::isxdigit(byte)) {
        holder += byte;
        currentState = State::CHUNK_SIZE;
    }
    else
        currentState = State::ERROR_CHUNK_SIZE;
}

void HttpRequest::handleChunkSize(uint8_t byte) {
    if (std::isxdigit(byte)) {
        holder += byte;
        currentState = State::CHUNK_SIZE_CR;
    }
    else if (byte == '\r')
        currentState = State::CHUNK_SIZE_LF;
    else
        currentState = State::ERROR_CHUNK_SIZE;
}
void HttpRequest::handleChunkSizeCR(uint8_t byte) {
    if (byte == '\r')
        currentState = State::CHUNK_SIZE_LF;
    else
        currentState = State::ERROR_CHUNK_SIZE;
}

void HttpRequest::handleChunkSizeLF(uint8_t byte) {
    if (byte == '\n')
    {
        chunkSize = std::stoi(holder, NULL, 16);
        if (chunkSize == 0)
            currentState = State::CHUNK_TRAILER_CR;
        else
            currentState = State::CHUNK_DATA;

    }
    else
        currentState = State::ERROR_CHUNK_SIZE;
}
void HttpRequest::handleChunkTrailerCR(uint8_t byte) {
    if (byte == '\r')
        currentState = State::CHUNK_TRAILER_LF;
    else
        currentState = State::ERROR_CHUNK_SIZE;
}
void HttpRequest::handleChunkTrailerLF(uint8_t byte) {
    if (byte == '\n')
        currentState = State::MESSAGE_COMPLETE ;
    else
        currentState = State::ERROR_CHUNK_SIZE;
}

void HttpRequest::handleChunkData(uint8_t byte) {
    if (byte == '\r')
        currentState = State::CHUNK_DATA_LF;
    else if (chunkbytesread > chunkSize)
        currentState = State::ERROR_CHUNK_SIZE;
    else
    {
        body.push_back(byte);
        chunkbytesread++;
    }
}

void HttpRequest::handleChunkDataLF(uint8_t byte) {
    if (byte == '\n')
        currentState = State::CHUNK_SIZE_START;
    else
        currentState = State::ERROR_CHUNK_SIZE;  
}






bool HttpRequest::isValidMultipart(std::string content) {
// Format : Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW
    std::size_t boundaryPos = content.find("boundary=");
    if (boundaryPos != std::string::npos && boundaryPos == content.find("multipart/form-data; ") + 21)
    {
        boundary = content.substr(boundaryPos + 9);
        return true;
    }
    else
        boundary.clear();
    return false;
}

bool HttpRequest::isValidHeaderNameChar(uint8_t byte) {
    return std::isalnum(byte) || byte == '-' || byte == '_';
}


void HttpRequest::addCurrentHeader() {
    if (!currentHeaderName.empty()) {
        size_t start = currentHeaderValue.find_first_not_of(" \t");
        size_t end = currentHeaderValue.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            currentHeaderValue = currentHeaderValue.substr(start, end - start + 1);
        }
        headers[currentHeaderName] = currentHeaderValue;
        currentHeaderName.clear();
        currentHeaderValue.clear();
    }
}








void HttpRequest::parse(uint8_t *buffer, int readSize) {
    for(int i = 0; i < readSize && !errorOccured(); i++)
    {
        (this->*currentHandler)(buffer[i]);
        std::map<State, StateHandler>::const_iterator it = stateHandlers.find(currentState);
        if (it != stateHandlers.end())
            currentHandler = it->second;
        else
            break;
    }
    if (errorOccured())
    {
        std::map<State, int>::const_iterator it = errorState.find(currentState);
            statusCode = it->second;
    }
}


void HttpRequest::setMethod(const std::string methodStr){
    method = methodStr;
};
std::string HttpRequest::getMethod() const{
    return method;
};
void HttpRequest::setUri(const std::string inputUrl){
    uri = inputUrl;
};
std::string HttpRequest::getUri() const{
    return uri;
};
void HttpRequest::reset(){
    resetMessage();
    method.clear();
    uri.clear();
    currentState = INITIALIZED;
    statusCode = 200;
};
bool HttpRequest::parsingCompleted() const {
    return currentState == MESSAGE_COMPLETE;
}
bool HttpRequest::errorOccured() const {
    return (currentState == ERROR_INVALID_METHOD || currentState == ERROR_INVALID_URI
    || currentState == ERROR_INVALID_VERSION || currentState == ERROR_INVALID_HEADER
    || currentState == ERROR_CONTENT_LENGTH || currentState == ERROR_CHUNK_SIZE
    || currentState == ERROR_BOUNDARY || currentState == ERROR_INCOMPLETE
    || currentState == ERROR_BUFFER_OVERFLOW || currentState == ERROR_BINARY_DATA
    || currentState == ERROR_BAD_REQUEST);
}
/*
    RFC 7230 Sec 3.1.1: Any VCHAR except delimiters
    Valid: A-Z, a-z, 0-9, -, ., _, ~, !, $, &, ', (, ), *, +, ,, ;, =, :, @
    No spaces, angle brackets, square brackets, curly brackets, quotes or backslash, ^ or `
*/
bool HttpRequest::isValidPathChar(uint8_t byte) {
    return (byte >= 33 && byte <= 126) &&
        byte != ' ' &&
        byte != '<' && byte != '>' &&
        byte != '[' && byte != ']' &&
        byte != '{' && byte != '}' &&
        byte != '"' && byte != '\\' &&
        byte != '^' && byte != '`';      
    }
bool HttpRequest::uriBehindRoot() {
    std::istringstream stream(uri);
    std::string segment;
    int pos = 0;

    while (std::getline(stream, segment, '/'))
    {
        if (segment == "..")
            pos--;
        else if (!segment.empty() && segment != ".")
            pos++;
    }
    return (pos < 0);
}





/*
# Full HTTP Request with Multipart Form Data

# Request Line (RFC 7230 Section 3.1.1)
POST /upload HTTP/1.1
# Host Header (RFC 7230 Section 5.4)
Host: example.com
# User Agent to identify client (RFC 7231 Section 5.5.3)
User-Agent: Mozilla/5.0 (Compatible Multipart Client)
# Content Type specifying multipart form data (RFC 7578 Section 4.2)
Content-Type: multipart/form-data; boundary=---------------------------735323031399963618057233701

# Request Body (RFC 7578 defines multipart/form-data format)
-----------------------------735323031399963618057233701
Content-Disposition: form-data; name="username"

johndoe
-----------------------------735323031399963618057233701
Content-Disposition: form-data; name="email"

john.doe@example.com
-----------------------------735323031399963618057233701
Content-Disposition: form-data; name="profile_picture"; filename="profile.jpg"
Content-Type: image/jpeg

[Binary image data would be here]
-----------------------------735323031399963618057233701--

# Breakdown of Key Components:

# 1. Request Line
# - Method: POST (indicates resource modification/creation)
# - Path: /upload (endpoint for handling form submission)
# - HTTP Version: 1.1 (current standard protocol version)

# 2. Headers
# - Host: Specifies the domain name of the server
# - User-Agent: Identifies the client software
# - Content-Type: Crucial for multipart form data
#   * multipart/form-data indicates multiple parts
#   * boundary is a unique marker separating form parts

# 3. Multipart Form Data Structure
# - Each part starts with the boundary
# - Content-Disposition header for each part
# - Optional Content-Type for file uploads
# - Empty line separates headers from content
# - Final boundary ends with '--'

# Typical Server-Side Processing Flow:
# 1. Parse Content-Type to extract boundary
# 2. Split request body using boundary
# 3. Process each part:
#    - Extract name from Content-Disposition
#    - Handle text fields or file uploads
#    - Validate and process data
*/