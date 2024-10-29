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
    {State::SECOND_SP, &HttpRequest::handleSecondSP},
    {State::VERSION_H, &HttpRequest::handleVersionH},
    {State::VERSION_T1, &HttpRequest::handleVersionT1},
    {State::VERSION_T2, &HttpRequest::handleVersionT2},
    {State::VERSION_P, &HttpRequest::handleVersionP},
    {State::VERSION_SLASH, &HttpRequest::handleVersionSlash},
    {State::VERSION_MAJOR, &HttpRequest::handleVersionMajor},
    {State::VERSION_DOT, &HttpRequest::handleVersionDot},
    {State::VERSION_MINOR, &HttpRequest::handleVersionMinor},
    {State::REQUEST_LINE_CR, &HttpRequest::handleRequestLineCR},
    {State::REQUEST_LINE_LF, &HttpRequest::handleRequestLineLF},
// header
    {State::HEADER_START, &HttpRequest::handleHeaderStart},
    {State::HEADER_NAME, &HttpRequest::handleHeaderName},
    {State::HEADER_COLON, &HttpRequest::handleHeaderColon},
    {State::HEADER_SPACE_AFTER_COLON, &HttpRequest::handleHeaderSpaceAfterColon},
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
        holder = byte;
        currentState = State::METHOD_PARSING;
    }
    else
        currentState =  State::ERROR_INVALID_METHOD;
}
void HttpRequest::handleMethodParsing(uint8_t byte) {
    std::string expectedMethod;
    if (holder[0] == 'G')
        expectedMethod = "GET";
    else if (holder[0] == 'P')
        expectedMethod = "POST";
    else
        expectedMethod = "DELETE";
    size_t holderNextByte = holder.length();
    if (holderNextByte >= expectedMethod.length()) {
        if (byte == expectedMethod[holderNextByte]) {
            holder += byte; 
            if (holder == expectedMethod) {
                currentState = State::FIRST_SP;
                method = expectedMethod;
                holder.clear();
            }
            // else
            //     currentState = State::METHOD_PARSING;
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
        currentState = State::VERSION_H;
// Check max length
    else if (uri.length() >= HttpRequest::MAX_URI_LENGTH)
        currentState = State::ERROR_INVALID_URI;
    else if (!isValidPathChar(byte))
        currentState = State::ERROR_INVALID_URI;
// check for consecutive // in the uri
    else if (byte == '/' && !uri.empty() && uri.back() == '/')
        currentState = State::URI_PATH_PARSING;
    else
        uri += byte;
}
void HttpRequest::handleVersionH(uint8_t byte) {
// check if path is like /../../../root
    if (uriBehindRoot())
        currentState = State::ERROR_BAD_REQUEST;
    else if (byte == 'H')
        currentState = State::VERSION_T1;
    else
        currentState = State::ERROR_BAD_REQUEST;
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
bool    HttpRequest::uriBehindRoot()
{
    std::string tmp(uri);
    char *res = strtok((char *)tmp.c_str(), "/");
    int pos = 0;
    while (res != NULL)
    {
        if (!strcmp(res, ".."))
            pos--;
        else
            pos++;
        if (pos < 0)
            return (1);
        res = strtok(NULL, "/");
    }
    return (0);
}