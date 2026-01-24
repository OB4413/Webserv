#include "../includes/webserver.hpp"

void    Server::parse_request(std::map<int, Request>::iterator &it)
{
    size_t start = 0;
    size_t end = 0;
    size_t pos = 0;
    size_t end_key = 0;
    size_t start_key = 0;
    size_t end_value = 0;
    size_t start_value = 0;
    std::stringstream ss;

    std::srand(std::time(0));
    if (it != this->clients_requests.end() && !it->second.request_str.empty())
    {
        it->second.header = it->second.request_str.substr(0, it->second.size_header);
        it->second.body = it->second.request_str.substr(it->second.size_header, it->second.request_str.length() - it->second.size_header);
        for (;!isspace(it->second.header[end]); end++)
            ;
        it->second.method = it->second.header.substr(0, end);
        end++;
        start = end;
        for (;!isspace(it->second.header[end]); end++)
            ;
        it->second.Path = it->second.header.substr(start, end - start);
        if ((start = it->second.header.find("Content-Type: ", 0)) != std::string::npos)
        {
            start += 14;
            if ((end = it->second.header.find("\r\n", start)) != std::string::npos)
                it->second.Content_Type = it->second.header.substr(start, end - start);
        }

        if (it->second.method == "POST")
        {
            if (it->second.Content_Type == "application/x-www-form-urlencoded")
            {
                while ((pos = it->second.body.find("=", pos)) != std::string::npos)
                {
                    end_key = pos;
                    start_value = pos + 1;
                    if ((pos = it->second.body.find("&", pos)) != std::string::npos)
                    {
                        end_value = pos;
                        it->second.data_kyvl[it->second.body.substr(start_key, end_key - start_key)] = it->second.body.substr(start_value, end_value - start_value);
                        start_key = pos + 1;
                    }
                    else
                    {
                        end_value = it->second.body.length();
                        it->second.data_kyvl[it->second.body.substr(start_key, end_key - start_key)] = it->second.body.substr(start_value, end_value - start_value);
                        break;
                    }
                }
                
            }
            else if (it->second.Content_Type.find("multipart/form-data") != std::string::npos)
            {
                it->second.boundary = it->second.Content_Type.substr(30, it->second.Content_Type.length() - 30);
                if ((pos = it->second.body.find("filename=\"", 0)) != std::string::npos)
                {
                    pos += 11;
                    start_value = pos - 1;
                    if ((pos = it->second.body.find("\"", pos)) != std::string::npos)
                        end_value = pos;
                    else
                        end_value = it->second.body.length();
                    it->second.filename = it->second.body.substr(start_value, end_value - start_value);
                }
                else
                {
                    pos = 0;
                    if ((pos = it->second.body.find("Content-Type: ", pos)) != std::string::npos)
                    {
                        pos += 14;
                        if ((pos = it->second.body.find("/", pos)) != std::string::npos)
                            start_value = pos + 1;
                        if ((pos = it->second.body.find("\r\n", pos)) != std::string::npos)
                            end_value = pos;
                        ss << std::rand();
                        it->second.filename = ss.str() + "." + it->second.body.substr(start_value, end_value - start_value);
                    }
                    else
                    {
                        ss << std::rand();
                        it->second.filename = ss.str() + ".txt";
                    }
                }
            }
        }
    }
    
}