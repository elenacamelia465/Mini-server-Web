#include "Config.h"
#include <iostream>

Config::Config() = default;

Config::~Config() = default;

bool Config::isHttpsEnabled = false;
const std::string Config::webRoot = "./web";
unsigned short Config::threadPoolNumWorkers = 5;
uint16_t Config::webServerPort = 8086;
unsigned short Config::timeoutSecond = Config::defaultTimeoutSecond;

void Config::displayBanner() 
{
    std::cout << R"(
   
    ███╗   ███╗██╗███╗   ██╗██╗   ██╗    ██╗███████╗██████╗ 
    ████╗ ████║██║████╗  ██║██║   ██║    ██║██╔════╝██╔══██╗
    ██╔████╔██║██║██╔██╗ ██║██║   ██║ █╗ ██║█████╗  ██████╔╝
    ██║╚██╔╝██║██║██║╚██╗██║██║   ██║███╗██║██╔══╝  ██╔══██╗ 
    ██║ ╚═╝ ██║██║██║ ╚████║██║   ╚███╔███╔╝███████╗███████║ 
    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝    ╚══╝╚══╝ ╚══════╝╚══════╝   
    )" << "\n";
    std::cout << "Welcome to MiniWeb Server\n\n";
}

void Config::parseArgs(int argc, char **argv) 
{
    displayBanner(); // Display the ASCII art banner

    static struct option longOptions[] = 
    {
        {"port", required_argument, nullptr, 'p'},
        {"numWorker", required_argument, nullptr, 'n'},
        {"https", no_argument, nullptr, 's'}, 
        {"timeout", required_argument, nullptr, 't'}  
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "p:n:st:", longOptions, nullptr)) != -1) 
    {
        char* endPtr = nullptr;
        switch (opt) 
        {
            case 'p':
                webServerPort = static_cast<uint16_t>(strtol(optarg, &endPtr, 10));
                if (*endPtr != '\0' || webServerPort == 0) 
                {
                    std::cerr << "Valoare invalidă pentru port. Se folosește valoarea implicită: 8086.\n";
                    webServerPort = 8086;
                }
                break;
            case 'n':
                threadPoolNumWorkers = static_cast<unsigned short>(strtol(optarg, &endPtr, 10));
                if (*endPtr != '\0' || threadPoolNumWorkers == 0) 
                {
                    std::cerr << "Valoare invalidă pentru numărul de lucrători. Se folosește valoarea implicită: 5.\n";
                    threadPoolNumWorkers = 5;
                }
                break;
            case 's':
                isHttpsEnabled = true;
                break;
            case 't':
                if (!optarg) 
                {
                    std::cerr << "Lipsă valoare pentru timeout. Se folosește valoarea implicită: "<< defaultTimeoutSecond << " secunde.\n";
                    timeoutSecond = defaultTimeoutSecond;
                } 
                else 
                {
                    timeoutSecond = static_cast<unsigned short>(strtol(optarg, &endPtr, 10));
                    if (*endPtr != '\0' || timeoutSecond < 1) 
                    {
                        std::cerr << "Valoare invalidă pentru timeout. Se folosește valoarea implicită: "<< defaultTimeoutSecond << " secunde.\n";
                        timeoutSecond = defaultTimeoutSecond;
                    }
                }
                break;
            default:
                break;
        }
    }

    std::cout << "Modul: " << (isHttpsEnabled ? "HTTPS" : "HTTP") << "\n";
    std::cout << "Timeout: " << timeoutSecond << " secunde\n";
}
