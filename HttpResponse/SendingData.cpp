#include "HttpResponse.hpp"
#include "../Connection/Connection.hpp"

void HttpResponse::SendHeaders(int clientSocketId, Status& status, std::vector<uint8_t>& heads)
{
    ssize_t SentedBytes = 0;
    if(!headerSended)
    {
        std::ostringstream oss;
        oss << version << " " << statusCode << " " << reasonPhrase << "\r\n";
        for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
            oss << it->first << ": " << it->second << "\r\n";
        oss << "\r\n";
        std::string headsStr = oss.str();
        heads.insert(heads.end(), headsStr.begin(), headsStr.end());
        SentedBytes = send(clientSocketId, reinterpret_cast<char*>(heads.data()), headsStr.size(), MSG_NOSIGNAL);
        if (SentedBytes < 0)
        {
            std::cerr << "[Error] ... 2 Send failed to client "<<clientSocketId << std::endl;
            status = DONE;
            return;
        }
        headerSended = true;

        heads.clear();
    }
}

void HttpResponse::sendData(int clientSocketId, Status& status)
{
    std::vector<uint8_t> heads;
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
                UpdateStatueCode(504);
        }
        SendHeaders(clientSocketId, status, heads);
        status = sendFileChunk(clientSocketId);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Error] ... Exception in sending data: " << e.what() << std::endl;
        status = DONE;
    }
}


Status HttpResponse::sendFileChunk(int clientSocketId)
{
    ssize_t SentedBytes = 0;
    size_t chunkSize = (totalSize < Connection::CHUNK_SIZE) ? totalSize : Connection::CHUNK_SIZE;
    std::ifstream file(Page.c_str(), std::ios::binary);
    if (!file.is_open() || totalSize == 0)
        return DONE;
    file.seekg(offset, std::ios::beg);
    body.resize(chunkSize);
    file.read(reinterpret_cast<char*>(body.data()), chunkSize);
    chunkSize = file.gcount();
    offset += chunkSize;
    if (chunkSize > 0)
    {
        SentedBytes = send(clientSocketId, reinterpret_cast<char*>(body.data()), chunkSize, MSG_NOSIGNAL);
        if (SentedBytes < 0)
        { 
            std::cerr << "[Error] ... 1 Send failed to client "<<clientSocketId << std::endl;
            file.close();
            return DONE;
        }
    }
    if (chunkSize == 0 || offset >= static_cast<size_t>(totalSize))
    {
        file.close();            
        return DONE;
    }
    return SENDING_RESPONSE;
}