#include "HttpRequest.hpp"

void HttpRequest::handleMethodStart(uint8_t byte) {
    if (byte == 'G' || byte == 'P' || byte == 'D') {
        method = byte;
        currentState = METHOD_PARSING;
        if (byte == 'G')
            holder = "GET";
        else if (byte == 'P'){
            holder = "POST";
            statusCode = 201;
        }
        else{
            holder = "DELETE";
            statusCode = 204;
        }
    }
    else
        currentState =  ERROR_INVALID_METHOD;
}

void HttpRequest::handleMethodParsing(uint8_t byte) {
    size_t methodNextByte = method.length();
    if (methodNextByte < holder.length()) {
        if (byte == holder[methodNextByte]) {
            method += byte; 
            if (method == holder)
                currentState = FIRST_SP;
            return;
        }
    }    
    currentState = ERROR_INVALID_METHOD;
}

void HttpRequest::handleFirstSP(uint8_t byte) {
    holder.clear();
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
    if (byte == ' ')
        currentState = VERSION_HTTP;
    else if (!isValidPathChar(byte) )
        currentState = ERROR_INVALID_URI;
    else if (byte == '/' && !uri.empty() && uri[uri.length() - 1] == '/')
        currentState = URI_PATH_PARSING;
    else if (byte == '%')
        currentState = DECODE_URI;
    else if (byte == '?')
        currentState = URI_HANDLE_QUERY;
    else
        uri += byte;
}

void HttpRequest::handleQuery(uint8_t byte) {
    if (byte == ' ')
        currentState = VERSION_HTTP; 
    else
        query += byte;
}
void HttpRequest::handleDecodeURI(uint8_t byte) {
    if (holder.length() < 2 && std::isxdigit(byte)) {
        holder += byte;
        if (holder.length() == 2)
        {
            uri += static_cast<char>(std::strtol(holder.c_str(), NULL, 16));
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
    if (uriBehindRoot()){
        currentState = ERROR_FORBIDDEN;
    }
    else if (versiondNextByte < expectedHttpVersion.length()) {
        if (byte == expectedHttpVersion[versiondNextByte]) {
            version += byte; 
            if (version == expectedHttpVersion)
                currentState = REQUEST_LINE_CR;
            return;
        }
    }
    else    
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
// HEADERS STATE HANDLERS

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
    handleTransfer();
    if (byte == '\n'){
        if (method != "POST")
            currentState = MESSAGE_COMPLETE;
        else
            currentState = BODY_START;
    }
    else
        currentState = ERROR_BAD_REQUEST;
}
// CHECK TYPE OF BODY TRANSFER
void HttpRequest::handleBodyStart(uint8_t byte) {
    if (!isContentLength)
        currentState = ERROR_CONTENT_LENGTH;
    else if (isContentLength && !contentLength)
        currentState = ERROR_BAD_REQUEST;
    else if (isChunked)
        handleChunkSizeStart(byte);
    else if (isMultipart)
        handleBodyBoundaryStart(byte);
    else
        handleBodyContentLength(byte);

}

// BODY CHUNKED TRANSFER HANDLERS
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
    if (std::isxdigit(byte))
        holder += byte;
    else if (byte == '\r')
        currentState = CHUNK_SIZE_LF;
    else
        currentState = ERROR_CHUNK_SIZE;
}

void HttpRequest::handleChunkSizeLF(uint8_t byte) {
    if (byte == '\n')
    {
        chunkSize = std::strtol(holder.c_str(), NULL, 16);
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
    if (chunkbytesread > chunkSize)
        currentState = ERROR_CHUNK_SIZE;
    else if (chunkbytesread < chunkSize)
    {
        body.push_back(byte);
        chunkbytesread++;
    }
    else
        currentState = CHUNK_DATA_LF;
}

void HttpRequest::handleChunkDataLF(uint8_t byte) {
    if (byte == '\n')
        currentState = CHUNK_SIZE_START;
    else
        currentState = ERROR_CHUNK_SIZE;  
}


// NORMAL BODY STATE HANDLERS
void    HttpRequest::handleBodyContentLength(uint8_t byte) {
    body.push_back(byte);
    if ((body.size() == contentLength ) && contentLength){
        currentState = MESSAGE_COMPLETE;
    }
}
// MULTIPART/FORM-DATA BODY STATE HANDLERS

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
            parts.push_back(boundaryPart());
            currentState = BODY_BOUNDARY_LF;
        }
        else if (holder == expectedBoundary + "--")
            currentState = MESSAGE_COMPLETE;
        else
            currentState = ERROR_BOUNDARY;
        holder.clear();
    }
    else {
        holder += byte;
        if (holder.length() > boundary.length() + 4){
            currentState = ERROR_BOUNDARY;
            holder.clear();
        }
    }   
}

void HttpRequest::handleBodyBoundaryLF(uint8_t byte) {
    if (byte == '\n') {
        currentState = BODY_PART_HEADER_NAME;
        holder.clear();
    }
    else
        currentState = ERROR_BOUNDARY;
}

void HttpRequest::handleBodyPartHeaderName(uint8_t byte) {
    if (byte == ':') {
        currentHeaderName = holder;
        holder.clear();
        currentState = BODY_PART_HEADER_VALUE;
    }
    else if (byte == '\r') {
        if (holder.empty())
            currentState = BODY_PART_HEADER_LF2;
        else
            currentState = ERROR_BOUNDARY;
    }
    else
        holder += byte;
}

void HttpRequest::handleBodyPartHeaderValue(uint8_t byte) {
    if (byte == '\r') {
        parts.back().boundaryHeader[currentHeaderName] = holder;        
        if (currentHeaderName == "Content-Disposition") {
            std::size_t namePos = holder.find("name=");
            std::size_t filenamePos = holder.find("filename=");
            if (namePos != std::string::npos) {
                namePos += 6;
                std::size_t endPos = holder.find("\"", namePos);
                if (endPos != std::string::npos)
                    parts.back().name = holder.substr(namePos, endPos - namePos);
            }
            if (filenamePos != std::string::npos) {
                filenamePos += 10;
                std::size_t endPos = holder.find("\"", filenamePos);
                if (endPos != std::string::npos) {
                    parts.back().fileName = holder.substr(filenamePos, endPos - filenamePos);
                    parts.back().isFile = true;
                }
                parts.back().isFile = false;
            }
        }
        holder.clear();
        currentState = BODY_PART_HEADER_LF;
    }
    else
        holder += byte;
}

void HttpRequest::handleBodyPartHeaderLF(uint8_t byte) {
    if (byte == '\n')
        currentState = BODY_PART_HEADER_NAME;
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
    if (byte == '\r')
        currentState = BODY_PART_END;
    else {
        if (parts.back().isFile)
            parts.back().fileBody.push_back(byte);
        else
            parts.back().value += byte;
    }
}

void HttpRequest::handleBodyPartEnd(uint8_t byte) {
    if (byte == '\n') {
        holder.clear();
        currentHeaderName.clear();
        currentState = BODY_BOUNDARY_START;
    }
    else
        currentState = ERROR_BOUNDARY;
}

