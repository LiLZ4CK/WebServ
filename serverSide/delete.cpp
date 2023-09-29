#include "server.hpp"
#include <cstdio>
#include <dirent.h>

bool compareByPathLength(const location& lhs, const location& rhs) {
	return lhs.path.length() > rhs.path.length();
}

int is_direc(std::string local, Client *client, Server *c)
{
	std::string loc;
	int result = access(local.c_str(), O_RDWR);
	if (result == -1)
	{
		client->delete_status = 403;
		return 1;
	}
	DIR* directory = opendir(local.c_str());
	if(directory == NULL)
	{
		client->delete_status = 403;
		closedir(directory);
		return 1;
	}
	else{
		struct dirent* file = NULL;
		while ((file = readdir(directory)) != NULL)
		{
			std::string a = file->d_name;
			if (a == "." || a == "..")
				continue;
			std::string path = local + "/" + file->d_name;
			struct stat res;
			stat(path.c_str(), &res);
			if (S_ISREG(res.st_mode))
			{
				int result = access(path.c_str(), W_OK);
				if (result == -1)
				{
					client->delete_status = 409;
					closedir(directory);
					return 1;
				}
				std::remove(path.c_str());
				std::fstream filee(path.c_str());
				if (!filee)
				{
					client->delete_status = 204;
					continue;
				}
				else
				{
					filee.close();
					if (result == 0)
					{
						client->delete_status = 500;
						closedir(directory);
						return 1;
					}
					else {
						client->delete_status = 413;
						closedir(directory);
						return 1;
					}
				}
			}
			else if (S_ISDIR(res.st_mode))
			{
				is_direc(path.c_str(), client, c);
			}
		}
		
	}
	result = access(local.c_str(), W_OK);
	if (result == -1)
	{
		client->delete_status = 403;
		closedir(directory);
		return 1;
	}
	else{
		closedir(directory);
		rmdir(local.c_str());
		client->delete_status = 204;
		return (1);
	}
	closedir(directory);
	client->delete_status = 204;
	return 1;
}

int delete_method(Client *client, Server *c)
{
	std::string local = client->full_path;
	struct stat res;
	if (stat(local.c_str(), &res) == 0)
	{
		if (S_ISREG(res.st_mode))
		{
			int result = access(local.c_str(), W_OK);
			if (result == -1)
			{
				error_page(client, 403, c);
				return 1;
			}
			std::remove(local.c_str());
			std::fstream file(local.c_str());
			if (!file)
			{
				response_header(client, 204, c, "");
				return 1;
			}
			else
			{
				file.close();
				if (result == 0)
				{
					response_header(client, 500, c, "");
					return 1;
				}
				else
					response_header(client, 413, c, "");
					return 1;
			}
		}
		else if (S_ISDIR(res.st_mode))
		{
			if (local[local.size()-1] != '/'){
				error_page(client, 400, c);
				return 1;
			}
			if (is_direc(local, client, c)){
				if (client->delete_status == 204)
					response_header(client, client->delete_status, c, "");
				else
					error_page(client, client->delete_status, c);
			}
		}
	}
	return 0;
}
