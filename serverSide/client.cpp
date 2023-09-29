#include "server.hpp"

void    Server::set_client(Server *client)
{
    Client *new_client = new Client;
    srand(time(NULL));
	std::stringstream ss;
	ss << rand();
    new_client->address_length = sizeof(new_client->address);
    ss >> new_client->cookieID;
    new_client->received = 0;
    new_client->resp_status = 0;
    new_client->_request.stat = 0;
    new_client->cgi_status = 0;
    new_client->socket =  accept(client->socket, (struct sockaddr*) &(new_client->address), &(new_client->address_length));
    int set = 1;
    setsockopt(new_client->socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
    if (!ISVALIDSOCKET(new_client->socket))
    {
        perror("accept : ");
        exit(1);
    }
    fcntl(new_client->socket, F_SETFL, O_NONBLOCK);
    client->clients.push_back(new_client);
}

void Server::drop_client(Client *client, Server *c) {
    
    CLOSESOCKET(client->socket);
    std::list<Client *>::iterator it = std::find(c->clients.begin(), c->clients.end(), client);
    if (it != c->clients.end()){
        delete *it;
        c->clients.erase(it);
        return ;
    }
    std::cerr << "drop_client not found.\n";
    exit(1);
} 

std::string Server::set_client_address(Client *ci)
{
    static char address_buffer[100];
    getnameinfo((struct sockaddr*)&ci->address, ci->address_length, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    return std::string(address_buffer);
}

int Server::wait_on_clients(Server *client)
{
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 300;
    FD_ZERO (&client->reads);
    FD_ZERO (&client->write);
    FD_SET(client->socket, &client->reads);
    SOCKET max_socket = client->socket;

    std::list<Client *>::iterator it;
    for (it = client->clients.begin(); it != client->clients.end(); it++)
    {
        Client *c = *it;
        FD_SET (c->socket, &client->reads);
        FD_SET (c->socket, &client->write);
        if (c->socket > max_socket){
            max_socket = c->socket;
        }
    }
    if (select(max_socket + 1, &client->reads, &client->write, 0, &time) < 0)
    {
        perror("Select :");
        return 1;
    }
    return 0;
}