#include "HttpResponse.hpp"
#include "../Connection/Connection.hpp"

void HttpResponse::SendHeaders(int clientSocketId, Status& status, std::vector<uint8_t>& heads)
{
    ssize_t SentedBytes = 0;
    if(!headerSended)
    {
        //headers["Content-Type"] ="text/html";
        std::ostringstream oss;
        //std::cout <<"\nbuildResponseBuffer : (status : "<<status<<") "<<clientSocketId<<" "<< version << " " << statusCode << " " << reasonPhrase <<"\n";
        oss << version << " " << statusCode << " " << reasonPhrase << "\r\n";
        for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
            oss << it->first << ": " << it->second << "\r\n";
        oss << "\r\n";
        std::string headsStr = oss.str();
        heads.insert(heads.end(), headsStr.begin(), headsStr.end());
        std::cout << "headers added\n";
        for(size_t i= 0;i < heads.size();i++)
            std::cout << heads[i];
        SentedBytes = send(clientSocketId, reinterpret_cast<char*>(heads.data()), headsStr.size(), MSG_NOSIGNAL);
        if (SentedBytes < 0)
        { 

            std::cerr << "2 Send failed to client "<<clientSocketId << std::endl;
            status = DONE;
            return;
        }
        headerSended = true;
        heads.clear();
    }
}

void HttpResponse::sendData(int clientSocketId, Status& status)
{
    ssize_t SentedBytes = 0;
    std::vector<uint8_t> heads;
    std::vector<uint8_t> body;
    int statusParent;
    try{
        
        if (cgi)
        {
            statusParent = parentProcess();
            if (ChildFInish)
            {
                SendHeaders(clientSocketId, status, heads);
                sendCgi(clientSocketId, status);
                return ;
            }
            if (statusParent == -1)
                return;
            else if (statusParent == 1)
            {
                std::cout << "timeeeeeeeeeeeeeeeee out \n";
                UpdateStatueCode(504);
            }
        }
        SendHeaders(clientSocketId, status, heads);
        size_t chunkSize = (totaSize < Connection::CHUNK_SIZE) ? totaSize : Connection::CHUNK_SIZE;
        std::ifstream file(Page.c_str(), std::ios::binary);
        if (!file.is_open() || totaSize == -1)
        {
            std::cerr << "no body for client :"<<clientSocketId << std::endl;
            status = DONE; 
            return;
        }
        file.seekg(offset, std::ios::beg); // go to position actual
        body.resize(chunkSize); // resize for chunck n 
        file.read(reinterpret_cast<char*>(body.data()), chunkSize);
        chunkSize = file.gcount(); // Get the number of bytes actually read
        offset += chunkSize;
        if (chunkSize > 0)
        {
            // for(size_t i= 0;i < body.size();i++)
            //     std::cout << body[i];
            SentedBytes = send(clientSocketId, reinterpret_cast<char*>(body.data()), chunkSize, MSG_NOSIGNAL);
            ///std::cout << "___________________________________________ body added : "<<SentedBytes<<" rsponos: "<<body.size()<<"\n";
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
            std::cout << "finish sendned all chuncking data !!\n";
            file.close();            
            status = DONE;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in sending data: " << e.what() << std::endl;
        status = DONE;
        // Handle any other standard exceptions that may occur
    }
}
