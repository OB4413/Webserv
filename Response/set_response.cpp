#include "../includes/webserver.hpp"

void    Server::set_response(std::map<int, Request>::iterator &it_req, std::map<int, Response>::iterator &it_resp)
{
    it_resp->second.set_respomse = true;
    it_resp->second.fd_body_response = -2;
    std::cout << "I am here to set response" << std::endl;

    size_t size_location = 0;
    std::string full_path;
    size_t pos;
    std::vector<location>::iterator it_locations = this->configfile.locations.end();
    for (std::vector<location>::iterator i = this->configfile.locations.begin(); i != this->configfile.locations.end(); i++)
    {
        if ((pos = it_req->second.Path.find(i->path)) != std::string::npos && it_req->second.Path[pos + i->path.length()] == '/')
        {
            if (i->path.length() > size_location)
            {
                it_locations = i;
                size_location = i->path.length();
            }
        }
    }

    //full path
    if (it_locations != this->configfile.locations.end())
        full_path = it_locations->root + it_req->second.Path;
    else
        full_path = this->configfile.root + it_req->second.Path;

    std::cout << "#### "<< full_path << std::endl;
    //check if cgi or static file
    std::vector<std::pair<std::string, std::string> >::iterator it_ext_cgi = this->configfile.cgi_conf.end();
    for (std::vector<std::pair<std::string, std::string> >::iterator i = this->configfile.cgi_conf.begin(); i != this->configfile.cgi_conf.end(); i++)
    {
        if (full_path.find(i->first) != std::string::npos)
        {
            it_ext_cgi = i;
            break;
        }
    }

    if (it_ext_cgi != this->configfile.cgi_conf.end())
    {
        if (it_locations != this->configfile.locations.end())
        {
            std::vector<std::string>::iterator it_method;
            for (it_method = it_locations->allow_methods.begin(); it_method != it_locations->allow_methods.end(); it_method++)
            {
                if (*it_method == it_req->second.method)
                    break;
            }
            if (it_method != it_locations->allow_methods.end())
                std::cout << it_ext_cgi->first << "  " << it_ext_cgi->second << "  " << "exit cgi" << std::endl;
            else
                std::cout << "not allow method" << std::endl;
        }
        else
            std::cout << it_ext_cgi->first << "  " << it_ext_cgi->second << "  " << "exit cgi with out loction" << std::endl;
    }
    else if (it_locations != this->configfile.locations.end())
    {
        std::cout << "static file with location" << std::endl;
        std::vector<std::string>::iterator it_method;
        for (it_method = it_locations->allow_methods.begin(); it_method != it_locations->allow_methods.end(); it_method++)
        {
            if (*it_method == it_req->second.method)
                break;
        }
        if (it_method == it_locations->allow_methods.end())
            std::cout << "not allow method" << std::endl;
        else if (!it_locations->return_to.empty())
            std::cout << "301 " << it_locations->return_to << std::endl;
        else if (it_locations->autoindex)
            std::cout << "autoindex run" << std::endl;
        else if (!it_locations->index.empty())
            std::cout << "index of location" << std::endl;
        else
        {
            std::cout << "normal static file" << std::endl;
            if (full_path == it_locations->root + "/")
                it_resp->second.fd_body_response = open((it_locations->root + "/" + "index.html").c_str(), O_RDONLY);
            else
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
            if (it_resp->second.fd_body_response < 0)
            {
                if (it_locations != this->configfile.locations.end())
                    full_path = it_locations->root + this->configfile.error_page[404];
                else
                    full_path = this->configfile.root + this->configfile.error_page[404];
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
                if (it_resp->second.fd_body_response < 0)
                    throw std::runtime_error("i did not find the file error");
                else
                {
                    it_resp->second.exit_status = 404;
                    it_resp->second.message_status = "not fund";
                }
            }
        }
    }
    else
    {
        std::cout << "------normal static file" << std::endl;
        if (full_path == this->configfile.root + "/")
                it_resp->second.fd_body_response = open((this->configfile.root + "/" + "index.html").c_str(), O_RDONLY);
        else
            it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
        if (it_resp->second.fd_body_response < 0)
        {
            full_path = this->configfile.root + this->configfile.error_page[404];
            it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
            if (it_resp->second.fd_body_response < 0)
                throw std::runtime_error("i did not find the file error");
            else
            {
                it_resp->second.exit_status = 404;
                it_resp->second.message_status = "not fund";
            }
        }
    }
}