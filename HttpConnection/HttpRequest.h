#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>

class HttpRequest {
public:
    std::string method;                      
    std::string uri;                         
    std::string version;                     
    std::map<std::string, std::string> headers; 
    std::string body;                       

    bool parse(char* buffer, unsigned long int size)
    {
        std::istringstream requestStream(std::string(buffer, size));
        std::string line;

    
    if (!std::getline(requestStream, line) || line.empty()) {
        return false;
    }

    std::istringstream lineStream(line);
    if (!(lineStream >> method >> uri >> version)) {
        return false;
    }

    
    std::transform(method.begin(), method.end(), method.begin(), ::toupper);

    
    while (std::getline(requestStream, line) && !line.empty() && line != "\r") {
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue; 
        }
        std::string headerName = line.substr(0, colonPos);
        std::string headerValue = line.substr(colonPos + 1);

        
        headerName.erase(headerName.begin(), std::find_if(headerName.begin(), headerName.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        headerName.erase(std::find_if(headerName.rbegin(), headerName.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), headerName.end());
        headerValue.erase(headerValue.begin(), std::find_if(headerValue.begin(), headerValue.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        headerValue.erase(std::find_if(headerValue.rbegin(), headerValue.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), headerValue.end());

        headers[headerName] = headerValue;
    }

    
    if (headers.find("Content-Length") != headers.end()) {
        size_t contentLength = std::stoul(headers["Content-Length"]);
        body.resize(contentLength);
        requestStream.read(&body[0], contentLength);
    }

    return true;
    }

    std::string getHeader(const std::string& headerName) const {
        auto it = headers.find(headerName);
        if (it != headers.end()) {
            return it->second;
        }
        return "";
    }

    bool IsKeepAlive() const {
        std::string connectionHeader = getHeader("Connection");
        return connectionHeader == "keep-alive";
}
    std::string getCookie(const std::string &name) const 
    {
        auto it = headers.find("Cookie");
        if (it != headers.end()) 
        {
            std::string cookies = it->second;
            size_t start = cookies.find(name + "=");
            if (start != std::string::npos) 
            {
                start += name.length() + 1; //move past "name="
                size_t end = cookies.find(";", start);
                return cookies.substr(start, end - start);
            }
        }
    return "";  //cookie not found
    }

};
