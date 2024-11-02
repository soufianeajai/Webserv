#include "HttpMessage.hpp"

HttpMessage::HttpMessage():version(""){};

void  HttpMessage::resetMessage(){
    version.clear();
    body.clear();
    headers.clear();
}