#include "server.hpp"

int post_response(Client *client, Server *c){
	if (client->_request.stat == 0){
		std::map<std::string, std::string> types;
		types["text/css"] = ".css";
		types["text/csv"] = ".csv";
		types["image/gif"] = ".gif";
		types["text/html"] = ".htm";
		types["text/html"] = ".html";
		types["image/x-icon"] = ".ico";
		types["image/jpeg"] = ".jpeg";
		types["image/jpeg"] = ".jpg";
		types["audio/mpeg"] = ".mp3";
		types["video/mp4"] = ".mp4";
		types["application/javascript"] = ".js";
		types["application/json"] = ".json";
		types["image/png"] = ".png";
		types["application/pdf"] = ".pdf";
		types["image/svg+xml"] = ".svg";
		types["text/plain"] = ".txt";
		std::string type = client->_request.reqm["Content-Type:"];
		if (!type.empty())
			type.erase(type.find(" "), type.size());
		srand(time(NULL));
		std::stringstream ss;
		ss << rand();
		std::getline(ss, client->_request.ljasad);
		client->_request.ljasad = "upload_" + client->_request.ljasad;
		type = types[type];
		client->_request.ljasad += type;
		std::ofstream file;
		if (!client->_location.upload_pass.empty()){
			std::string tmp = client->_location.upload_pass + '/' + client->_request.ljasad;
			file.open(tmp, std::ios::app);
			if (!file){
				client->_request.ljasad = "./public/uploads/" + client->_request.ljasad;
				file.open(client->_request.ljasad, std::ios::app);
				if (!file){
					error_page(client, 500, c);
					client->_request.stat = 1;
					return 1;
				}
			}
			else
				client->_request.ljasad = tmp;
		}
		else if (!client->_location.cgi_pass.empty()){
			if (access("./private/", O_RDWR) == -1){
				default_errors(client, c, 500);
				client->_request.stat = 1;
				return 1;
			}
			client->_request.ljasad = "./private/" + client->_request.ljasad;
			file.open(client->_request.ljasad, std::ios::app);
			if (!file){
				default_errors(client, c, 500);
				client->_request.stat = 1;
				return 1;
			}
		}
		else{
			default_errors(client, c, 403);
			client->_request.stat = 1;
			return 1;
		}
		if (!client->_request.reqm["Content-Length:"].empty()){
			client->_request.contentl = atoi(client->_request.reqm["Content-Length:"].c_str());
			client->_request.chunk = 0;
		}
		else
			client->_request.contentl = -1;
		if (!client->_request.reqm["Transfer-Encoding:"].empty()){
			client->_request.chunk_stat = true;
			type = client->_request.body.substr(0, client->_request.body.find("\r\n"));
			client->_request.body.erase(0, client->_request.body.find("\r\n")+2);
			client->_request.chunk = strtol(type.c_str(), nullptr, 16);
			if (client->_request.chunk == 0){
				file.close();
				client->received = 0;
				if (auto_indexing(client, c) == 1)
					return 1;
				client->file.close();
				client->file.open(client->full_path.c_str(), std::ios::binary);
				if (!client->file) {
					if (error_page(client, 404, c) == 1)
						return 1;
				}
				content_lenght_type(client, client->full_path);
				response_header(client, 201, c, "");
				return 0;
			}
			client->_request.chunk -= client->_request.body.size();
		}
		file.write(client->_request.body.c_str(), client->_request.body.size());
		client->received = client->_request.body.size();
		client->_request.stat = 1;
		if (client->_request.contentl <= client->received && !client->_request.chunk_stat){
			if (client->_location.upload_pass.empty() && !client->_location.cgi_pass.empty()){
				if (correct_cgi(client, c, "POST") == 1)
					return 0;
				return 1;
			}
			else {
				file.close();
				client->received = 0;
				if (auto_indexing(client, c) == 1)
					return 1;
				client->file.close();
				client->file.open(client->full_path.c_str(), std::ios::binary);
				if (!client->file) {
					if (error_page(client, 404, c) == 1)
						return 1;
				}
				content_lenght_type(client, client->full_path);
				response_header(client, 201, c, "");
			}
		}
	}
	else {
		std::fstream file;
		if (!client->_request.chunk_stat){
			if (client->_request.contentl <= client->received){
				file.open(client->_request.ljasad, std::ios::app);
				if (!file){
					error_page(client, 500, c);
					return 1;
				}
				file.write(client->request, client->recv);
			    file.close();
			}
			else {
				file.open(client->_request.ljasad, std::ios::app);
				if (!file){
					error_page(client, 500, c);
					return 1;
				}
				file.write(client->request, client->recv);
			}
		}
		else {
			std::string tmp;
			tmp.append(client->request, client->recv);
			file.open(client->_request.ljasad, std::ios::app);
			if (!file){
				error_page(client, 500, c);
				return 1;
			}
			if (client->_request.chunk == -1){
				size_t i = tmp.find("\r\n");
				if (i == 0){
					if (client->_request.type.size() != 0)
						tmp.erase(0, 2);
					else{
						tmp.erase(0, 2);
						i = tmp.find("\r\n");
						client->_request.type += tmp.substr(0, i);
						tmp.erase(0, i+2);
					}
				}
				else {
					client->_request.type += tmp.substr(0, i);
					tmp.erase(0, i+2);
				}
				client->_request.chunk = strtol(client->_request.type.c_str(), nullptr, 16);
			}
			if (client->_request.chunk > 0){
				if ((size_t)client->_request.chunk > tmp.size()){
					file.write(tmp.c_str(), tmp.size());
					client->_request.chunk -= tmp.size();
				}
				else {
					file.write(tmp.c_str(), client->_request.chunk);
					tmp.erase(0, client->_request.chunk+2);
					client->_request.chunk = 0;
					if (tmp.size() == 0){
						client->_request.chunk = -1;
						client->_request.type = "";
						return 0;
					}
					if (tmp.find("\r\n") != std::string::npos){
						client->_request.type = tmp.substr(0, tmp.find("\r\n"));
        				tmp.erase(0, client->_request.type.size()+2);
        				client->_request.chunk = strtol(client->_request.type.c_str(), nullptr, 16);
						if ((size_t)client->_request.chunk >= tmp.size()){
							file.write(tmp.c_str(), tmp.size());
							client->_request.chunk -= tmp.size();
						}
						else {
							file.write(tmp.c_str(), client->_request.chunk);
						}
					}
					else {
						client->_request.chunk = -1;
						client->_request.type = tmp;
					}
				}
			}
    	}
		if (client->recv != MAX_REQUEST_SIZE && client->_request.contentl <= client->received && client->_request.chunk == 0){
			if (client->_location.upload_pass.empty() && !client->_location.cgi_pass.empty()){
				if (correct_cgi(client, c, "POST") == 1)
					return 0;
				return 1;
			}
			else {
				file.close();
				client->received = 0;
				if (auto_indexing(client, c) == 1)
					return 1;
				client->file.close();
				client->file.open(client->full_path.c_str(), std::ios::binary);
				if (!client->file) {
					if (error_page(client, 404, c) == 1)
						return 1;
				}
				content_lenght_type(client, client->full_path);
				response_header(client, 201, c, "");
			}
		}
	}
	return 0;
}
