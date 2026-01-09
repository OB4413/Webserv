#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <arpa/inet.h>
#include <unistd.h>

#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/epoll.h>


// ###################################
// class of config file              #
// ###################################

typedef struct location
{
    std::string path;
    std::vector<std::string> allow_methods;
    bool autoindex;
    std::string root;
    std::string return_to;
    std::string index;
} location;


class ConfigFile {
  public:
    std::vector<int> listen;
    std::string server_name;
    std::string host;
    std::string root;
    int client_max_body_size;
    std::string index;
    std::map<int, std::string> error_page;
    std::vector<location> locations;
    std::vector<std::string> cgi_path;
    std::vector<std::string> cgi_ext;

    void  init_the_header_conf_default();
    void  parse_config_file(char *av);
};

// ###################################



// ###################################
// class of Request                  #
// ###################################

class Request {
    public:
        std::string path;
};

// ###################################



// ###################################
// class of Response                  #
// ###################################

class Response {
    public:
        std::string Response;
};

// ###################################




// ###################################
// class of config file              #
// ###################################

class Server {
    public:
        ConfigFile configfile;
        Request request;
        Response response;

        void    run();
};

// ###################################