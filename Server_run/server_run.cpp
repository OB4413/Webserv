#include "../includes/webserver.hpp"

void    NonBlockSocket(int fd_socket)
{
    int flags = fcntl(fd_socket, F_GETFL, 0);
    fcntl(fd_socket, F_SETFL, flags | O_NONBLOCK);
}

void   Server::run(){
    std::vector<int> sockets_fds;
    sockaddr_in addr_server;
    int opt = 1;
    int server_fd;
    int epoll_fd;
    epoll_event evens_epoll;

    if((epoll_fd = epoll_create(1024)) == -1)
        throw std::runtime_error("epoll create fialed");
    for (std::vector<int>::iterator i = this->configfile.listen.begin(); i != this->configfile.listen.end(); i++)
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
            throw std::runtime_error("socket creat fialed");
    
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
        NonBlockSocket(server_fd);

        evens_epoll.events = EPOLLIN;
        evens_epoll.data.fd = server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &evens_epoll) == -1)
            throw std::runtime_error("epoll ctl fialed");

        sockets_fds.push_back(server_fd);
    }

    int client_fd;
    int num_of_evens;
    epoll_event max_events[1024];

    while (true)
    {
        num_of_evens = epoll_wait(epoll_fd, max_events, 1024, -1);
        if (num_of_evens == -1)
            throw std::runtime_error("epoll wait fialed");
        for (int i = 0; i < num_of_evens; i++)
        {
            if (std::find(sockets_fds.begin(), sockets_fds.end(), max_events[i].data.fd) != sockets_fds.end())
            {
                // new connection
                client_fd = accept(sockets_fds[i], NULL, NULL);
                if (client_fd == -1)
                    continue;
                NonBlockSocket(client_fd);
                evens_epoll.events = EPOLLIN;
                evens_epoll.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &evens_epoll) == -1)
                    throw std::runtime_error("epoll ctl fialed");
            }
            else
            {
                // request from client
            }
        }
    }
    close(server_fd);
}