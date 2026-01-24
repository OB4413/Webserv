#include "../includes/webserver.hpp"

void    NonBlockSocket(int fd_socket)
{
    int flags = fcntl(fd_socket, F_GETFL, 0);
    fcntl(fd_socket, F_SETFL, flags | O_NONBLOCK);
}

void    Server::init_the_main_sockets_listing_end_epoll(std::vector<int> &sockets_fds, int &epoll_fd, epoll_event &evens_epoll)
{
    sockaddr_in addr_server;
    int opt = 1;
    int server_fd = 0;

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
}

void    Server::accept_connection(epoll_event *max_events, int i, epoll_event &evens_epoll, int epoll_fd)
{
    int client_fd;
    Request content_request;
    Response content_response;

    client_fd = accept(max_events[i].data.fd, NULL, NULL);
    if (client_fd == -1)
        return;
    NonBlockSocket(client_fd);
    content_request.byte_I_read = 0;
    content_request.size_header = 0;
    content_request.content_lenght = 0;
    content_request.pos = 0;
    this->clients_requests[client_fd] = content_request;
    content_response.set_respomse = false;
    content_response.header_send = false;
    this->clients_responses[client_fd] = content_response;
    evens_epoll.events = EPOLLIN;
    evens_epoll.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &evens_epoll) == -1)
        throw std::runtime_error("epoll ctl fialed");
}

void   Server::run(){
    std::vector<int> sockets_fds;
    int epoll_fd = 0;
    epoll_event evens_epoll;

    this->init_the_main_sockets_listing_end_epoll(sockets_fds, epoll_fd, evens_epoll);

    int num_of_evens;
    epoll_event max_events[1024];
    std::map<int , Request>::iterator it_request;
    std::map<int , Response>::iterator it_response;
    int of;
    char buffer[1024];

    while (true)
    {
        num_of_evens = epoll_wait(epoll_fd, max_events, 1023, -1);
        if (num_of_evens == -1)
            throw std::runtime_error("epoll wait fialed");
        for (int i = 0; i < num_of_evens; i++)
        {
            if (std::find(sockets_fds.begin(), sockets_fds.end(), max_events[i].data.fd) != sockets_fds.end())
                Server::accept_connection(max_events, i, evens_epoll, epoll_fd);
            else
            {
                if (max_events[i].events & EPOLLIN)
                {
                    of = recv(max_events[i].data.fd, buffer, 1023, 0);
                    it_request = this->clients_requests.lower_bound(max_events[i].data.fd);
                    if (of == -1)
                    {
                        this->clients_requests.erase(it_request);
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, max_events[i].data.fd, NULL) == -1)
                            throw std::runtime_error("epoll ctl fialed");
                        close (max_events[i].data.fd);
                        continue;
                    }
                    buffer[of] = '\0';
                    if (it_request != this->clients_requests.end())
                    {
                        it_request->second.request_str.append(buffer, of);
                        it_request->second.byte_I_read += of;
                    }
                    if (it_request->second.size_header == 0 && (it_request->second.pos = it_request->second.request_str.find("\r\n\r\n", it_request->second.pos)) != std::string::npos)
                    {
                        it_request->second.size_header = it_request->second.pos + 4;
                        if ((it_request->second.pos = it_request->second.request_str.find("Content-Length: ", 0)) != std::string::npos)
                        {
                            it_request->second.pos += 16;
                            for (size_t i = it_request->second.pos; !isdigit(it_request->second.request_str[i]); i++)
                                ;
                            it_request->second.content_lenght = atoll(it_request->second.request_str.substr(it_request->second.pos, i - it_request->second.pos).c_str());
                        }
                    }
                    if (it_request->second.byte_I_read - it_request->second.size_header >= it_request->second.content_lenght)
                    { 
                        this->parse_request(it_request);
                        evens_epoll.events = EPOLLIN | EPOLLOUT;
                        evens_epoll.data.fd = max_events[i].data.fd;
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, max_events[i].data.fd, &evens_epoll) == -1)
                            throw std::runtime_error("epoll ctl fialed");
                    }
                }
                else if (max_events[i].events & EPOLLOUT)
                {
                    it_response = this->clients_responses.lower_bound(max_events[i].data.fd);
                    it_request = this->clients_requests.lower_bound(max_events[i].data.fd);
                    if (!it_response->second.set_respomse)
                        this->set_response(it_request ,it_response);
                    std::string body;
                    char buf[1024];
                    int ofs;
                    ofs = read(it_response->second.fd_body_response, buf, 1023);
                    if (ofs < 0)
                        throw std::runtime_error("hhhhhhhh");
                    else if (ofs > 0)
                    {
                        body.append(buf);
                        struct stat fileinfo;
                        fstat(it_response->second.fd_body_response, &fileinfo);
                        std::stringstream response;
                        if (!it_response->second.header_send)
                        {
                            response << "HTTP/1.1 ";
                            response << it_response->second.exit_status << it_response->second.message_status << "\r\n";
                            response << "Content-Type: " << "text/html" << "\r\n";
                            response << "Content-Length: " << fileinfo.st_size << "\r\n";
                            response << "\r\n";
                            it_response->second.header_send = true;
                        }
                        response << body;

                        std::string res = response.str();
                        send(max_events[i].data.fd, res.c_str(), res.size(), 0);
                        response.str("");
                        response.clear();
                    }
                    else
                    {
                        close(it_response->second.fd_body_response);
                        this->clients_requests.erase(it_request);
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, max_events[i].data.fd, NULL) == -1)
                                throw std::runtime_error("epoll ctl fialed");
                        close (max_events[i].data.fd);
                    }
                }
                else
                {
                    it_request = this->clients_requests.lower_bound(max_events[i].data.fd);
                    this->clients_requests.erase(it_request);
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, max_events[i].data.fd, NULL) == -1)
                            throw std::runtime_error("epoll ctl fialed");
                    close (max_events[i].data.fd);
                }
            }
        }
    }
    for (std::vector<int>::iterator i = sockets_fds.begin(); i != sockets_fds.end(); i++)
        close(*i);
}