#include <mutex>
#include <fstream>
#include <iostream>
#include <sstream>
#include <zlib.h>
#include <nlohmann/json.hpp>
#include "HttpConnection.h"
#include "../Utility/Utility.h"


HttpConnection::HttpConnection()
    : epollFd(0), fd(0), isReadEvent(true), sendBuf{}, recvBuf{}, recvIndex{}, sendIndex{}, webRoot("web/")
{
    //Bufferul de raspuns va fi completat din fisiere HTML in functie de cerere
}

HttpConnection::~HttpConnection() = default;

void HttpConnection::closeConnection() const
{
    close(fd);
}


//MODIFICARE FUNCTIE PENTRU A SUPORTA GESTIONAREA TIPURILOR DE CERERI
std::string HttpConnection::getRequestedPage() {
    std::string request(recvBuf);
    std::string page = "index_test.html";  

    size_t methodEnd = request.find(' ');
    if (methodEnd == std::string::npos) {
        return page;  
    }

    size_t urlStart = methodEnd + 1;
    size_t urlEnd = request.find(' ', urlStart);
    if (urlEnd == std::string::npos) {
        return page;  
    }

    std::string url = request.substr(urlStart, urlEnd - urlStart);

    size_t queryStart = url.find('?');
    if (queryStart != std::string::npos) {
        std::string queryString = url.substr(queryStart + 1);
        url = url.substr(0, queryStart); 
        std::cout << "Query string: " << queryString << std::endl;
       
    }

    if (!url.empty() && url[0] == '/') {
        url = url.substr(1);
    }

    if (url.empty()) {
        return page;
    }

    if (url.find("..") != std::string::npos) {
        std::cerr << "Tentativă de acces neautorizat detectată!" << std::endl;
        return "index_test.html";
    }

    return url;
}

//!!!!Modificata pentru a gestiona si compresie!!!
//MODIFICARE FUNCTIE PENTRU A SUPORTA GESTIONAREA TIPURILOR DE CERERI
void HttpConnection::handleRead()
{
    ssize_t ret;
    if (ssl) //verificare pt htts
    {
        ret = SSL_read(ssl, recvBuf + recvIndex, Config::sendRecvBufSize - recvIndex);
    } 
    else 
    {
        ret = recv(fd, recvBuf + recvIndex, Config::sendRecvBufSize - recvIndex, 0);
    }
    if (ret <= 0) {
        return; 
    }
    recvIndex += ret;

    HttpRequest request;
    if (!request.parse(recvBuf, recvIndex)) {
        
        this->responseHeader = ERROR_HEADER;
        this->responseBody = ERROR_BODY;
        return;
    }
    

        if (request.method == "GET") {
            handleGet(request);
        } else if (request.method == "POST") {
            handlePost(request);
        } else if (request.method == "PUT") {
            handlePut(request);
        } else if (request.method == "DELETE") {
            handleDelete(request);
        } else if (request.method == "HEAD") {
            handleHead(request);
        } else {
           
            this->responseHeader = ERROR_HEADER;
            this->responseBody = ERROR_BODY;
        }

        recvIndex = 0;
        memset(recvBuf, 0, sizeof(recvBuf));

        epoll_event event{};
        event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
    
}

void HttpConnection::handleRequest() //demos functionare thread
{
    auto start = std::chrono::high_resolution_clock::now(); // Începutul cronometrării

    if (isReadEvent)
    {
        std::cout << "[FD " << getFd() << "] Handling READ event...\n";
        handleRead();
        std::cout << "[FD " << getFd() << "] READ event completed.\n";
    }
    else
    {
        std::cout << "[FD " << getFd() << "] Handling WRITE event...\n";
        handleWrite();
        std::cout << "[FD " << getFd() << "] WRITE event completed.\n";
    }

    auto end = std::chrono::high_resolution_clock::now(); // Sfârșitul cronometrării
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "[FD " << getFd() << "] Event processed in " << elapsed.count() << " seconds.\n";
}


void HttpConnection::finalizeConnection() {
    recvIndex = 0;
    sendIndex = 0; 
    responseHeader.clear();
    responseBody.clear();
    if (isKeepAlive) {
        epoll_event event{};
        event.events = EPOLLIN | EPOLLONESHOT | EPOLLRDHUP;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
    } else {
        std::unique_lock<std::mutex> locker(Utility::mutex);
        close(fd);
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = nullptr;
        }
        locker.unlock();
    }
}


void HttpConnection::handleWrite() {
    ssize_t ret = 0;
    std::cout << "Răspuns Header trimis la socketFd: " << fd << std::endl;
    std::cout << responseHeader;

    std::cout << "Răspuns Body trimis la socketFd: " << fd << std::endl;
    std::cout << responseBody;

    
    while (headerIndex < responseHeader.size())
    {
        if (ssl) {
            ret = SSL_write(ssl, responseHeader.data() + headerIndex, responseHeader.size() - headerIndex);
        } else {
            ret = send(fd, responseHeader.data() + headerIndex, responseHeader.size() - headerIndex, 0);
        }
        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; 
            }
        }
        else if (ret > 0)
        {
            headerIndex += ret;
        }
    }

    
    if (!responseBody.empty())
    {
   
        while (bodyIndex < responseBody.size())
        {
            if (ssl) {
                ret = SSL_write(ssl, responseBody.data() + bodyIndex, responseBody.size() - bodyIndex);
            } else {
                ret = send(fd, responseBody.data() + bodyIndex, responseBody.size() - bodyIndex, 0);
            }
            if (ret == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break; 
                }
            }
            else if (ret > 0)
            {
               bodyIndex += ret;
            }
        }
    }
    
    if (headerIndex >= responseHeader.size() && bodyIndex >= responseBody.size()) 
    {
        headerIndex = 0;
        bodyIndex = 0;
        responseBody.clear(); 
        responseHeader.clear();

        if (isKeepAlive)
        {
            epoll_event event{};
            event.events = EPOLLIN | EPOLLONESHOT | EPOLLRDHUP;
            event.data.fd = fd;
            epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
        }
        else
        {
            std::unique_lock<std::mutex> locker(Utility::mutex);
            close(fd);
            if (ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
                ssl = nullptr;
            }
            locker.unlock();
        }
    }
    else 
    {
        epoll_event event{};
        event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
    }
}



void HttpConnection::init(const int &epollFd, const int &fd, SSL *ssl) //!!!
{
    this->epollFd = epollFd;
    this->fd = fd;
    this->ssl = ssl;
}

void HttpConnection::setEvent(const bool &isReadEvent)
{
    this->isReadEvent = isReadEvent;
}

int HttpConnection::getFd() const
{
    return fd;
}


//ADAUGARE PENTRU GESTIONAREA TIPURILOR DE CERERI
void HttpConnection::handleHead(HttpRequest request)
{
    
    std::string fullPath = webRoot + request.uri;

    struct stat fileStat;
    if(stat(fullPath.c_str(), &fileStat) == -1)
    {
        this->responseHeader = RESPONSE_NOT_FOUND;
        this->responseBody.clear();
       
    }
    else{
        std::string contentType = getContentType(fullPath);
        char timeBuf[100];
        struct tm tm;
        gmtime_r(&fileStat.st_mtime, &tm);
        strftime(timeBuf, sizeof(timeBuf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
        const char* connectionState = isKeepAlive ? "keep-alive" : "close";
        
      
        this->responseHeader = HEAD_SUCCESS_HEADER;
        this->responseBody.clear();
        
    }
    epoll_event event{};
    event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
    event.data.fd = fd;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}


std::string HttpConnection::getContentType(const std::string& filePath) {
    static const std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".bmp", "image/bmp"},
        {".webp", "image/webp"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},
        {".eot", "application/vnd.ms-fontobject"},
        {".otf", "font/otf"}
    };

    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos) {
        std::string extension = filePath.substr(dotPos);
        auto it = mimeTypes.find(extension);
        if (it != mimeTypes.end()) {
            return it->second;
        }
    }
    return "application/octet-stream"; 
}

//-----------POST----------------

bool HttpConnection::isValidUsername(const std::string& username) {
    if (username.empty()) {
        return false;
    }

    return std::all_of(username.begin(), username.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c));
    });
}

bool HttpConnection::isValidPassword(const std::string& password) {
    if (password.empty()) {
        return false;
    }

    if (password.length() < 6) {
        return false;
    }

    bool hasLetter = false;
    bool hasDigit = false;
    for (char c : password) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            hasLetter = true;
        }
        if (std::isdigit(static_cast<unsigned char>(c))) {
            hasDigit = true;
        }
    }
    return hasLetter && hasDigit;
}

std::unordered_map<std::string, std::string> userDatabase = {
    {"admin", "admin123"},
    {"user", "user456"}
};

bool HttpConnection::authenticateUser(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end() && it->second == password) {
        return true;
    }
    return false;
}

std::string HttpConnection::urlDecode(const std::string& str) {
    std::string decoded;
    char ch;
    int i, ii;
    for (i = 0; i < str.length(); i++) {
        if (int(str[i]) == 37) {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            decoded += ch;
            i = i + 2;
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

std::unordered_map<std::string, std::string> HttpConnection::parseFormData(const std::string& body) {
    std::unordered_map<std::string, std::string> formData;
    std::istringstream stream(body);
    std::string pair;

    while (std::getline(stream, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            
            key = urlDecode(key);
            value = urlDecode(value);

            formData[key] = value;
        }
    }
    return formData;
}

std::string HttpConnection::compressStringGzip(const std::string& str) {
    if (str.empty()) {
        return std::string();
    }

    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;

   
    int ret = deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        throw std::runtime_error("deflateInit2 failed with error code: " + std::to_string(ret));
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = static_cast<uInt>(str.size());

    const size_t bufferSize = 32768;
    char outbuffer[bufferSize];
    std::string outstring;

  
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = static_cast<uInt>(bufferSize);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("Exception during zlib compression: " + std::to_string(ret));
    }

    return outstring;
}

std::string generateSimpleToken(const std::string& username) {
    return username + "_" + std::to_string(std::time(nullptr));
}

void saveTokenToFile(const std::string& token, const std::string& username) {
    std::ofstream file(Config::webRoot + "/tokens.txt", std::ios::app); 
    if (file.is_open()) {
        file << token << "," << username << "\n"; 
        file.close();
    } else {
        std::cerr << "Error: Unable to open tokens.txt for writing." << std::endl;
    }
}

void HttpConnection::handlePost(HttpRequest request) {
    std::string responseBody;
    std::string body = request.body;

    
    //std::cout << "DEBUG: Request Body: " << body << std::endl;

    std::string contentType = request.getHeader("Content-Type");

    
    //std::cout << "DEBUG: Content-Type: " << contentType << std::endl;

    if (contentType.empty()) {
        
        
        
        this->responseHeader = badRequestResponse_HEADER;
        this->responseBody.clear();
        return;
    }

    if (contentType == "application/x-www-form-urlencoded") {
        
        std::unordered_map<std::string, std::string> formData = parseFormData(body);
        std::string username = formData["username"];
        std::string password = formData["password"];

        
        //std::cout << "DEBUG: Username: " << username << ", Password: " << password << std::endl;

        bool validUsername = isValidUsername(username);
        bool validPassword = isValidPassword(password);
        std::string redirectResponse;

       
        //std::cout << "DEBUG: Valid Username: " << validUsername << ", Valid Password: " << validPassword << std::endl;

        if (validUsername && validPassword) {
            if (authenticateUser(username, password)) {
                
                //std::cout << "DEBUG: Authentication successful" << std::endl;

                std::string token = generateSimpleToken(username);
                saveTokenToFile(token, username);

                redirectResponse =
                    "HTTP/1.1 303 See Other\r\n"
                    "Location: /welcome.php?token=" + token + "\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
            } 
            else {
                
                redirectResponse =
                        "HTTP/1.1 303 See Other\r\n"
                        "Location: /login_fail.php\r\n" 
                        "Content-Length: 0\r\n"
                        "Connection: close\r\n"
                        "\r\n";
            }
           
            this->responseHeader = redirectResponse;
            this->responseBody.clear();
            return;
        }
        else
        {
             redirectResponse =
                        "HTTP/1.1 303 See Other\r\n"
                        "Location: /login_fail.php\r\n" 
                        "Content-Length: 0\r\n"
                        "Connection: close\r\n"
                        "\r\n";
             this->responseHeader = redirectResponse;
            this->responseBody.clear();
            return;
        }
    } else if (contentType == "application/json") {
        try {
            auto jsonData = nlohmann::json::parse(body);
            std::string username = jsonData["username"];
            std::string password = jsonData["password"];

            
            //std::cout << "DEBUG: JSON Username: " << username << ", JSON Password: " << password << std::endl;

            if (isValidUsername(username) && isValidPassword(password)) {
                if (authenticateUser(username, password)) {
                  
                    std::string token = generateSimpleToken(username);
                    saveTokenToFile(token, username);

                    std::string redirectResponse =
                        "HTTP/1.1 303 See Other\r\n"
                        "Location: /welcome.php?token=" + token + "\r\n"
                        "Content-Length: 0\r\n"
                        "Connection: close\r\n"
                        "\r\n";

                    
                    this->responseHeader = redirectResponse;
                    this->responseBody.clear();
                    return;
                } else {
                    responseBody = "<html><body><p>Login failed. Invalid username or password.</p></body></html>";
                }
            } else {
                responseBody = "<html><body><p>Invalid input.</p></body></html>";
            }
        } catch (const nlohmann::json::parse_error& e) {
            
            responseBody = "<html><body><p>Bad Request: JSON parsing error.</p></body></html>";
        }
    } else {
        
        responseBody = "<html><body><p>Unsupported Media Type.</p></body></html>";
    }


    epoll_event event{};
    event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
    event.data.fd = fd;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}

//---------------------END POST-------------------------



//------------------PUT-----------------------------


void HttpConnection::handlePut(HttpRequest request) {

    
    std::string relativePath = request.uri;

    
    if (!relativePath.empty() && relativePath[0] == '/') {
        relativePath = relativePath.substr(1);
    }

    
    std::string fileName = std::filesystem::path(relativePath).filename().string();
    std::string filePath = "web/upload/" + fileName;

    
    std::filesystem::path parentDir = std::filesystem::path(filePath).parent_path();
    if (!std::filesystem::exists(parentDir)) {
        std::filesystem::create_directories(parentDir);
    }

    
    std::string fileData = request.body;

    
    std::string contentEncoding = request.getHeader("Content-Encoding");
    if (contentEncoding == "gzip") {
        
        fileData = decompressGzip(fileData);
        if (fileData.empty()) {
            std::cerr << "Eroare: Decompresia datelor a eșuat." << std::endl;
            
            this->responseHeader = ERROR_HEADER;
            this->responseBody = ERROR_BODY;
            return;
        }
    } else if (!contentEncoding.empty()) {
        
        std::cerr << "Eroare: Content-Encoding nesuportat: " << contentEncoding << std::endl;
       
        this->responseHeader = UPLOAD_FAILED_HEADER;
        this->responseBody = UPLOAD_FAILED_BODY;
        return;
    }


    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Eroare: Nu s-a putut deschide fișierul pentru scriere: " << filePath << std::endl;
       
        this->responseHeader = ERROR_HEADER;
        this->responseBody = ERROR_BODY;
        return;
    }
    outFile.write(fileData.c_str(), fileData.size());
    outFile.close();


    std::string responseBody = "<html><body><p>Upload reușit!</p></body></html>";
    std::string response =
        "HTTP/1.1 200 Created\r\n"
        "Cache-Control: no-store, no-cache, must-revalidate\r\n"
        "Content-Length: " + std::to_string(responseBody.size()) + "\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n";


    
    this->responseHeader = response;
    this->responseBody = responseBody;

    
    epoll_event event{};
    event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
    event.data.fd = fd;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}

std::string HttpConnection::decompressGzip(const std::string& str) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) {
        std::cerr << "Eroare: Nu s-a putut inițializa decompresia." << std::endl;
        return "";
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = str.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        std::cerr << "Eroare: Decompresia a eșuat cu codul: " << ret << std::endl;
        return "";
    }

    return outstring;
}

//-------------------END PUT-------------------------------


void HttpConnection::handleDelete(HttpRequest request) {
    
    std::string relativePath = request.uri;
   
    
    if (!relativePath.empty() && relativePath[0] == '/') {
        relativePath = relativePath.substr(1);
    }

   
    std::string fileName = std::filesystem::path(relativePath).filename().string();

    
    std::string fullPath = "web/upload/" + fileName;

   
    std::filesystem::path fullPathAbsolute = std::filesystem::absolute(fullPath);

    
    std::filesystem::path uploadDirAbsolute = std::filesystem::absolute("web/upload");
    if (fullPathAbsolute.string().find(uploadDirAbsolute.string()) != 0) {
        std::cerr << "Tentativă de acces neautorizat detectată!" << std::endl;
       
        this->responseHeader = forbiddenResponse_HEADER;
        return;
    }

    
    if (!std::filesystem::exists(fullPathAbsolute)) {
        std::cerr << "Eroare: Fișierul nu există: " << fullPathAbsolute << std::endl;
       
        this->responseHeader = notFoundResponse_HEADER;
        return;
    }

    
    if (!std::filesystem::is_regular_file(fullPathAbsolute)) {
        std::cerr << "Eroare: Calea nu este un fișier: " << fullPathAbsolute << std::endl;
        
        this->responseHeader = badRequestResponse_HEADER;
        return;
    }

    
    try {
        std::filesystem::remove(fullPathAbsolute);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Eroare la ștergerea fișierului: " << e.what() << std::endl;
       
        this->responseHeader = internalServerErrorResponse_HEADER;
        return;
    }

    
    std::string responseBody = "<html><body><p>Fișier șters cu succes!</p></body></html>";
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Cache-Control: no-store, no-cache, must-revalidate\r\n"
        "Content-Length: " + std::to_string(responseBody.size()) + "\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n";

    
   
    this->responseHeader = response;
    this->responseBody = responseBody;

    
    epoll_event event{};
    event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
    event.data.fd = fd;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}

void HttpConnection::handleGet(HttpRequest request) {
   
    //std::cout << "DEBUG: URI cerut: " << request.uri << std::endl;
    std::string uri = request.uri.empty() ? "/" : request.uri;
    size_t queryPos = uri.find('?');
    std::string filePath = uri.substr(0, queryPos);
    std::string queryString = (queryPos != std::string::npos) ? uri.substr(queryPos + 1) : "";

    
    if (filePath == "/") {
        filePath = "index_test.html";
    }

    
    std::string acceptEncoding = request.getHeader("Accept-Encoding");
    bool clientAcceptsGzip = acceptEncoding.find("gzip") != std::string::npos;

   
    if (filePath.find("/files/") == 0) 
    {
        std::string fullPath = webRoot + filePath;

        if (!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath)) {
           
            this->responseHeader = ERROR_HEADER;
            this->responseBody = ERROR_BODY;
            handleWrite();
            return;
        }

        
        std::ifstream file(fullPath, std::ios::binary);
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        std::string responseBody = buffer.str();
        std::string responseBodyToSend = responseBody;
        std::string contentEncodingHeader;

       
        if (clientAcceptsGzip) {
            try {
                responseBodyToSend = compressStringGzip(responseBody);
                contentEncodingHeader = "Content-Encoding: gzip\r\n";
            } catch (const std::exception &e) {
                std::cerr << "Error compressing response: " << e.what() << std::endl;
                responseBodyToSend = responseBody; 
            }
        }

       
        std::string fileType = getContentType((char *)filePath.c_str());

       
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Cache-Control: no-store, no-cache, must-revalidate\r\n"
            "Content-Length: " + std::to_string(responseBodyToSend.size()) + "\r\n"
            "Content-Type: " + fileType + "\r\n" +
             "Content-Disposition: attachment; filename=\"" + std::filesystem::path(filePath).filename().string() + "\"\r\n" +
            contentEncodingHeader +
            "Connection: close\r\n"
            "\r\n";

       
        this->responseHeader = response;
        this->responseBody = responseBodyToSend;
       // std::cout << "DEBUG: Header trimis:\n" << this->responseHeader << std::endl;
        handleWrite();
        return;
    }

   
    if (filePath.find(".php") != std::string::npos) {
        std::string fullPath = webRoot + filePath;

        
        setenv("REQUEST_METHOD", "GET", 1);
        setenv("QUERY_STRING", queryString.c_str(), 1);
        setenv("SCRIPT_FILENAME", fullPath.c_str(), 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("HTTP_HOST", "localhost", 1);
        setenv("REDIRECT_STATUS", "1", 1);

       
         //folosim pipe pentru ca acesta preia iesirea generata de comanda php-cgi. Pipe-ul permite server-ului sa captureze continutul dinamic produs de scriptul php si sa il includa in raspunsul http trimis catre client
        //cand serverul executa un script php folosind php-cgi, iesirea este trimisa la stdout. de aceea folosim un pipe pentru a citi aceasta iesire si pentru a o include in raspunsul http catre client
        FILE *pipe = popen(("php-cgi " + fullPath).c_str(), "r");
        if (!pipe) {
            
           
            this->responseHeader = internalServerErrorResponse_HEADER;
            this->responseBody.clear();
            handleWrite();
            return;
        }

        std::stringstream output;
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output << buffer;
        }
        pclose(pipe);

        std::string phpOutput = output.str();
        size_t headerEnd = phpOutput.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            phpOutput = phpOutput.substr(headerEnd + 4); 
        }

        std::string responseBodyToSend = phpOutput;
        std::string contentEncodingHeader;

      
        if (clientAcceptsGzip) {
            try {
                responseBodyToSend = compressStringGzip(phpOutput);
                contentEncodingHeader = "Content-Encoding: gzip\r\n";
            } catch (const std::exception &e) {
                std::cerr << "Error compressing PHP output: " << e.what() << std::endl;
                responseBodyToSend = phpOutput; 
            }
        }

        
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Cache-Control: no-store, no-cache, must-revalidate\r\n"
            "Content-Length: " + std::to_string(responseBodyToSend.size()) + "\r\n"
            "Content-Type: text/html\r\n" +
            contentEncodingHeader +
            "Connection: close\r\n"
            "\r\n";

       
        this->responseHeader = response;
        this->responseBody = responseBodyToSend;
        handleWrite();
        return;
    }

    
    std::string fullPath = webRoot + filePath;
    if (!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath)) {
        
       
        this->responseHeader = ERROR_HEADER;
        this->responseBody = ERROR_BODY;
        handleWrite();
        return;
    }

    
    std::ifstream file(fullPath, std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string responseBody = buffer.str();
    std::string responseBodyToSend = responseBody;
    std::string contentEncodingHeader;

   
    if (clientAcceptsGzip) {
        try {
            responseBodyToSend = compressStringGzip(responseBody);
            contentEncodingHeader = "Content-Encoding: gzip\r\n";
        } catch (const std::exception &e) {
            std::cerr << "Error compressing response: " << e.what() << std::endl;
            responseBodyToSend = responseBody; 
        }
    }

    std::string fileType = getContentType((char *)filePath.c_str());

    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Cache-Control: no-store, no-cache, must-revalidate\r\n"
        "Content-Length: " + std::to_string(responseBodyToSend.size()) + "\r\n"
        "Content-Type: " + fileType + "\r\n" +
        contentEncodingHeader +
        "Connection: close\r\n"
        "\r\n";

   
    this->responseHeader = response;
    this->responseBody = responseBodyToSend;
    handleWrite();
}



void HttpConnection::handleWriteLargeResponse(int fileFd) {
    char buffer[DATA_LENGTH];  
    ssize_t bytesRead, bytesSent;

    while ((bytesRead = read(fileFd, buffer, sizeof(buffer))) > 0) {
        bytesSent = send(fd, buffer, bytesRead, 0);
        if (bytesSent == -1) {
            perror("Eroare la trimiterea datelor către client");
            break;
        }
    }
    close(fileFd);
}


// const char* HttpConnection::get_file_type(char* filename)
// {
//     if ( strstr(filename, ".html") || strstr(filename, ".php")) return TEXT_HTML;
//     else if ( strstr(filename, ".jar")) return APP_JS_ARCHIVE;
//     else if ( strstr(filename, ".js")) return TEXT_JS;
//     else if ( strstr(filename, ".ogg")) return APP_OGG;
//     else if ( strstr(filename, ".pdf")) return APP_PDF;
//     else if ( strstr(filename, ".json")) return APP_JSON;
//     else if ( strstr(filename, ".xml")) return APP_XML;
//     else if ( strstr(filename, ".zip")) return APP_ZIP;
//     else if ( strstr(filename, ".mp3")) return AUDIO_MPEG;
//     else if ( strstr(filename, ".wav")) return AUDIO_WAV;
//     else if ( strstr(filename, ".gif")) return IMAGE_GIF;
//     else if ( strstr(filename, ".jpeg") ||  strstr(filename, ".jpg")) return IMAGE_JPEG;
//     else if ( strstr(filename, ".png")) return IMAGE_PNG;
//     else if ( strstr(filename, ".tiff")) return IMAGE_TIFF;
//     else if ( strstr(filename, ".ico")) return IMAGE_VND_MSICON;
//     else if ( strstr(filename, ".json")) return APP_JSON;
//     else if ( strstr(filename, ".xml")) return APP_XML;
//     else if ( strstr(filename, ".mpeg")) return VIDEO_MPEG;
//     else if ( strstr(filename, ".mp4")) return VIDEO_MP4;
//     else if ( strstr(filename, ".webm")) return VIDEO_WEBM;
//     return TEXT_PLAIN;
// }