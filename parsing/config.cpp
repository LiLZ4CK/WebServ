#include "config.hpp"
#include <fstream>
#include <sstream>

void    ft_error(std::string s){
    std::cout << s;
    exit (1);
}

location::location(){
	this->path = "";
	this->index.push_back("index.html");
	this->auto_index = false;
	this->root = "public";
}

serverconf::serverconf(){
	this->port = "8080";
	this->host = "127.0.0.1";
	this->max_client_body_size = 1000000;
}

location::~location(){}
serverconf::~serverconf(){}

void	ft_check_file(std::string str)
{
	int	i;
	int	j;
    std::string conf = ".conf";
	i = str.size() - conf.size();
	if (i <= 0 || str[i] != '.')
		ft_error("Wrong File!!\n");
	j = 0;
	while (str[i + j] != '\0' && conf[j] != '\0')
	{
		if (str[i + j] == conf[j])
			j++;
		else
			break ;
	}
	if (!(str[i + j] == '\0' && conf[j] == '\0'))
		ft_error("That's Wrong!!, It Should be (*.conf)\n");
}

bool is_integer(const std::string& str) {
    for (size_t i=0;i<str.size();i++) {
        if (std::isdigit(str[i]) == 0) return false;
    }
    return true;
}

const std::list<serverconf> config::getserv(){
	return (this->_serverconf);
}

void	config::error_check(std::vector<std::string> vec, config *con, int sc){
	std::vector<std::string>::iterator it = vec.begin();
	std::vector<std::string>::iterator ite = vec.end();
	while (sc){
		int host=0;int port=0;int mbs=0;int sloc=0;int sn=0;
		serverconf serv;
		if (*it == "server"){
			it++;
			if (*it == "{"){
				it++;
				while (it != ite && *it != "}"){
					if (*it == "\n"){
						it++;
						continue;
					}
					if (*it == "host"){
						if (host)
							ft_error("Error: duplicate\n");
						it++;
						serv.host = *it;
						host = 1;
					}
					else if (*it == "port"){
						if (port)
							ft_error("Error: duplicate\n");
						it++;
						serv.port = *it;
						port = 1;
					}
					else if (*it == "max_client_body_size"){
						if (mbs)
							ft_error("Error :duplicate\n");
						it++;
						if ((*it).size() > 18)
							ft_error("Error: max_client_body_size to long\n");
						if (!is_integer(*it))
							ft_error("Error: max_client_body_size should be an integer\n");
						serv.max_client_body_size = std::strtol((*it).c_str(), NULL, 10);
						mbs = 1;
					}
					else if (*it == "error_page"){
						it++;
						if (!is_integer(*it))
							ft_error("Error: error_page first param should be an integer\n");
						int n = atoi((*it).c_str());
						it++;
						serv.error_page.insert(std::make_pair(n, *it));
					}
					else if (*it == "server_name"){
						if (sn)
							ft_error("Error: duplicate\n");
						it++;
						serv.server_name = *it;
						sn = 1;
					}
					else if (*it == "location"){
						location loc;sloc++;
						int am=0;int ind=0;int auind=0;int root=0;
						int up=0;int ret=0;
						it++;
						loc.path = *it++;
						if (*it == "{"){
							it++;
							while (it != ite && *it != "}"){
								if (*it == "allow_methods"){
									if (am)
										ft_error("Error: duplicate\n");
									it++;
									loc.allow_methods.clear();
									for (int i = 0; i < 3; i++){
										if (*it != "GET" && *it != "POST" && *it != "DELETE")
											break;
										loc.allow_methods.push_back(*it);
										it++;
									}
									it--;
									am = 1;
								}
								else if (*it == "index"){
									if (ind)
										ft_error("Error: duplicate\n");
									loc.index.clear();
									std::vector<std::vector<std::string> >::iterator itv = con->indexs.begin();
									std::vector<std::string> tmp = *itv;
									loc.index.insert(loc.index.begin(), tmp.begin(), tmp.end());
									for (size_t i = 0; i < loc.index.size(); i++){
										it++;
									}
									con->indexs.erase(itv);
									ind = 1;
								}
								else if (*it == "autoindex"){
									if (auind)
										ft_error("Error: duplicate\n");
									it++;
									if (*it == "on")
										loc.auto_index = true;
									else if (*it == "off")
										loc.auto_index = false;
									else
										ft_error("Error\n");
									auind = 1;
								}
								else if (*it == "root"){
									if (root)
										ft_error("Error: duplicate\n");
									it++;
									loc.root = *it;
									root = 1;
								}
								else if (*it == "upload_pass"){
									if (up)
										ft_error("Error: duplicate\n");
									it++;
									loc.upload_pass = *it;
									up = 1;
								}
								else if (*it == "return"){
									if (ret)
										ft_error("Error :duplicate\n");
									it++;
									loc._return = *it;
									ret = 1;
								}
								else if (*it == "cgi_pass"){
									it++;
									std::string tmp = *it;
									it++;
									loc.cgi_pass.insert(std::make_pair(tmp, *it));
								}
								else
									ft_error("Error\n");
								it++;
							}
						}
						else
							ft_error("Error\n");
						serv.loc.push_back(loc);
					}
					else
						ft_error("Error\n");
					it++;
				}
			}
			else
				ft_error("Error\n");
			it++;
		}
		else
			ft_error("Error\n");
		if (!mbs || serv.error_page.empty() || !sloc)
			ft_error("Error: missing role\n");
		con->_serverconf.push_back(serv);
		sc--;
	}
	if (it != ite)
		ft_error("Error\n");
}

int		find_server(std::vector<std::string> vec){
	int	n=0;
	std::vector<std::string>::iterator it = vec.begin();
	std::vector<std::string>::iterator ite = vec.end();
	while (it != ite){
		it = std::find(it, ite, "server");
		if (it != ite){
			it++;
			if (*it == "{")
				n++;
		}
	}
	return (n);
}

void    config::ft_openf(char *av, config *con){
	std::vector<std::string> vec;
	std::fstream confile;
	if (!av)
		confile.open("configFile/webserv.conf", std::fstream::in);
	else {
		ft_check_file(av);
		confile.open(av, std::fstream::in);
	}
    if (!confile)
        ft_error("file not found!\n");
	std::string str;
	std::string string;
	int sc=0;int obrak=0;int cbrak=0;int stat=0;
	while (std::getline(confile, string)){
		if (string[0] == 0)
			continue;
		std::stringstream s(string);
		s >> str;
		if (str == "{")
			obrak++;
		if (str == "}")
			cbrak++;
		if (str == "index"){
			stat=1;
			vec.push_back(str);
			std::vector<std::string> vecs;
			while (s >> str){
				if (str == "{")
					obrak++;
				if (str == "}")
					cbrak++;
				vecs.push_back(str);
				vec.push_back(str);
			}
			con->indexs.push_back(vecs);
		}
		if (stat == 0)
			vec.push_back(str);
		stat = 0;
		while (s >> str){
			if (str == "{")
				obrak++;
			if (str == "}")
				cbrak++;
			vec.push_back(str);
		}
	}
	sc = find_server(vec);
	if (obrak != cbrak || !sc)
		ft_error("Error\n");
	con->server_num = sc;
	con->error_check(vec, con, sc);
	std::list<serverconf>::iterator it = _serverconf.begin();
	std::vector<std::string> v;
	while (it != _serverconf.end()){
		v.push_back((*it).port);
		it++;
	}
	std::sort(v.begin(), v.end());
	std::vector<std::string>::iterator vit = std::adjacent_find(v.begin(), v.end());
	if (vit != v.end())
		ft_error("Error: Double Port\n");
}
