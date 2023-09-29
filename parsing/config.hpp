#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>


class location
{
    public:
        std::string path;
        std::vector<std::string> allow_methods;
        std::vector<std::string> index;
        bool auto_index;
        std::string root;
        std::string upload_pass;
        std::string _return;
        std::map<std::string, std::string> cgi_pass;
        location();
        ~location();
};

class serverconf
{
    public:
        std::list<location> loc;
        std::string host;
        std::string port;
        std::string server_name;
        long max_client_body_size;
        std::map<int, std::string> error_page;
        serverconf();
        ~serverconf();
};

class config
{
    private:
        std::list<serverconf> _serverconf;
        std::vector<std::vector<std::string> > indexs;
    public:
        int server_num;
        const std::list<serverconf> getserv();
        void    ft_openf(char *av, config *con);
        void	error_check(std::vector<std::string> vec, config *con, int sc);
};

#endif
