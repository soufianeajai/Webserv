#include "HttpResponse.hpp"
#include "../Connection/Connection.hpp"
void HttpResponse::sendData(int clientSocketId, Status& status)
{
    ssize_t SentedBytes = 0;
    std::vector<uint8_t> response;
    int statusChild;
    try{
        if(!headerSended)
        {
            std::ostringstream oss;
            //std::cout <<"\nbuildResponseBuffer : (status : "<<status<<") "<<clientSocketId<<" "<< version << " " << statusCode << " " << reasonPhrase <<"\n";
            oss << version << " " << statusCode << " " << reasonPhrase << "\r\n";
            for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
                oss << it->first << ": " << it->second << "\r\n";
             oss << "\r\n";
            std::string responseStr = oss.str();
            response.insert(response.end(), responseStr.begin(), responseStr.end());
            // for(size_t i= 0;i < response.size();i++)
            //     std::cout << response[i];
            SentedBytes = send(clientSocketId, reinterpret_cast<char*>(response.data()), responseStr.size(), MSG_NOSIGNAL);
            if (SentedBytes < 0)
            { 

                std::cerr << "2 Send failed to client "<<clientSocketId << std::endl;
                status = DONE;
                return;
            }
            headerSended = true;
            response.clear();
        }
        if (cgi)
        {
            
            statusChild = parentProcess();
            //std::cout << "we are in cgi status: "<<statusChild<<"\n";
            if (statusChild == -1)
                return;
            else if (statusChild == 1)
            {
                UpdateStatueCode(500);
                sendData(clientSocketId,status);
            }
            else if (!statusChild)
                sendCgi(clientSocketId, status);
            return ;
        }
        size_t chunkSize = (totaSize < Connection::CHUNK_SIZE) ? totaSize : Connection::CHUNK_SIZE;
        std::ifstream file(Page.c_str(), std::ios::binary);
        if (!file.is_open() || totaSize == -1)
        {
            std::cerr << "no body for client :"<<clientSocketId << std::endl;
            status = DONE; 
            return;
        }
        file.seekg(offset, std::ios::beg); // go to position actual
        response.resize(chunkSize); // resize for chunck n 
        file.read(reinterpret_cast<char*>(response.data()), chunkSize);
        chunkSize = file.gcount(); // Get the number of bytes actually read
        offset += chunkSize;
        if (chunkSize > 0)
        {
            SentedBytes = send(clientSocketId, reinterpret_cast<char*>(response.data()), chunkSize, MSG_NOSIGNAL);
            if (SentedBytes < 0)
            { 
                std::cerr << "1 Send failed to client "<<clientSocketId << std::endl;
                file.close();
                status = DONE;  // handle error as needed
                return;
            }
            
        }
        if (chunkSize == 0 || offset >= static_cast<size_t>(totaSize))
        {
            file.close();            
            status = DONE;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in buildResponseBuffer: " << e.what() << std::endl;
        status = DONE;
        // Handle any other standard exceptions that may occur
    }
}
