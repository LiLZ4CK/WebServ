#include "server.hpp"

void default_errors(Client *client, Server *c, int error)
{
	std::string buffer;
	if (error == 301)
	{
		buffer = "HTTP/1.1 301 Moved Permanently\r\n"
				 "Connection: close\r\n"
				 "Content-Length: 11\r\n\r\nMoved Permanently";
	}
	else if (error == 400)
	{
		buffer = "HTTP/1.1 400 Bad Request\r\n"
							"Connection: close\r\n"
							"Content-Type: text/plain\r\n"
							"Content-Length: 11\r\n\r\nBad Request";
	}
	else if (error == 403)
	{
		buffer = "HTTP/1.1 403 Forbidden\r\n"
						"Connection: close\r\n"
						"Content-Length: 63\r\n\r\nThe server understood the request, but refuses to authorize it.";
	}
	else if (error == 404)
	{
		buffer = "HTTP/1.1 404 Not Found\r\n"
						"Connection: close\r\n"
						"Content-Length: 9\r\n\r\nNot Found";
	}
	else if (error == 405)
	{
		buffer = "HTTP/1.1 405 Method Not Allowed\r\n"
						"Connection: close\r\n"
						"Content-Length: 60\r\n\r\nThe requested HTTP method is not supported by this resource.";
	}
	else if (error == 409)
	{
		buffer = "HTTP/1.1 409 Conflict\r\n"
				 "Connection: close\r\n"
				 "Content-Length: 8\r\n\r\nConflict";
	}
	else if (error == 413)
	{
		buffer = "HTTP/1.1 413 Payload Too Large\r\n"
				 "Connection: close\r\n"
				 "Content-Length: 17\r\n\r\nPayload Too Large";
	}
	else if (error == 414)
	{
		buffer = "HTTP/1.1 414 URI Too Long\r\n"
				 "Connection: close\r\n"
				 "Content-Length: 12\r\n\r\nURI Too Long";
	}
	else if (error == 415)
	{
		buffer = "HTTP/1.1 415 Unsupported Media Type\r\n"
				 "Connection: close\r\n"
				 "Content-Length: 22\r\n\r\nUnsupported Media Type";
	}
	else if (error == 500)
	{
		buffer = "HTTP/1.1 500 Internal Server Error\r\n"
				 "Connection: close\r\n"
				 "Content-Length: 21\r\n\r\nInternal Server Error";
	}
	else if (error == 501)
	{
		buffer = "HTTP/1.1 501 Not Implemented\r\n"
				 "Connection: close\r\n"
				 "Content-Length: 15\r\n\r\nNot Implemented";
	}
	if (send(client->socket, buffer.c_str(), buffer.size(), 0) < 0){
		perror("Send :");
	}
	c->drop_client(client, c);
}

int  error_page(Client *client, int error, Server *c)
{
	std::string full_path;
	if (!c->_server.error_page[error].empty())
	{
		full_path = c->_server.error_page[error];
		client->file.open(full_path.c_str(), std::ios::binary);
		if (!client->file)
		{
			default_errors(client, c, error);
			return 1;
		}
		content_lenght_type(client, full_path);
		response_header(client, error, c, "");
	}
	else
	{
		default_errors(client, c, error);
		return 1;
	}

	return (0);
}

int	auto_indexing(Client *client, Server *c){
	std::string tmp;
	tmp = client->full_path;
	tmp.erase(0, tmp.rfind('/'));
	size_t n = tmp.find('.');
	if (n == std::string::npos){
		if (!client->_location.auto_index){
			client->full_path += '/';
			for (size_t i = 0; i < client->_location.index.size(); i++){
				client->full_path += client->_location.index[i];
				client->file.open(client->full_path.c_str(), std::ios::binary);
				if (!client->file){
					client->full_path.erase(client->full_path.rfind('/'));
					client->full_path += '/';
					continue;
				}
				else
					break;
			}
			if (!client->file){
				error_page(client, 404, c);
				return 1;
			}
		}
		else {
			struct stat st;
			if (stat(client->full_path.c_str(), &st) != -1) {
				if (S_ISDIR(st.st_mode)) {
					if (generate_autoindex_page(client, c, client->full_path))
						return 1;
					client->ct = "text/html";
					client->cl = client->auto_index.size();
					response_header(client, 200, c, client->auto_index);
					return 1;
				}
				else {
					error_page(client, 403, c);
					return 1;
				}
			}
		}
	}
	return 0;
}

int check_file(Client *client, std::string path, Server *c)
{
	std::string tmp;
	std::list<location>::iterator it = c->_server.loc.begin();
	tmp = path;
	while (!tmp.empty()){
		it = c->_server.loc.begin();
		while (it != c->_server.loc.end()){
			if (!(*it)._return.empty())
			{
				response_header(client, 301, c, (*it)._return);
				return 1;
			}
			else if (tmp == (*it).path){
				path.erase(0, tmp.size());
				if (path[0] != '/' && (*it).root[(*it).root.size()-1] != '/')
					client->full_path = (*it).root + '/' +  path;
				else
					client->full_path = (*it).root + path;
				break;
			}
			it++;
		}
		if (!client->full_path.empty())
			break;
		int i = tmp.rfind('/');
		if (i == 0 && tmp.size() > 1)
			tmp = '/';
		else
			tmp.erase(i);
	}
	if (client->full_path.empty()){
		error_page(client, 404, c);
		return 1;
	}
	size_t n = client->full_path.find('?');
	if (n != std::string::npos){
		client->query_str = client->full_path.substr(n+1, client->full_path.size());
		client->full_path.erase(n, client->full_path.size());
	}
	struct stat res;
	if (stat(client->full_path.c_str(), &res) != 0){
		error_page(client, 404, c);
		return 1;
	}
	client->_location = *it;
	if (std::find(client->_location.allow_methods.begin(), client->_location.allow_methods.end(), client->_request.method) == client->_location.allow_methods.end()){
		error_page(client, 405, c);
		return 1;
	}
	return (0);
}

int content_lenght_type(Client *client, std::string full_path)
{
	client->file.close();
	client->file.open(full_path.c_str(), std::ios::binary);
	std::streampos begin = client->file.tellg();
	client->file.seekg(0, std::ios::end);
	std::streampos end = client->file.tellg();
	client->cl = static_cast<size_t>(end - begin);
	client->file.seekg(0, std::ios::beg);
	client->ct = get_content_type(full_path);
	return (0);
}

int generate_autoindex_page(Client *client, Server *c, std::string& full_path)
{
    std::ostringstream oss;

    oss << "<html>\n";
    oss << "<head>\n";
    oss << "<title>Index of " << full_path << "</title>\n";
    oss << "</head>\n";
    oss << "<body>\n";
    oss << "<h1>Index of " << full_path << "</h1>\n";
    oss << "<hr>\n";
    oss << "<pre>\n";

    DIR* dirp = opendir(full_path.c_str());
    if (dirp == NULL) {
        error_page(client, 403, c);
        return 1;
    }

    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
			std::string filename(dp->d_name);
			struct stat stat_buf;
			std::string stat_path;

			stat_path = full_path + filename;
            if (stat(stat_path.c_str(), &stat_buf) == 0) {
                if (S_ISDIR(stat_buf.st_mode)) {
                    filename += '/';
                }
            }
			oss << "<a href=\"" << filename << "\">" << filename << "</a>\n";
    }
    closedir(dirp);
    oss << "</pre>\n";
    oss << "<hr>\n";
    oss << "</body>\n";
    oss << "</html>\n";
	client->auto_index = oss.str();
    return 0;
}

int response_header(Client *client, int status, Server *c, std::string string)
{
	(void)c;
	std::string buffer;
    if (client->resp_status == 0)
    {
		if (status == 204)
		{
			buffer = "HTTP/1.1 204 No Content\r\nConnection: close\r\nContent-Length: 7\r\n\r\nDeleted";
		}
		else if (status == 301)
		{
			buffer = "HTTP/1.1 301 Moved Permanently\r\n"
					+ std::string("Location: ")
					+ string
					+ "\r\n\r\n";
		}
		else if (status == 201)
		{
			buffer = "HTTP/1.1 201 Created\r\nConnection: close\r\n\r\n";
		}
		else if (status == 200)
		{
			buffer = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: "
							+ client->ct
							+ "\r\nContent-Length: "
							+ std::to_string(client->cl)
							+ "\r\nSet-Cookie: "
							// + std::to_string(client->cookieID);
							// + ";"
							+ client->query_str
							+ "\r\n\r\n"
							+ string;
		}
		else if (status == 301)
		{
			buffer = "HTTP/1.1 301 Moved Permanently\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 404)
		{
			buffer = "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 403)
		{
			buffer = "HTTP/1.1 403 Forbidden\r\nConnection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 400)
		{
			buffer = "HTTP/1.1 400 Bad Request\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 405)
		{
			buffer = "HTTP/1.1 405 Method Not Allowed\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 409)
		{
			buffer = "HTTP/1.1 409 Conflict\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 413)
		{
			buffer = "HTTP/1.1 413 Payload Too Large\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 414)
		{
			buffer = "HTTP/1.1 414 URI Too Long\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 415)
		{
			buffer = "HTTP/1.1 415 Unsupported Media Type\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 500)
		{
			buffer = "HTTP/1.1 500 Internal Server Error\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 501)
		{
			buffer = "HTTP/1.1 501 Not Implemented\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		else if (status == 502)
		{
			buffer = "HTTP/1.1 502 Bad Gateway\r\n"
						"Connection: close\r\nContent-Type: "
						+ client->ct
						+"\r\nContent-Length: "
						+std::to_string(client->cl)
						+"\r\n\r\n";
		}
		if (send(client->socket, buffer.c_str(), buffer.size(), 0) < 0){
			perror("Send :");
			c->drop_client(client, c);
			return 1;
		}
		client->resp_status = 1;
    }
    return 0;
}

int response_file(Client *client, Server *c)
{
    char str[BSIZE+1];
    client->file.read(str, BSIZE);
	str[client->file.gcount()] = '\0';
    if (send(client->socket, str, client->file.gcount(), 0) < 0){
		perror("Send :");
		c->drop_client(client, c);
		return 1;
	}
    if (client->file.eof() || !client->file){
        client->reset();
        client->file.close();
        c->drop_client(client, c);
		std::remove(client->cgi_file.c_str());
		return 1;
    }
	return 0;
}

int request_routing(Client *client, Server *c)
{
	if (client->_request.method == "GET"){
		if(!client->_location.cgi_pass.empty())
		{
			int k = correct_cgi(client, c, "GET");
			if (k == 1)
				return 0;
			else if (k == 0)
				return 1;
		}
		if (auto_indexing(client, c) == 1)
			return 1;
		client->file.close();
		client->file.open(client->full_path.c_str(), std::ios::binary);
		if (!client->file) {
			error_page(client, 404, c);
			return 1;
		}
		content_lenght_type(client, client->full_path);
		response_header(client, 200, c, "");
	}
	else if (client->_request.method == "POST"){
		if (post_response(client, c))
			return 1;
	}
	else if (client->_request.method == "DELETE"){
	    if (delete_method(client, c))
			return 1;
	}
    return 0;
}