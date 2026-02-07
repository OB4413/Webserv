#include "../includes/webserver.hpp"
std::string know_content_type_by_path(std::string path)
{
    ssize_t pos;
    std::string extension;

    if ((pos = path.rfind(".", path.length() - 1)) != 0)
        extension = path.substr(pos, path.length() - pos);
    if (extension == ".html") return "Content-Type: text/html";
    else if (extension == ".png") return "Content-Type: image/png";
    else if (extension == ".css") return "Content-Type: text/css";
    else if (extension == ".jpg") return "Content-Type: image/jpeg";
    else if (extension == ".mp4") return "Content-Type: video/mp4";
    else return "Content-Type: application/octet-stream";
}

void    Server::set_response(std::map<int, Request>::iterator &it_req, std::map<int, Response>::iterator &it_resp)
{
    it_resp->second.set_respomse = true;
    it_resp->second.fd_body_response = -2;
    std::cout << "I am here to set response" << std::endl;

    size_t size_location = 0;
    std::string full_path;
    size_t pos;
    std::vector<location>::iterator it_locations = this->configfile.locations.end();
    full_path = this->configfile.root + it_req->second.Path;
    for (std::vector<location>::iterator i = this->configfile.locations.begin(); i != this->configfile.locations.end(); i++)
    {
        if ((pos = full_path.find(i->path)) != std::string::npos && (pos = full_path.length() - i->path.length() || full_path[pos + i->path.length()] == '/'))
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
        std::vector<std::string>::iterator it_method;
        if (it_locations != this->configfile.locations.end())
        {
            for (it_method = it_locations->allow_methods.begin(); it_method != it_locations->allow_methods.end(); it_method++)
            {
                if (*it_method == it_req->second.method)
                    break;
            }
            if (it_method == it_locations->allow_methods.end())
            {
                full_path = it_locations->root + this->configfile.error_page[405];
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
                if (it_resp->second.fd_body_response < 0)
                    it_resp->second.fd_body_response = open("www/Errors/405.html", O_RDONLY);
                it_resp->second.exit_status = 405;
                it_resp->second.message_status = "Method Not Allowed";
                it_resp->second.content_type = "Content-Type: text/html";
            }
        }
        if (it_method != it_locations->allow_methods.end())
        {
            std::cout << it_ext_cgi->first << "  " << it_ext_cgi->second << "  " << "exit cgi with out loction" << std::endl;
        }
    }
    else if (it_locations != this->configfile.locations.end())
    {
        std::cout << "###### static file with location" << std::endl;
        struct stat DataFolder;
        std::vector<std::string>::iterator it_method;
        for (it_method = it_locations->allow_methods.begin(); it_method != it_locations->allow_methods.end(); it_method++)
        {
            if (*it_method == it_req->second.method)
                break;
        }
        if (it_method == it_locations->allow_methods.end())
        {
            std::cout << "not allow method" << std::endl;
            full_path = it_locations->root + this->configfile.error_page[405];
            it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
            if (it_resp->second.fd_body_response < 0)
                it_resp->second.fd_body_response = open("www/Errors/405.html", O_RDONLY);
            it_resp->second.exit_status = 405;
            it_resp->second.message_status = "Method Not Allowed";
            it_resp->second.content_type = "Content-Type: text/html";
        }
        else if (!it_locations->return_to.empty())
        {
            std::cout << "301 " << it_locations->return_to << std::endl;
            it_resp->second.return_to = it_locations->return_to;
            it_resp->second.exit_status = 301;
            it_resp->second.message_status = "redirect";
        }
        else if (it_locations->autoindex && !stat(full_path.c_str(), &DataFolder) && S_ISDIR(DataFolder.st_mode))
        {
            std::cout << "autoindex run" << std::endl;
            DIR* dir = opendir(full_path.c_str());
            struct dirent* entry;
            
            it_resp->second.req_autoindex = 
                "<html>"
                "<body style=\"font-family:Arial;background:#0f1220;color:#e6e7ee;padding:20px\">"
                "<h1>Index of " + full_path + "</h1>"
                "<ul style=\"list-style:none;padding:0\">";

            while ((entry = readdir(dir)) != NULL)
            {
                std::string name = entry->d_name;
                if (name != "." && name != "..")
                {
                    it_resp->second.req_autoindex +=
                        "<li style=\"margin:8px 0\">"
                        "<a style=\"color:#6c7cff;text-decoration:none\" href=\"" + it_req->second.Path + "/" + name + "\">"
                        + name +
                        "</a></li>";
                }
            }
            it_resp->second.req_autoindex += "</ul></body></html>";
            closedir(dir);
            it_resp->second.exit_status = 200;
            it_resp->second.message_status = "OK";
            it_resp->second.content_type = "Content-Type: text/html";
        }
        else if (it_req->second.method == "GET")
        {
            std::cout << "+++++++++normal static file" << std::endl;
            if (full_path == it_locations->root + "/" + it_locations->path)
            {
                if (!it_locations->index.empty())
                    it_resp->second.fd_body_response = open((it_locations->root + "/" + it_locations->index).c_str(), O_RDONLY);
                if (it_resp->second.fd_body_response < 0 || it_locations->index.empty())
                    it_resp->second.fd_body_response = open("www/index.html", O_RDONLY);
                it_resp->second.content_type = "text/html";
            }
            else
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
            if (it_resp->second.fd_body_response < 0)
            {
                full_path = it_locations->root + "/" + this->configfile.error_page[404];
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
                if (it_resp->second.fd_body_response < 0)
                    it_resp->second.fd_body_response = open("www/Errors/404.html", O_RDONLY);
                it_resp->second.exit_status = 404;
                it_resp->second.message_status = "Not Fuond";
                it_resp->second.content_type = "text/html";
            }
            else
            {
                it_resp->second.exit_status = 200;
                it_resp->second.message_status = "OK";
                if(full_path != it_locations->root + "/" + it_locations->path)
                    it_resp->second.content_type = know_content_type_by_path(full_path);
            }
        }
        else if (it_req->second.method == "POST")
        {
            if (!it_req->second.filename.empty() && this->configfile.client_max_body_size >= (int)it_req->second.body.length())
            {
                it_resp->second.fd_body_response = open("www/upsu.html", O_RDONLY);
                it_resp->second.exit_status = 200;
                it_resp->second.message_status = "OK";
                it_resp->second.content_type = "text/html";
            }
            else if (!it_req->second.data_kyvl.empty())
            {
                it_resp->second.fd_body_response = open("www/signin.html", O_RDONLY);
                it_resp->second.exit_status = 200;
                it_resp->second.message_status = "OK";
                it_resp->second.content_type = "text/html";
            }
            else
            {
                full_path = it_locations->root + "/" + this->configfile.error_page[403];
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
                if (it_resp->second.fd_body_response < 0)
                    it_resp->second.fd_body_response = open("www/Errors/403.html", O_RDONLY);
                it_resp->second.exit_status = 403;
                it_resp->second.message_status = "Forbidden";
                it_resp->second.content_type = "text/html";
            }
        }
        else if (it_req->second.method == "DELETE")
        {
            if(std::remove(full_path.c_str()) == 0)
            {
                it_resp->second.fd_body_response = open("www/desu.html", O_RDONLY);
                it_resp->second.exit_status = 200;
                it_resp->second.message_status = "KO";
                it_resp->second.content_type = "text/html";
            }
            else
            {
                it_resp->second.fd_body_response = open("www/Error/101.html", O_RDONLY);
                it_resp->second.exit_status = 404;
                it_resp->second.message_status = "Not Found";
                it_resp->second.content_type = "text/html";
            }
        }
        else
        {
            it_resp->second.fd_body_response = open("www/Error/405.html", O_RDONLY);
            it_resp->second.exit_status = 405;
            it_resp->second.message_status = "Method Not Allowed Page";
            it_resp->second.content_type = "text/html";
        }
    }
    else
    {
        if (it_req->second.method == "GET")
        {
            std::cout << "------normal static file" << std::endl;
            if (full_path == this->configfile.root + "/")
            {
                if (!this->configfile.index.empty())
                    it_resp->second.fd_body_response = open((this->configfile.root + "/" + this->configfile.index).c_str(), O_RDONLY);
                if (it_resp->second.fd_body_response < 0 || this->configfile.index.empty())
                    it_resp->second.fd_body_response = open("www/index.html", O_RDONLY);
                it_resp->second.content_type = "text/html";
            }
            else
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
            if (it_resp->second.fd_body_response < 0)
            {
                full_path = this->configfile.root + this->configfile.error_page[404];
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
                if (it_resp->second.fd_body_response < 0)
                    it_resp->second.fd_body_response = open("www/Errors/404.html", O_RDONLY);
                it_resp->second.exit_status = 404;
                it_resp->second.message_status = "Not Fuond";
                it_resp->second.content_type = "text/html";
            }
            else
            {
                it_resp->second.exit_status = 200;
                it_resp->second.message_status = "OK";
                if (full_path != this->configfile.root + "/")
                    it_resp->second.content_type = know_content_type_by_path(full_path);
            }
        }
        else if (it_req->second.method == "POST")
        {
            if (!it_req->second.filename.empty() && this->configfile.client_max_body_size >= (int)it_req->second.body.length())
            {
                int fd = open(("www/upload/" + it_req->second.filename).c_str(), O_CREAT | O_RDWR, 777);
                size_t pos = it_req->second.body.find("\r\n\r\n");
                size_t start = pos + 4;
                size_t lenght = it_req->second.body.length() - (pos + 4);
                pos = it_req->second.body.find("\r\n\r\n", pos);
                lenght -= it_req->second.body.length() - (pos - 2);
                write(fd, (it_req->second.body.substr(start, pos - start)).c_str(), lenght);
                close(fd);
                it_resp->second.fd_body_response = open("www/upsu.html", O_RDONLY);
                it_resp->second.exit_status = 200;
                it_resp->second.message_status = "OK";
                it_resp->second.content_type = "text/html";
            }
            else if (!it_req->second.data_kyvl.empty())
            {

                it_resp->second.fd_body_response = open("www/signin.html", O_RDONLY);
                it_resp->second.exit_status = 200;
                it_resp->second.message_status = "OK";
                it_resp->second.content_type = "text/html";
            }
            else
            {
                full_path = it_locations->root + "/" + this->configfile.error_page[403];
                it_resp->second.fd_body_response = open(full_path.c_str(), O_RDONLY);
                if (it_resp->second.fd_body_response < 0)
                    it_resp->second.fd_body_response = open("www/Errors/403.html", O_RDONLY);
                it_resp->second.exit_status = 403;
                it_resp->second.message_status = "Forbidden";
                it_resp->second.content_type = "text/html";
            }
        }
        else if (it_req->second.method == "DELETE")
        {
            if(std::remove(full_path.c_str()) == 0)
            {
                it_resp->second.fd_body_response = open("www/desu.html", O_RDONLY);
                it_resp->second.exit_status = 200;
                it_resp->second.message_status = "KO";
                it_resp->second.content_type = "text/html";
            }
            else
            {
                it_resp->second.fd_body_response = open("www/Error/101.html", O_RDONLY);
                it_resp->second.exit_status = 404;
                it_resp->second.message_status = "Not Found";
                it_resp->second.content_type = "text/html";
            }
        }
        else
        {
            it_resp->second.fd_body_response = open("www/405.html", O_RDONLY);
            it_resp->second.exit_status = 405;
            it_resp->second.message_status = "Method Not Allowed Page";
            it_resp->second.content_type = "text/html";
        }
    }
}