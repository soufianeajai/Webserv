#include "HttpRequest.hpp"

HttpRequest::HttpRequest():HttpMessage(), currentState(METHOD_START), method(""), uri(""),  statusCode(200),
holder(""), currentHeaderName(""), currentHeaderValue(""), isChunked(false), isMultipart(false), contentLength(0),
boundary(""), chunkSize(0), chunkbytesread(0), currentHandler(&HttpRequest::handleMethodStart), query(""){
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
    errorState.insert(std::make_pair(ERROR_INVALID_VERSION, 505));
    errorState.insert(std::make_pair(ERROR_INVALID_HEADER, 400));
    errorState.insert(std::make_pair(ERROR_CONTENT_LENGTH, 411));
    errorState.insert(std::make_pair(ERROR_CHUNK_SIZE, 400));
    errorState.insert(std::make_pair(ERROR_BOUNDARY, 400));
    errorState.insert(std::make_pair(ERROR_FILE_TOO_LARGE, 413));
    errorState.insert(std::make_pair(ERROR_FORBIDDEN, 403));
};

std::vector<uint8_t>& HttpRequest::GetBody(){return body;}

std::map<std::string, std::string>& HttpRequest::getheaders(){return headers;}

Route& HttpRequest::getCurrentRoute(){return CurrentRoute;}

void HttpRequest::setMethod(const std::string methodStr){method = methodStr;};

std::string& HttpRequest::getMethod() {return method;};

void HttpRequest::setUri(const std::string inputUrl){uri = inputUrl;}

std::string& HttpRequest::getUri() {return uri;}

bool HttpRequest::parsingCompleted() const {return currentState == MESSAGE_COMPLETE;}

int HttpRequest::GetStatusCode() const{return statusCode;}

std::string HttpRequest::getQuery() const{return query;}

State HttpRequest::getcurrentState() const{return currentState;}

std::string& HttpRequest::getHeader(std::string key){return headers[key];}

std::map<State, int>& HttpRequest::getErrorState(){return errorState;}

void HttpRequest::SetStatusCode(int status){statusCode = status;}

void HttpRequest::parse(uint8_t *buffer, int readSize, size_t limitBodySize)
{
    for(int i = 0; i < readSize && !errorOccured(); i++)
    {
        (this->*currentHandler)(buffer[i]);
        std::map<State, StateHandler>::const_iterator it = stateHandlers.find(currentState);
        if (it != stateHandlers.end())
            currentHandler = it->second;
        else
            break;
    }
    if (body.size() > limitBodySize)
        currentState = ERROR_FILE_TOO_LARGE;
                std::cout << "--------> "<< contentLength << std::endl;

}
