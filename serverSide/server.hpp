#ifndef SERVER_HPP
#define  SERVER_HPP

#include "client.hpp"
#include "../parsing/config.hpp"
#include <sys/stat.h>

class Client;
class serverconf;

class Server
{
    public:
        fd_set reads;
        fd_set write;
        SOCKET socket;
        std::list<Client *> clients;
        serverconf _server;
        void set_client(Server *client);
        void drop_client(Client *client, Server *c);
        std::string set_client_address(Client *ci);
        int wait_on_clients(Server *client);
};

void    default_errors(Client *client, Server *c, int error);
int     request_routing(Client *client, Server *c);
int     check_file(Client *client, std::string path, Server *c);
int     content_lenght_type(Client *client, std::string full_path);
int response_header(Client *client, int status, Server *c, std::string string);
int     response_file(Client *client, Server *c);
int     post_response(Client *client, Server *c);
int     delete_method(Client *client, Server *c);
int     error_page(Client *client, int error, Server *c);
int	auto_indexing(Client *client, Server *c);
int generate_autoindex_page(Client *client, Server *c, std::string& full_path);
int if_cgi(Client *client, Server *c, std::string cgi);
int correct_cgi(Client *client, Server *c, std::string meth);
int get_cgi(Client *client, std::string cgi);
int do_cgi(Client *client, Server *c);
int post_cgi(Client *client, std::string cgi);

std::string get_content_type(const std::string &path);
SOCKET create_socket(const char *host, const char *port);
std::string error_send(Client *client, Server *c);
int    request_parse(Client *client, Server *server);
#endif