#include "HttpRequest.hpp"
#include "Connection.hpp"

const std::map<State, HttpRequest::StateHandler> HttpRequest::stateHandlers = {
    {State::INITIALIZED, &HttpRequest::handleInitialized},
//Sart line
    {State::METHOD_START, &HttpRequest::handleMethodStart},
    {State::METHOD_PARSING, &HttpRequest::handleMethodParsing},
    {State::METHOD_END, &HttpRequest::handleMethodEnd},
    {State::URI_START, &HttpRequest::handleURIStart},
    {State::URI_PATH_PARSING, &HttpRequest::handleURIPathParsing},
    {State::URI_END, &HttpRequest::handleURIEnd},
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
HttpRequest::HttpRequest():HttpMessage(), method(""), url(""), currentState(INITIALIZED), statusCode(200){};

void HttpRequest::handleInitialized(uint8_t byte){
    currentState = METHOD_START;
}

void HttpRequest::handleMethodStart(uint8_t byte) {
    if (byte == 'G' || byte == 'P' || byte == 'D') {
        holder = byte;
        currentState = State::METHOD_PARSING;
    }
    else{
        currentState =  State::ERROR_INVALID_METHOD;
        statusCode = 501;
    }
}
void HttpRequest::handleMethodParsing(uint8_t byte) {
    std::string method;
    if (holder[0] == 'G')
        method = "GET";
    else if (holder[0] == 'P')
        method = "POST";
    else
        method = "DELETE";
    size_t holderNextByte = holder.length();
    if (holderNextByte < method.length()) {
        if (byte == method[holderNextByte]) {
            holder += byte; 
            if (holder == method) {
                currentState =  State::METHOD_END;
            }
            currentState =  State::METHOD_PARSING;
        }
    }    
    currentState =  State::ERROR_INVALID_METHOD;
}

void HttpRequest::parse(uint8_t *buffer, int readSize) {
    for(int i = 0; i < readSize && currentHandler && !errorOccured(); i++)
    {
        (this->*currentHandler)(buffer[i]);
        std::map<State, StateHandler>::const_iterator it = stateHandlers.find(currentState);
        if (it != stateHandlers.end())
            currentHandler = it->second;
        else
            currentHandler = NULL;
    }
}


void HttpRequest::setMethod(const std::string methodStr){
    method = methodStr;
};
std::string HttpRequest::getMethod() const{
    return method;
};
void HttpRequest::setUrl(const std::string inputUrl){
    url = inputUrl;
};
std::string HttpRequest::getUrl() const{
    return url;
};
void HttpRequest::reset(){
    resetMessage();
    method = "";
    url = "";
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
    || currentState == ERROR_BUFFER_OVERFLOW || currentState == ERROR_BINARY_DATA);
}