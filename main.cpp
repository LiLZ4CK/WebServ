#include "serverSide/client.hpp"
#include "serverSide/server.hpp"
#include "parsing/config.hpp"

int main(int ac, char **av)
{
	if (ac != 2 && ac != 1)
		return 0;
	config *_con = new config;
	try {
		_con->ft_openf(av[1], _con);
	}
	catch (std::exception &e){
		std::cout << e.what() << '\n';
		exit(1);
	}
	Server *server = new Server[_con->server_num];
	std::list<serverconf> servlist = _con->getserv();
	std::list<serverconf>::iterator it = servlist.begin();
	for (int i = 0; i < _con->server_num; i++)
	{
		server[i]._server = *it++;
		if (server[i]._server.host.empty())
		{
			server[i].socket = create_socket(server[i]._server.server_name.c_str(), server[i]._server.port.c_str());
			if (server[i].socket == -1)
				exit(1);
		}
		else
		{
			server[i].socket = create_socket(server[i]._server.host.c_str(), server[i]._server.port.c_str());
			if (server[i].socket == -1)
			{
				server[i].socket = create_socket(server[i]._server.server_name.c_str(), server[i]._server.port.c_str());
				if (server[i].socket == -1)
				exit(1);
			}
		}
	}
	signal(SIGPIPE, SIG_IGN);
	while (1)
	{
		for (int c = 0; c < _con->server_num; c++)
		{
			if (server->wait_on_clients(&server[c]) == 1)
				continue;
			if (FD_ISSET(server[c].socket, &server[c].reads))
				server->set_client(&server[c]);
			std::list<Client *>::iterator it = server[c].clients.begin();
			while (it != server[c].clients.end())
			{
				if (FD_ISSET((*it)->socket, &server[c].reads) && (*it)->resp_status == 0)
				{
					(*it)->recv = recv((*it)->socket, (*it)->request, MAX_REQUEST_SIZE, 0);
					if ((*it)->recv < 1)
					{
						error_page((*it), 500, &server[c]);
						it = server[c].clients.begin();
						continue;
					}
					(*it)->received += (*it)->recv;
					if ((*it)->_request.stat == 0)
					{
						if (request_parse(*it, &server[c]) == 1)
						{
							it = server[c].clients.begin();
							continue;
						}
						if (check_file((*it), (*it)->_request.path1, &server[c]) == 1){
							it = server[c].clients.begin();
							continue;
						}
					}
					if ((*it)->received > server[c]._server.max_client_body_size)
					{
						error_page((*it), 413, &server[c]);
						it = server[c].clients.begin();
						continue;
					}
					if (request_routing(*it, &server[c]) == 1)
					{
						it = server[c].clients.begin();
						continue;
					}
				}
				else if (FD_ISSET((*it)->socket, &server[c].write) && (*it)->resp_status == 1)
				{
					if ((*it)->cgi_status == 1)
					{
						int k = do_cgi((*it), &server[c]);
						if (k == 1){
							it = server[c].clients.begin();
							continue;
						}
						else if (k == 2){
							it++;
							continue;
						}
					}
					if (response_file(*it, &server[c]) == 1){
						it = server[c].clients.begin();
						continue;
					}
				}
				if (server[c].clients.size() <= 1)
				{
					it = server[c].clients.begin();
				}
				it++;
			}
		}
	}
	return 0;
}
