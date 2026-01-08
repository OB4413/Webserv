#include "../includes/webserver.hpp"

void ConfigFile::init_the_header_conf_default(){
    this->listen.push_back(8080);
    this->server_name = "wedserv/1.0";
    this->host = "127.0.0.1";
    this->root = "pages";
    this->client_max_body_size = 1024;
    this->index = "index.html";
    this->error_page[403] = "errors/403.html";
    this->error_page[404] = "errors/404.html";
    this->error_page[405] = "errors/405.html";
    this->error_page[500] = "errors/500.html";
}


void   parse_location(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i, ConfigFile &conf){
    location location_to_push;
    location_to_push.autoindex = false;
    i++;
    location_to_push.path = *i;
    i++;
    if (i->compare("{"))
        throw std::runtime_error("error syntax (config file {)");
    i++;
    while (i != tokens.end())
    {
        if (!i->compare("allow_methods"))
        {
            i++;
            if (!i->compare(";"))
                throw std::runtime_error("error syntax (config file allow_methods)");
            while (i != tokens.end() && i->compare(";"))
            {
                if (i->compare("delete") && i->compare("post") && i->compare("get"))
                    throw std::runtime_error("error syntax (config file allow_methods)");
                location_to_push.allow_methods.push_back(*i);
                i++;
            }
            if (i == tokens.end())
                throw std::runtime_error("error syntax (config file allow_methods)");
        }
        else if (!i->compare("autoindex"))
        {
            i++;
            if (i->compare("on") && i->compare("off"))
                throw std::runtime_error("error syntax (config file autoindex)");
            if (!i->compare("on"))
                location_to_push.autoindex = true;
            else if (!i->compare("off"))
                location_to_push.autoindex = false;
            i++;
            if (i->compare(";"))
                throw std::runtime_error("error syntax (config file autoindex)");
        }
        else if (!i->compare("root") || !i->compare("return") || !i->compare("index"))
        {
            i++;
            if (!(i - 1)->compare("root"))
                location_to_push.root = *i;
            else if (!(i - 1)->compare("return"))
                location_to_push.return_to = *i;
            else if (!(i - 1)->compare("index"))
                location_to_push.index = *i;
            i++;
            if (i->compare(";"))
                throw std::runtime_error("error syntax (config file [root] [return] [index])");
        }
        else if (!i->compare("}"))
            break;
        else
            throw std::runtime_error("error syntax (config file)");
        i++;
    }
    conf.locations.push_back(location_to_push);
}


void   check_errors_and_init_config_server(std::vector<std::string> &tokens, ConfigFile &conf){
    int count = 0;
    int index = 0;
    std::vector<std::string>::iterator i = tokens.begin();
    while ((i = std::find(i, tokens.end(), "server")) != tokens.end())
    {
        if ((i = std::find(i, tokens.end(), "server")) != tokens.end())
            count++;
        i++;
    }
    if (count > 1)
        throw std::runtime_error("we do not work with multiple server (config file)");
    if (!tokens.begin()->compare("server") && !(tokens.begin() + 1)->compare("{") && !(tokens.end() - 1)->compare("}"))
    {
        for (i = tokens.begin(); i != tokens.end() - 1; i++)
        {
            if (!i->compare("server"))
                i++;
            else if (!i->compare("listen"))
            {
                if (index == 0)
                    conf.listen.clear();
                i++;
                for (size_t j = 0; j < i->size(); j++)
                {
                    if (!isdigit((*i)[j]))
                        throw std::runtime_error("error syntax (config file listen)");
                }
                int n = atoi(i->c_str());
                if (std::find(conf.listen.begin(), conf.listen.end(), n) != conf.listen.end())
                    throw std::runtime_error("error syntax (config file listen)");
                if (index == 0)
                {
                    conf.listen.push_back(n);
                    index++;
                }
                else
                    conf.listen.push_back(n);
                i++;
                if (i->compare(";") || n < 0 || n > 65535)
                    throw std::runtime_error("error syntax (config file listen)");
            }
            else if (!i->compare("server_name") || !i->compare("root") || !i->compare("index"))
            {
                i++;
                if (!(i - 1)->compare("server_name"))
                    conf.server_name = *i;
                if (!(i - 1)->compare("root"))
                    conf.root = *i;
                if (!(i - 1)->compare("index"))
                    conf.index = *i;
                i++;
                if (i->compare(";"))
                    throw std::runtime_error("error syntax (config file [server_name] [root] [index])");
            }
            else if (!i->compare("client_max_body_size"))
            {
                i++;
                for (size_t j = 0; j < i->size(); j++)
                {
                    if (!isdigit((*i)[j]))
                        throw std::runtime_error("error syntax (config file client_max_body_size)");
                }
                conf.client_max_body_size = atoi(i->c_str());
                i++;
                if (i->compare(";"))
                    throw std::runtime_error("error syntax (config file client_max_body_size)");
            }
            else if (!i->compare("host"))
            {
                i++;
                struct sockaddr_in sa;
                if (inet_pton(AF_INET, i->c_str(), &(sa.sin_addr)) != 1)
                    throw std::runtime_error("error syntax (config file host)");
                conf.host = *i;
                i++;
                if (i->compare(";"))
                    throw std::runtime_error("error syntax (config file host)");
            }
            else if (!i->compare("error_page"))
            {
                i++;
                for (size_t j = 0; j < i->size(); j++)
                {
                    if (!isdigit((*i)[j]))
                        throw std::runtime_error("error syntax (config file error_page)");
                }
                i++;
                conf.error_page[atoi((i - 1)->c_str())] = *i;
                i++;
                if (i->compare(";"))
                    throw std::runtime_error("error syntax (config file error_page)");
            }
            else if (!i->compare("location"))
                parse_location(tokens, i, conf);
            else if (!i->compare("cgi_path"))
            {
                i++;
                while (i != tokens.end() && i->compare(";"))
                {
                    conf.cgi_path.push_back(*i);
                    i++;
                }
                if (i == tokens.end())
                    throw std::runtime_error("error syntax (config file cgi_path)");
            }
            else if (!i->compare("cgi_ext"))
            {
                i++;
                while (i != tokens.end() && i->compare(";"))
                {
                    conf.cgi_ext.push_back(*i);
                    i++;
                }
                if (i == tokens.end())
                    throw std::runtime_error("error syntax (config file cgi_ext)");
            }
            else
                throw std::runtime_error("error syntax (config file)");
        }
    }
    else
        throw std::runtime_error("error syntax (config file server {})");
}


void    ConfigFile::parse_config_file(char *av)
{
    std::string line;
    std::string file;
    size_t pos;
    std::ifstream inFile(av);
    std::vector<std::string> tokens;

    if (!inFile.is_open())
        throw std::runtime_error("i can not open config file");
    while (true)
    {
        std::getline(inFile, line);
        if (line.empty() && inFile.eof())
            break;
        if ((pos = line.find("#", 0)) != std::string::npos)
            line.erase(pos, line.length());
        file.append(line);
        line.clear();
    }
    for (size_t i = 0; i < file.size(); i++)
    {
        if (file[i] >= 65 && file[i] <= 90)
            throw std::runtime_error("we work with lower case only (config file)");
    }
    size_t end = 0;
    size_t start;
    while (end < file.size())
    {
        while (end < file.size() && isspace(file[end]))
            end++;
        start = end;
        while (end < file.size() && !isspace(file[end]) && file[end] != '{' && file[end] != '}' && file[end] != ';')
            end++;
        if (file[end] == '{' || file[end] == '}' || file[end] == ';')
        {
            if (end != 0 && !isspace(file[end - 1]) && file[end - 1] != '{' && file[end - 1] != '}' && file[end - 1] != ';')
                tokens.push_back(file.substr(start, end - start));
            if (file[end] == ';')
                tokens.push_back(";");
            else if (file[end] == '{')
                tokens.push_back("{");
            else
                tokens.push_back("}");
            end++;
        }
        else
            tokens.push_back(file.substr(start, end - start));
    }
    check_errors_and_init_config_server(tokens, *this);
}
