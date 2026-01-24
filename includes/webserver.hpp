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
#include <cstdlib>
#include <ctime>
#include <iostream>


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
    std::vector<std::pair<std::string, std::string> > cgi_conf;

    void  init_the_header_conf_default();
    void  parse_config_file(char *av);
};

// ###################################



// ###################################
// class of Request                  #
// ###################################

class Request {
    public:
        std::string request_str;
        long long byte_I_read;
        long long size_header;
        size_t pos;

        //data of request
        std::string method;
        std::string Path; // it is relative_path in delete
        long long content_lenght;
        std::string Content_Type;
        std::string header;
        std::string body;
        std::string boundary;
        std::map<std::string, std::string> data_kyvl;
        std::string filename;
};

// ###################################



// ###################################
// class of Response                 #
// ###################################

class Response {
    public:
        bool header_send;
        bool set_respomse;
        int exit_status;
        std::string message_status;
        int fd_body_response;
};

// ###################################



// ###################################
// class of config file              #
// ###################################

class Server {
    public:
        ConfigFile configfile;
        std::map<int, Request> clients_requests;
        std::map<int, Response> clients_responses;

        void    run();
        void    init_the_main_sockets_listing_end_epoll(std::vector<int> &sockets_fds, int &epoll_fd, epoll_event &evens_epoll);
        void    accept_connection(epoll_event *max_events, int i, epoll_event &evens_epoll, int epoll_fd);
        void    parse_request(std::map<int, Request>::iterator &it);
        void    set_response(std::map<int, Request>::iterator &it_req, std::map<int, Response>::iterator &it_resp);
};

// ###################################