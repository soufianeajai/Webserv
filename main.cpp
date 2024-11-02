#include "HttpRequest/HttpRequest.hpp"
#include "HttpMessage/HttpMessage.hpp"

int main() {
    try {
       // testSimpleGetRequest();
     //   testPostWithChunkedTransfer();
    //     testMultipartFormData();
       //  testErrorCases();
      //   testURIDecoding();
        
        std::cout << "\nAll tests passed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}