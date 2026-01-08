#include "../includes/webserver.hpp"

void   Server::run(){
    std::vector<int> sockets_fds;
    sockaddr_in addr_server;
    int opt = 1;
    int server_fd;

    // creat sockets for every port
    for (std::vector<int>::iterator i = this->configfile.listen.begin(); i != this->configfile.listen.end(); i++)
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
        memset(&addr_server, 0, sizeof(addr_server));
        addr_server.sin_family = AF_INET;
        addr_server.sin_port = htons(*i);
        addr_server.sin_addr.s_addr = inet_addr(this->configfile.host.c_str());
    
        std::cout << this->configfile.host << ':' << *i << std::endl;
    
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("REUSADDR is fialed");
    
        if (bind(server_fd, (sockaddr *)&addr_server, sizeof(addr_server)) == -1)
            throw std::runtime_error("bind fialed");
    
        if (listen(server_fd, 1024) < 0)
            throw std::runtime_error("listen fialed");
        sockets_fds.push_back(server_fd);
    }

    // small test of accept and recv and send
    while (true)
    {
        int client_fd = accept(sockets_fds[0], NULL, NULL);
        char buffer[5000];
        recv(client_fd, buffer, 5000 - 1, 0);
        std::cout << buffer << std::endl;
        std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 55\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>\r\n"
        "<html>\r\n"
        "<body>\r\n"
        "<h1>Welcome to Webserv</h1>\r\n"
        "</body>\r\n"
        "</html>";
        send(client_fd, response.c_str(), 1024, 0);
    }
}