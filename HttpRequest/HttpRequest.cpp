#include "HttpRequest.hpp"
//#include "Connection.hpp"

// const std::map<State, HttpRequest::StateHandler> HttpRequest::stateHandlers = {
//     {INITIALIZED, &HttpRequest::handleInitialized},
// //Sart line
//     {METHOD_START, &HttpRequest::handleMethodStart},
//     {METHOD_PARSING, &HttpRequest::handleMethodParsing},
//     {FIRST_SP, &HttpRequest::handleFirstSP},
//     {URI_START, &HttpRequest::handleURIStart},
//     {URI_PATH_PARSING, &HttpRequest::handleURIPathParsing},
//     {DECODE_URI, &HttpRequest::handleDecodeURI},
//     {URI_SKIP_QUERY_OR_FRAGMENT, &HttpRequest::handleSkipQF},
//     {SECOND_SP, &HttpRequest::handleSecondSP},
//     {VERSION_HTTP, &HttpRequest::handleVersionHTTP},
//     {REQUEST_LINE_CR, &HttpRequest::handleRequestLineCR},
//     {REQUEST_LINE_LF, &HttpRequest::handleRequestLineLF},
// // header
//     {HEADER_START, &HttpRequest::handleHeaderStart},
//     {HEADER_NAME, &HttpRequest::handleHeaderName},
//     {HEADER_COLON, &HttpRequest::handleHeaderColon},
//     {HEADER_SPACE_AFTER_COLON, &HttpRequest::handleHeaderValueStart},
//     {HEADER_VALUE, &HttpRequest::handleHeaderValue},
//     {HEADER_CR, &HttpRequest::handleHeaderCR},
//     {HEADER_LF, &HttpRequest::handleHeaderLF},
//     {HEADERS_END_CR, &HttpRequest::handleHeadersEndCR},
//     {HEADERS_END_LF, &HttpRequest::handleHeadersEndLF},
// // body
//     {BODY_START, &HttpRequest::handleBodyStart},
//     {BODY_CONTENT_LENGTH, &HttpRequest::handleBodyContentLength},
// //chunked body
//     {CHUNK_SIZE_START, &HttpRequest::handleChunkSizeStart},
//     {CHUNK_SIZE, &HttpRequest::handleChunkSize},
//     {CHUNK_SIZE_CR, &HttpRequest::handleChunkSizeCR},
//     {CHUNK_SIZE_LF, &HttpRequest::handleChunkSizeLF},
//     {CHUNK_DATA, &HttpRequest::handleChunkData},
//     {CHUNK_DATA_CR, &HttpRequest::handleChunkDataCR},
//     {CHUNK_DATA_LF, &HttpRequest::handleChunkDataLF},
//     {CHUNK_TRAILER_START, &HttpRequest::handleChunkTrailerStart},
//     {CHUNK_TRAILER_CR, &HttpRequest::handleChunkTrailerCR},
//     {CHUNK_TRAILER_LF, &HttpRequest::handleChunkTrailerLF},
// // multipart body
//     {BODY_BOUNDARY_START, &HttpRequest::handleBodyBoundaryStart},
//     {BODY_BOUNDARY_PARSING, &HttpRequest::handleBodyBoundaryParsing},
//     {BODY_BOUNDARY_CR, &HttpRequest::handleBodyBoundaryCR},
//     {BODY_BOUNDARY_LF, &HttpRequest::handleBodyBoundaryLF},
//     {BODY_PART_HEADER_START, &HttpRequest::handleBodyPartHeaderStart},
//     {BODY_PART_HEADER, &HttpRequest::handleBodyPartHeader},
//     {BODY_PART_HEADER_CR, &HttpRequest::handleBodyPartHeaderCR},
//     {BODY_PART_HEADER_LF, &HttpRequest::handleBodyPartHeaderLF},
//     {BODY_PART_DATA, &HttpRequest::handleBodyPartData},
//     {BODY_PART_END, &HttpRequest::handleBodyPartEnd}
// };
// const std::map<State, int> HttpRequest::errorState{
//     {ERROR_BAD_REQUEST, 400},
//     {ERROR_INVALID_METHOD, 501},
//     {ERROR_INVALID_URI, 400},
//     {ERROR_INVALID_VERSION, 505},
//     {ERROR_INVALID_HEADER, 400},
//     {ERROR_CONTENT_LENGTH, 411},
//     {ERROR_CHUNK_SIZE, 400},
//     {ERROR_BOUNDARY, 400},
//     {ERROR_INCOMPLETE, 400},
//     {ERROR_BUFFER_OVERFLOW, 400},
//     {ERROR_BINARY_DATA, 415},
//     {REQUEST_URI_TOO_LONG, 414}
// };

HttpRequest::HttpRequest():HttpMessage(), method(""), uri(""), currentState(METHOD_START), statusCode(200),
holder(""), currentHeaderName(""), currentHeaderValue(""), isChunked(false), isMultipart(false), contentLength(-1),
boundary(""), chunkSize(0), chunkbytesread(0), currentHandler(&HttpRequest::handleMethodStart), fieldName(""){
    // Initialize stateHandlers
    stateHandlers.insert(std::make_pair(METHOD_START, &HttpRequest::handleMethodStart));
    stateHandlers.insert(std::make_pair(METHOD_PARSING, &HttpRequest::handleMethodParsing));
    stateHandlers.insert(std::make_pair(FIRST_SP, &HttpRequest::handleFirstSP));
    stateHandlers.insert(std::make_pair(URI_START, &HttpRequest::handleURIStart));
    stateHandlers.insert(std::make_pair(URI_PATH_PARSING, &HttpRequest::handleURIPathParsing));
    stateHandlers.insert(std::make_pair(DECODE_URI, &HttpRequest::handleDecodeURI));
    stateHandlers.insert(std::make_pair(URI_SKIP_QUERY_OR_FRAGMENT, &HttpRequest::handleSkipQF));
//    stateHandlers.insert(std::make_pair(SECOND_SP, &HttpRequest::handleSecondSP));
    stateHandlers.insert(std::make_pair(VERSION_HTTP, &HttpRequest::handleVersionHTTP));
    stateHandlers.insert(std::make_pair(REQUEST_LINE_CR, &HttpRequest::handleRequestLineCR));
    stateHandlers.insert(std::make_pair(REQUEST_LINE_LF, &HttpRequest::handleRequestLineLF));
    stateHandlers.insert(std::make_pair(HEADER_START, &HttpRequest::handleHeaderStart));
    stateHandlers.insert(std::make_pair(HEADER_NAME, &HttpRequest::handleHeaderName));
//    stateHandlers.insert(std::make_pair(HEADER_COLON, &HttpRequest::handleHeaderColon));
    stateHandlers.insert(std::make_pair(HEADER_VALUE_START, &HttpRequest::handleHeaderValueStart));
    stateHandlers.insert(std::make_pair(HEADER_VALUE, &HttpRequest::handleHeaderValue));
//    stateHandlers.insert(std::make_pair(HEADER_CR, &HttpRequest::handleHeaderCR));
    stateHandlers.insert(std::make_pair(HEADER_LF, &HttpRequest::handleHeaderLF));
//    stateHandlers.insert(std::make_pair(HEADERS_END_CR, &HttpRequest::handleHeadersEndCR));
    stateHandlers.insert(std::make_pair(HEADERS_END_LF, &HttpRequest::handleHeadersEndLF));
    stateHandlers.insert(std::make_pair(BODY_START, &HttpRequest::handleBodyStart));
    stateHandlers.insert(std::make_pair(BODY_CONTENT_LENGTH, &HttpRequest::handleBodyContentLength));
    stateHandlers.insert(std::make_pair(CHUNK_SIZE_START, &HttpRequest::handleChunkSizeStart));
    stateHandlers.insert(std::make_pair(CHUNK_SIZE, &HttpRequest::handleChunkSize));
    stateHandlers.insert(std::make_pair(CHUNK_SIZE_CR, &HttpRequest::handleChunkSizeCR));
    stateHandlers.insert(std::make_pair(CHUNK_SIZE_LF, &HttpRequest::handleChunkSizeLF));
    stateHandlers.insert(std::make_pair(CHUNK_DATA, &HttpRequest::handleChunkData));
//    stateHandlers.insert(std::make_pair(CHUNK_DATA_CR, &HttpRequest::handleChunkDataCR));
    stateHandlers.insert(std::make_pair(CHUNK_DATA_LF, &HttpRequest::handleChunkDataLF));
//    stateHandlers.insert(std::make_pair(CHUNK_TRAILER_START, &HttpRequest::handleChunkTrailerStart));
    stateHandlers.insert(std::make_pair(CHUNK_TRAILER_CR, &HttpRequest::handleChunkTrailerCR));
    stateHandlers.insert(std::make_pair(CHUNK_TRAILER_LF, &HttpRequest::handleChunkTrailerLF));
    stateHandlers.insert(std::make_pair(BODY_BOUNDARY_START, &HttpRequest::handleBodyBoundaryStart));
    stateHandlers.insert(std::make_pair(BODY_BOUNDARY_PARSING, &HttpRequest::handleBodyBoundaryParsing));
//    stateHandlers.insert(std::make_pair(BODY_BOUNDARY_CR, &HttpRequest::handleBodyBoundaryCR));
    stateHandlers.insert(std::make_pair(BODY_BOUNDARY_LF, &HttpRequest::handleBodyBoundaryLF));
    // stateHandlers.insert(std::make_pair(BODY_PART_HEADERLF2, &HttpRequest::handleBodyBoundaryLF2));
    // stateHandlers.insert(std::make_pair(BODY_PART_HEADERCR2, &HttpRequest::handleBodyBoundaryCR2));
//    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_START, &HttpRequest::handleBodyPartHeaderStart));
    stateHandlers.insert(std::make_pair(BODY_PART_HEADER, &HttpRequest::handleBodyPartHeader));
//    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_CR, &HttpRequest::handleBodyPartHeaderCR));
    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_LF, &HttpRequest::handleBodyPartHeaderLF));
    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_CR2, &HttpRequest::handleBodyPartHeaderCR2));
    stateHandlers.insert(std::make_pair(BODY_PART_HEADER_LF2, &HttpRequest::handleBodyPartHeaderLF2));
    stateHandlers.insert(std::make_pair(BODY_PART_DATA, &HttpRequest::handleBodyPartData));
    stateHandlers.insert(std::make_pair(BODY_PART_END, &HttpRequest::handleBodyPartEnd));

    // Initialize errorState
    errorState.insert(std::make_pair(ERROR_BAD_REQUEST, 400));
    errorState.insert(std::make_pair(ERROR_INVALID_METHOD, 501));
    errorState.insert(std::make_pair(ERROR_INVALID_URI, 400));
    errorState.insert(std::make_pair(ERROR_INVALID_VERSION, 505));
    errorState.insert(std::make_pair(ERROR_INVALID_HEADER, 400));
    errorState.insert(std::make_pair(ERROR_CONTENT_LENGTH, 411));
    errorState.insert(std::make_pair(ERROR_CHUNK_SIZE, 400));
    errorState.insert(std::make_pair(ERROR_BOUNDARY, 400));
    errorState.insert(std::make_pair(ERROR_INCOMPLETE, 400));
    errorState.insert(std::make_pair(ERROR_BUFFER_OVERFLOW, 400));
    errorState.insert(std::make_pair(ERROR_BINARY_DATA, 415));
    errorState.insert(std::make_pair(REQUEST_URI_TOO_LONG, 414));
};

void HttpRequest::handleMethodStart(uint8_t byte) {
    if (byte == 'G' || byte == 'P' || byte == 'D') {
        method = byte;
        currentState = METHOD_PARSING;
    }
    else
        currentState =  ERROR_INVALID_METHOD;
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
                currentState = FIRST_SP;
            return;
        }
    }    
    currentState = ERROR_INVALID_METHOD;
}

void HttpRequest::handleFirstSP(uint8_t byte) {
    if (byte == ' ')
        currentState = URI_START;
    else
        currentState = ERROR_BAD_REQUEST;
}
void HttpRequest::handleURIStart(uint8_t byte) {
    if (byte == '/') {
        currentState = URI_PATH_PARSING;
        uri += byte;
    }
    else
        currentState = ERROR_INVALID_URI;
}
void HttpRequest::handleURIPathParsing(uint8_t byte) {
// Check for end of URI (space before HTTP version)
    if (byte == ' ')
        currentState = VERSION_HTTP;
// Check max length or invalid char
    else if (uri.length() >= HttpRequest::MAX_URI_LENGTH ||!isValidPathChar(byte) )
        currentState = ERROR_INVALID_URI;
// check for consecutive // in the uri if so ... do nothing 
    else if (byte == '/' && !uri.empty() && uri.back() == '/')
        currentState = URI_PATH_PARSING;
    else if (byte == '%')
    {
        holder.clear();
        currentState = DECODE_URI;
    }
    else if (byte == '?' || byte == '#') {
        currentState = URI_SKIP_QUERY_OR_FRAGMENT;
    }
    else
        uri += byte;
}

void HttpRequest::handleSkipQF(uint8_t byte) {
    if (byte == ' ')
        currentState = VERSION_HTTP; 
}
void HttpRequest::handleDecodeURI(uint8_t byte) {
    if (holder.length() < 2 && std::isxdigit(byte)) {
        holder += byte;
        if (holder.length() == 2)
        {
            int value = std::stoi(holder, NULL, 16); 
            uri += static_cast<char>(value);
            holder.clear();
            currentState = URI_PATH_PARSING;
        }
    }
    else if (!std::isxdigit(byte))
        currentState = ERROR_INVALID_URI;
}

void HttpRequest::handleVersionHTTP(uint8_t byte) {
    std::string expectedHttpVersion = "HTTP/1.1";
    size_t versiondNextByte = version.length();
    if (uriBehindRoot())
        currentState = ERROR_BAD_REQUEST;
    else if (versiondNextByte < expectedHttpVersion.length()) {
        if (byte == expectedHttpVersion[versiondNextByte]) {
            version += byte; 
            if (version == expectedHttpVersion)
                currentState = REQUEST_LINE_CR;
            return;
        }
    }    
    currentState = ERROR_INVALID_VERSION;
}

void HttpRequest::handleRequestLineCR(uint8_t byte) {
    if (byte == '\r')
        currentState = REQUEST_LINE_LF;
    else
        currentState = ERROR_BAD_REQUEST;
}
void HttpRequest::handleRequestLineLF(uint8_t byte) {
    if (byte == '\n')
        currentState = HEADER_START;
    else
        currentState = ERROR_BAD_REQUEST;
}

void HttpRequest::handleHeaderStart(uint8_t byte) {
    if (byte == '\r')
       currentState = HEADERS_END_LF;
    else if (!isValidHeaderNameChar(byte))
       currentState = ERROR_INVALID_HEADER;
    else {
        currentHeaderName += byte;
       currentState = HEADER_NAME;
    }
}

void HttpRequest::handleHeaderName(uint8_t byte) {
    if (byte == ':')
        currentState = HEADER_VALUE_START;
    else if (!isValidHeaderNameChar(byte))
        currentState = ERROR_BAD_REQUEST;
    else
        currentHeaderName += byte;
}


void HttpRequest::handleHeaderValueStart(uint8_t byte) {
    if (byte == '\r')
        currentState = HEADER_LF;
    else {
        currentHeaderValue += byte;
        currentState = HEADER_VALUE;
    }    
}

void HttpRequest::handleHeaderValue(uint8_t byte) {
    if (byte == '\r')
        currentState = HEADER_LF;
    else
        currentHeaderValue += byte;
}

void HttpRequest::handleHeaderLF(uint8_t byte) {
    if (byte == '\n') {
        addCurrentHeader();
        currentState = HEADER_START;
    }
    else {
        currentState = ERROR_BAD_REQUEST;
    }    
}
void HttpRequest::handleHeadersEndLF(uint8_t byte) {
    // if (byte == '\n' && (headers.find("Content-Length") == headers.end() && headers.find("Transfer-Encoding") == headers.end()))
    //     currentState = MESSAGE_COMPLETE;
    if (byte == '\n')
        currentState = BODY_START;
    else
        currentState = ERROR_BAD_REQUEST;
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
        handleChunkSizeStart(byte);
    else if (isMultipart)
        handleBodyBoundaryStart(byte);
    else if (contentLength > 0)
        handleBodyContentLength(byte);
    else
        currentState = MESSAGE_COMPLETE;
}

void HttpRequest::handleChunkSizeStart(uint8_t byte) {
    chunkbytesread = 0;
    chunkSize = 0;
    holder.clear();
    if (std::isxdigit(byte)) {
        holder += byte;
        currentState = CHUNK_SIZE;
    }
    else
        currentState = ERROR_CHUNK_SIZE;
}

void HttpRequest::handleChunkSize(uint8_t byte) {
    if (std::isxdigit(byte)) {
        holder += byte;
        currentState = CHUNK_SIZE_CR;
    }
    else if (byte == '\r')
        currentState = CHUNK_SIZE_LF;
    else
        currentState = ERROR_CHUNK_SIZE;
}
void HttpRequest::handleChunkSizeCR(uint8_t byte) {
    if (byte == '\r')
        currentState = CHUNK_SIZE_LF;
    else
        currentState = ERROR_CHUNK_SIZE;
}

void HttpRequest::handleChunkSizeLF(uint8_t byte) {
    if (byte == '\n')
    {
        chunkSize = std::stoi(holder, NULL, 16);
        if (chunkSize == 0)
            currentState = CHUNK_TRAILER_CR;
        else
            currentState = CHUNK_DATA;

    }
    else
        currentState = ERROR_CHUNK_SIZE;
}
void HttpRequest::handleChunkTrailerCR(uint8_t byte) {
    if (byte == '\r')
        currentState = CHUNK_TRAILER_LF;
    else
        currentState = ERROR_CHUNK_SIZE;
}
void HttpRequest::handleChunkTrailerLF(uint8_t byte) {
    if (byte == '\n')
        currentState = MESSAGE_COMPLETE ;
    else
        currentState = ERROR_CHUNK_SIZE;
}

void HttpRequest::handleChunkData(uint8_t byte) {
    if (byte == '\r')
        currentState = CHUNK_DATA_LF;
    else if (chunkbytesread > chunkSize)
        currentState = ERROR_CHUNK_SIZE;
    else
    {
        body.push_back(byte);
        chunkbytesread++;
    }
}

void HttpRequest::handleChunkDataLF(uint8_t byte) {
    if (byte == '\n')
        currentState = CHUNK_SIZE_START;
    else
        currentState = ERROR_CHUNK_SIZE;  
}



void    HttpRequest::handleBodyContentLength(uint8_t byte) {
    if (contentLength == -1 || currentState == MESSAGE_COMPLETE)
        currentState = ERROR_CONTENT_LENGTH;
    else if (contentLength == 0)
        currentState = MESSAGE_COMPLETE;
    else
    {
        body.push_back(byte);
        contentLength--;
    }

}

void HttpRequest::handleBodyBoundaryStart(uint8_t byte) {
    if (byte == '-') {
        holder += byte;
        currentState = BODY_BOUNDARY_PARSING;
    }
    else
        currentState = ERROR_BOUNDARY;
}

void HttpRequest::handleBodyBoundaryParsing(uint8_t byte) {
    if (byte == '\r') {
        std::string expectedBoundary = "--" + boundary;
        if (holder == expectedBoundary) {
            currentState = BODY_BOUNDARY_LF;
        }
        else if (holder == expectedBoundary + "--")
            currentState = MESSAGE_COMPLETE;
        else
            currentState = ERROR_BOUNDARY;
    }
    else {
        holder += byte;
        if (holder.length() > boundary.length() + 4)
            currentState = ERROR_BOUNDARY;
    }   
}

void HttpRequest::handleBodyBoundaryLF(uint8_t byte) {
    if (byte == '\n') {
        currentState = BODY_PART_HEADER;
        holder.clear();
    }
    else
        currentState = ERROR_BOUNDARY;
}
// void HttpRequest::handleBodyBoundaryCR2(uint8_t byte) {
//     if (byte == '\r') {
//         currentState = BODY_PART_HEADERLF2;
//     }
//     else
//         currentState = ERROR_BOUNDARY;
// }
// void HttpRequest::handleBodyBoundaryLF2(uint8_t byte) {
//     if (byte == '\n') {
//         currentState = BODY_PART_HEADER;
//     }
//     else
//         currentState = ERROR_BOUNDARY;
// }

void HttpRequest::handleBodyPartHeader(uint8_t byte) {
    if (byte == '\r') {
        currentState = BODY_PART_HEADER_LF;
    }
    else
        holder += byte;
}

void HttpRequest::handleBodyPartHeaderLF(uint8_t byte) {
    std::size_t namePos = holder.find("name=");
    if (namePos != std::string::npos) {
        namePos += 6;
        std::size_t endPos = holder.find("\"", namePos);
        if (endPos != std::string::npos){
            fieldName = holder.substr(namePos, endPos - namePos);
            std::cout << "field " << fieldName << std::endl;
            formFields.insert(std::make_pair(fieldName, ""));
        }
        else {
            currentState = ERROR_BOUNDARY;
            return;
        }
    }
    if (byte == '\n')
        currentState = BODY_PART_HEADER_CR2;
    else
        currentState = ERROR_BOUNDARY;
}
void HttpRequest::handleBodyPartHeaderCR2(uint8_t byte) {
    if (byte == '\r')
        currentState = BODY_PART_HEADER_LF2;
    else
        currentState = ERROR_BOUNDARY;
}

void HttpRequest::handleBodyPartHeaderLF2(uint8_t byte) {
    if (byte == '\n')
        currentState = BODY_PART_DATA;
    else
        currentState = ERROR_BOUNDARY;
}
void HttpRequest::handleBodyPartData(uint8_t byte) {
//        std::cout << "fieldname = " << fieldName << " " << "value " << formFields[fieldName] << std::cout;
    if (byte == '\r')
        currentState = BODY_PART_END;
    else if (!holder.empty())
    {
        formFields[fieldName] += byte;
    }
    else
        currentState = ERROR_BOUNDARY;
}

void HttpRequest::handleBodyPartEnd(uint8_t byte) {
    if (byte == '\n') {
        holder.clear();
        fieldName.clear();
        currentState = BODY_BOUNDARY_START;
    }
    else
        currentState = ERROR_BOUNDARY;
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
        std::cout << currentState << std::endl;
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
    // std::cout <<" method is : " << method << std::endl;
    // std::cout <<" uri is : " << uri << std::endl;
    // std::cout << "version is " << version << std::endl;
     std::cout <<" status code is : " << statusCode << std::endl;
    // std::cout <<" isChunked are : " << isChunked << std::endl;
    // std::cout <<" isMultipart are : " << isMultipart << std::endl;
    // std::cout <<" contentLength are : " << contentLength << std::endl;
    // std::cout <<" boundary are :                             " << boundary << std::endl;

    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout <<" body is : " << std::endl;
    for (std::vector<uint8_t>::const_iterator it = body.begin(); it != body.end(); ++it) {
        std::cout << static_cast<char>(*it);
    }
    std::cout <<" form fields are : " << std::endl;
    for(std::map<std::string, std::string>::const_iterator it = formFields.begin(); it != formFields.end(); it++){
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << std::endl;
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
    currentState = METHOD_START;
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