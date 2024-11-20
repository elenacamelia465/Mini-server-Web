#include "Config.h"
#include <iostream>

Config::Config() = default;

Config::~Config() = default;
bool Config::isHttpsEnabled = false;
const std::string Config::webRoot = "./www";
//the server will use 5 worker threads in its thread pool
unsigned short Config::threadPoolNumWorkers = 5;
uint16_t Config::webServerPort=8086;



void Config::parseArgs(int argc, char **argv) 
{
    static struct option longOptions[] = 
    {
        {"port", required_argument, nullptr, 'p'},
        {"numWorker", required_argument, nullptr, 'n'},
        {"https", no_argument, nullptr, 's'}  // Adăugăm opțiunea HTTPS
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "p:n:s", longOptions, nullptr)) != -1) 
    {
        switch (opt) 
        {
            case 'p':
                webServerPort = static_cast<uint16_t>(strtol(optarg, nullptr, 10));
                break;
            case 'n':
                threadPoolNumWorkers = static_cast<unsigned short>(strtol(optarg, nullptr, 10));
                break;
            case 's':
                isHttpsEnabled = true;  // Activăm modul HTTPS
                break;
            default:
                break;
        }
    }

    std::cout << "Modul: " << (isHttpsEnabled ? "HTTPS" : "HTTP") << "\n";
}