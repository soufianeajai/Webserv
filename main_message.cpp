#include "HttpMessage.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

int main()
{
    HttpResponse r;
    r.setStatusCode(500);
    r.LoadErrorPage();
    std::cout << r.toString();
}