#include "../serverSide/server.hpp"

int allowed_chars(std::string uri)
{
	int i = 0;
	while (uri[i]){
		if(isalnum(uri[i]))
			i++;
		else if(uri[i] == '-' || uri[i] == '.' || uri[i] == '_' || uri[i] == '~'
			|| uri[i] == ':' || uri[i] == '/' || uri[i] == '?' || uri[i] == '#'
			|| uri[i] == '[' || uri[i] == ']' || uri[i] == '@' || uri[i] == '!'
			|| uri[i] == '$' || uri[i] == '&' || uri[i] == '\'' || uri[i] == '('
			|| uri[i] == ')' || uri[i] == '*' || uri[i] == '+' || uri[i] == ','
			|| uri[i] == ';' || uri[i] == '=' || uri[i] == '%')
			i++;
		else
			return 1;
	}
	return 0;
}

int    request_parse(Client *client, Server *server){
    client->_request.body.append(client->request, client->recv);
    size_t i = client->_request.body.find("\r\n\r\n");
    if (i == std::string::npos){
        error_page(client, 414, server);
        return 1;
    }
    client->_request.body.erase(0, i+4);
    std::string str;
    str.append(client->request, client->recv);
    int nb = str.size() - client->_request.body.size();
    str.erase(nb);
    std::stringstream s(str);
    std::vector<std::string> vec;
    std::string s1;
    while (s >> s1)
        vec.push_back(s1);
    std::vector<std::string>::iterator it = vec.begin();
    client->_request.method = *it++;
    if (client->_request.method != "GET" && client->_request.method != "POST" && client->_request.method != "DELETE"){
        error_page(client, 400, server);
		return 1;
    }
    client->_request.path1 = *it++;
    if (allowed_chars(client->_request.path1)){
		error_page(client, 400, server);
        return 1;
    }
    if (client->_request.path1.size() >= 1048){
        error_page(client, 414, server);
        return 1;
    }
    client->_request.path2 = *it++;
    std::string s2;
    std::string s3;
    std::string tmp;
    while (it != vec.end()){
        s2 = *it;
        it++;
        while (it != vec.end()){
            tmp = *it;
            if (tmp[tmp.size()-1] == ':'){
                break;
            }
            s3 += tmp;
            s3 += " ";
            it++;
        }
        client->_request.reqm[s2] = s3;
        s3 = "";
    }
    tmp = client->_request.reqm["Transfer-Encoding:"];
    if (!tmp.empty()){
        if (tmp != "chunked "){
            error_page(client, 501, server);
            return 1;
        }
    }
    if (tmp.empty() && client->_request.reqm["Content-Length:"].empty() && client->_request.method == "POST"){
        error_page(client, 400, server);
        return 1;
    }
    if (!client->_request.reqm["Content-Length:"].empty()){
        int n = atoi(client->_request.reqm["Content-Length:"].c_str());
        if (n > server->_server.max_client_body_size){
            error_page(client, 413, server);
            return 1;
        }
    }
    return 0;
}
