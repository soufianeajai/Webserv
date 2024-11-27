#include "HttpRequest.hpp"

HttpRequest::HttpRequest():HttpMessage(), currentState(METHOD_START), method(""), uri(""),  statusCode(200),
holder(""), currentHeaderName(""), currentHeaderValue(""), isChunked(false), isMultipart(false), contentLength(0),
boundary(""), chunkSize(0), chunkbytesread(0), currentHandler(&HttpRequest::handleMethodStart), fieldName(""), query(""){
// FIRST LINE STATE HANDLERS
    stateHandlers.insert(std::make_pair(METHOD_START, &HttpRequest::handleMethodStart));
    stateHandlers.insert(std::make_pair(METHOD_PARSING, &HttpRequest::handleMethodParsing));
    stateHandlers.insert(std::make_pair(FIRST_SP, &HttpRequest::handleFirstSP));
    stateHandlers.insert(std::make_pair(URI_START, &HttpRequest::handleURIStart));
    stateHandlers.insert(std::make_pair(URI_PATH_PARSING, &HttpRequest::handleURIPathParsing));
    stateHandlers.insert(std::make_pair(DECODE_URI, &HttpRequest::handleDecodeURI));
    stateHandlers.insert(std::make_pair(URI_HANDLE_QUERY, &HttpRequest::handleQuery));
    stateHandlers.insert(std::make_pair(VERSION_HTTP, &HttpRequest::handleVersionHTTP));
    stateHandlers.insert(std::make_pair(REQUEST_LINE_CR, &HttpRequest::handleRequestLineCR));
    stateHandlers.insert(std::make_pair(REQUEST_LINE_LF, &HttpRequest::handleRequestLineLF));
// HEADERS STATE HANDLERS
    stateHandlers.insert(std::make_pair(HEADER_START, &HttpRequest::handleHeaderStart));
    stateHandlers.insert(std::make_pair(HEADER_NAME, &HttpRequest::handleHeaderName));
    stateHandlers.insert(std::make_pair(HEADER_VALUE_START, &HttpRequest::handleHeaderValueStart));
    stateHandlers.insert(std::make_pair(HEADER_VALUE, &HttpRequest::handleHeaderValue));
    stateHandlers.insert(std::make_pair(HEADER_LF, &HttpRequest::handleHeaderLF));
    stateHandlers.insert(std::make_pair(HEADERS_END_LF, &HttpRequest::handleHeadersEndLF));
// NORMAL BODY STATE HANDLERS
    stateHandlers.insert(std::make_pair(BODY_START, &HttpRequest::handleBodyStart));
    stateHandlers.insert(std::make_pair(BODY_CONTENT_LENGTH, &HttpRequest::handleBodyContentLength));
// CHUNKED BODY STATE HANDLERS
    stateHandlers.insert(std::make_pair(CHUNK_SIZE_START, &HttpRequest::handleChunkSizeStart));
    stateHandlers.insert(std::make_pair(CHUNK_SIZE, &HttpRequest::handleChunkSize));
//    stateHandlers.insert(std::make_pair(CHUNK_SIZE_CR, &HttpRequest::handleChunkSizeCR));
    stateHandlers.insert(std::make_pair(CHUNK_SIZE_LF, &HttpRequest::handleChunkSizeLF));
    stateHandlers.insert(std::make_pair(CHUNK_DATA, &HttpRequest::handleChunkData));
    stateHandlers.insert(std::make_pair(CHUNK_DATA_LF, &HttpRequest::handleChunkDataLF));
    stateHandlers.insert(std::make_pair(CHUNK_TRAILER_CR, &HttpRequest::handleChunkTrailerCR));
    stateHandlers.insert(std::make_pair(CHUNK_TRAILER_LF, &HttpRequest::handleChunkTrailerLF));
// MULTIPART/FORM-DATA BODY STATE HANDLERS
    stateHandlers.insert(std::make_pair(BODY_BOUNDARY_START, &HttpRequest::handleBodyBoundaryStart));
    stateHandlers.insert(std::make_pair(BODY_BOUNDARY_PARSING, &HttpRequest::handleBodyBoundaryParsing));
    stateHandlers.insert(std::make_pair(BODY_BOUNDARY_LF, &HttpRequest::handleBodyBoundaryLF));
    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_NAME, &HttpRequest::handleBodyPartHeaderName));
    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_VALUE, &HttpRequest::handleBodyPartHeaderValue));
    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_LF, &HttpRequest::handleBodyPartHeaderLF));
    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_LF2, &HttpRequest::handleBodyPartHeaderLF2));
    stateHandlers.insert(std::make_pair(BODY_PART_DATA, &HttpRequest::handleBodyPartData));
    stateHandlers.insert(std::make_pair(BODY_PART_END, &HttpRequest::handleBodyPartEnd));
// ERRORS STATE CODES
    errorState.insert(std::make_pair(ERROR_BAD_REQUEST, 400));
    errorState.insert(std::make_pair(ERROR_INVALID_METHOD, 501));
    errorState.insert(std::make_pair(ERROR_INTERNAL_ERROR, 500));
    errorState.insert(std::make_pair(ERROR_METHOD_NOT_ALLOWED, 405));
    errorState.insert(std::make_pair(ERROR_NOT_FOUND, 404));
    errorState.insert(std::make_pair(ERROR_INVALID_URI, 400));
    errorState.insert(std::make_pair(REQUEST_URI_TOO_LONG, 414));
    errorState.insert(std::make_pair(ERROR_INVALID_VERSION, 505));
    errorState.insert(std::make_pair(ERROR_INVALID_HEADER, 400));
    errorState.insert(std::make_pair(ERROR_CONTENT_LENGTH, 411));
    errorState.insert(std::make_pair(ERROR_CHUNK_SIZE, 400));
    errorState.insert(std::make_pair(ERROR_BOUNDARY, 400));
    errorState.insert(std::make_pair(ERROR_INCOMPLETE, 400));
    errorState.insert(std::make_pair(ERROR_BUFFER_OVERFLOW, 400));
    errorState.insert(std::make_pair(ERROR_BINARY_DATA, 415));
    errorState.insert(std::make_pair(ERROR_FILE_TOO_LARGE, 413));
};

std::vector<uint8_t>& HttpRequest::GetBody()
{
    return body;
}

std::map<std::string, std::string>& HttpRequest::getheaders()
{
    return headers;
}
void HttpRequest::parse(uint8_t *buffer, int readSize, size_t limitBodySize)
{
//    std::cout << "state in state machine parser  " << std::endl;
    //     for(int i = 0; i < readSize && !errorOccured(); i++)
    // {
    //     std::cout << buffer[i];
    //     }

    for(int i = 0; i < readSize && !errorOccured(); i++)
    {
        //        std::cout << buffer[i];

        (this->*currentHandler)(buffer[i]);
        std::map<State, StateHandler>::const_iterator it = stateHandlers.find(currentState);
        if (it != stateHandlers.end())
            currentHandler = it->second;
        else
            break;
    }
    if (body.size() > limitBodySize)
        currentState = ERROR_FILE_TOO_LARGE;
    if (errorOccured())
    {
        std::map<State, int>::const_iterator it = errorState.find(currentState);
        statusCode = it->second;
    }
        //  std::cout << "state in state machine parser  " << currentBodySize << " " << limitBodySize << std::endl;
        // std::cout << "chunked body read  " << chunkbytesread << std::endl;
        // std::cout << "chunk size  " << chunkSize << std::endl;
    //  std::cout <<" method is : " << method << std::endl;
    //   std::cout <<" uri is : " << uri << std::endl;
    //  std::cout << "version is " << version << std::endl;
    //  std::cout <<" status code is : " << statusCode << std::endl;
    // std::cout <<" isChunked are : " << isChunked << std::endl;
    // std::cout <<" isMultipart are : " << isMultipart << std::endl;
    //  std::cout <<" contentLength are : " << contentLength << std::endl;
    // std::cout <<" boundary are :                             " << boundary << std::endl;

    // for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
    //     std::cout << it->first << ": " << it->second << std::endl;
    // }
    // std::cout <<" body is : " << std::endl;
    // for (std::vector<uint8_t>::const_iterator it = body.begin(); it != body.end(); ++it) {
    //     std::cout << static_cast<char>(*it);
    // }
    // std::cout <<" form fields are : " << std::endl;
    // for(std::map<std::string, std::string>::const_iterator it = formFields.begin(); it != formFields.end(); it++){
    //     std::cout << it->first << ": " << it->second << std::endl;
    // }
    // std::cout << std::endl;
}

Route& HttpRequest::getCurrentRoute()
{
    return CurrentRoute;
}

void HttpRequest::setMethod(const std::string methodStr){
    method = methodStr;
};
std::string& HttpRequest::getMethod() {
    return method;
};
void HttpRequest::setUri(const std::string inputUrl){
    uri = inputUrl;
};
std::string& HttpRequest::getUri() {
    return uri;
};
void HttpRequest::reset(){
    resetMessage();
    method.clear();
    uri.clear();
    currentState = METHOD_START;
    statusCode = 200;
};

bool HttpRequest::parsingCompleted() const {
    return currentState == MESSAGE_COMPLETE;
}

int HttpRequest::GetStatusCode() const
{
    return statusCode;
}

std::string HttpRequest::getQuery() const
{
    return query;
}

State HttpRequest::getcurrentState() const{
    return currentState;
}

std::string& HttpRequest::getHeader(std::string key)
{
    return headers[key];
}
std::map<State, int>& HttpRequest::getErrorState(){
    return errorState;
}
void HttpRequest::SetStatusCode(int status){
    statusCode = status;
}
