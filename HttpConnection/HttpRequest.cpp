#include <sstream>
#include <algorithm>
#include <cctype>
#include "HttpRequest.h"

// bool HttpRequest::parse(char* buffer, unsigned long int size){
//     std::istringstream requestStream(std::string(buffer, size));
//     std::string line;

//     // Citirea primei linii: metoda, URI și versiunea
//     if (!std::getline(requestStream, line) || line.empty()) {
//         return false;
//     }

//     std::istringstream lineStream(line);
//     if (!(lineStream >> method >> uri >> version)) {
//         return false;
//     }

//     // Transformarea metodei în majuscule pentru consistență
//     std::transform(method.begin(), method.end(), method.begin(), ::toupper);

//     // Citirea anteturilor
//     while (std::getline(requestStream, line) && !line.empty() && line != "\r") {
//         auto colonPos = line.find(':');
//         if (colonPos == std::string::npos) {
//             continue; // Linie invalidă, o ignorăm
//         }
//         std::string headerName = line.substr(0, colonPos);
//         std::string headerValue = line.substr(colonPos + 1);

//         // Eliminarea spațiilor albe de la început și sfârșit
//         headerName.erase(headerName.begin(), std::find_if(headerName.begin(), headerName.end(), [](unsigned char ch) { return !std::isspace(ch); }));
//         headerName.erase(std::find_if(headerName.rbegin(), headerName.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), headerName.end());
//         headerValue.erase(headerValue.begin(), std::find_if(headerValue.begin(), headerValue.end(), [](unsigned char ch) { return !std::isspace(ch); }));
//         headerValue.erase(std::find_if(headerValue.rbegin(), headerValue.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), headerValue.end());

//         headers[headerName] = headerValue;
//     }

//     // Citirea corpului cererii, dacă există
//     if (headers.find("Content-Length") != headers.end()) {
//         size_t contentLength = std::stoul(headers["Content-Length"]);
//         body.resize(contentLength);
//         requestStream.read(&body[0], contentLength);
//     }

//     return true;
// }
