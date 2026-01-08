#include "includes/webserver.hpp"

int main(int ac, char **av) {
    Server server;
    try{
        server.configfile.init_the_header_conf_default();
        if (ac > 2)
            throw std::runtime_error("./program [config file]");
        else if (ac == 2)
            server.configfile.parse_config_file(av[1]);

        // praint config file
        // std::cout << server.configfile.listen[0] << std::endl;
        // std::cout << server.configfile.server_name << std::endl;
        // std::cout << server.configfile.host << std::endl;
        // std::cout << server.configfile.root << std::endl;
        // std::cout << server.configfile.client_max_body_size << std::endl;
        // std::cout << server.configfile.index << std::endl;
        // for (std::map<int, std::string>::iterator i = server.configfile.error_page.begin(); i != server.configfile.error_page.end(); i++)
        //     std::cout << i->first << "  " << i->second << std::endl;
        // std::cout << std::endl;
        // std::cout << std::endl;
        // for (std::vector<location>::iterator i = server.configfile.locations.begin(); i != server.configfile.locations.end(); i++)
        // {
        //     std::cout << i->path << std::endl;
        //     for (std::vector<std::string>::iterator j = i->allow_methods.begin(); j != i->allow_methods.end(); j++)
        //         std::cout << *j << ' ';
        //     std::cout << i->autoindex << std::endl; 
        //     std::cout << i->return_to << std::endl; 
        //     std::cout << i->root << std::endl;
        //     std::cout << i->index << std::endl;
        //     std::cout << std::endl;
        //     std::cout << std::endl;
        // }
        // std::cout << std::endl;
        // std::cout << std::endl;
        // for (std::vector<std::string>::iterator j = server.configfile.cgi_path.begin(); j != server.configfile.cgi_path.end(); j++)
        //         std::cout << *j << ' ';
        // for (std::vector<std::string>::iterator j = server.configfile.cgi_ext.begin(); j != server.configfile.cgi_ext.end(); j++)
        //         std::cout << *j << ' ';
        server.run();
    }
    catch (const std::exception &e){
        std::cerr << e.what() << '\n';
        return 1;
    }
}