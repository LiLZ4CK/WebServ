/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_handler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zwalad <zwalad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/03 18:14:08 by zwalad            #+#    #+#             */
/*   Updated: 2023/04/13 00:27:46 by zwalad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <signal.h>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <unistd.h>

char *const *setup_env(Client *client)
{
	std::string path1 = client->_request.path1;
	std::string fullp = client->full_path;
    std::string map_env[8];
	std::string s1 = client->_request.reqm["Content-Length:"];
	if (!s1.empty())
		s1.erase(s1.size()-1, s1.size());
	std::string s2 =  client->_request.reqm["Content-Type:"];
	if (!s2.empty())
		s2.erase(s2.size()-1, s2.size());
	std::string s3 =  client->_request.reqm["Cookie:"];
	if (!s3.empty())
		s3.erase(s3.size()-1, s3.size());
    map_env[0] = "REQUEST_METHOD=" + client->_request.method;
    map_env[1] = "CONTENT_LENGTH=" + s1;
    map_env[2] = "SCRIPT_FILENAME=" + fullp;
    map_env[3] = "REDIRECT_STATUS=200";
    map_env[4] = "CONTENT_TYPE="+s2;
    map_env[5] = "PATH_INFO=" + fullp;
    map_env[6] = "HTTP_COOKIE=" + s3;
    map_env[7] = "QUERY_STRING=" + client->query_str;
    char **envp = new char *[8 + 1];
    for (int i = 0; i < 8; i++) {
        char *env_var = new char[map_env[i].length() + 1];
        strcpy(env_var, map_env[i].c_str());
        envp[i] = env_var;
    }
    envp[8] = 0;
    return (envp);
}

int if_cgi(Client *client, Server *c, std::string cgi)
{
	if (client->full_path.find(cgi) != std::string::npos)
	{
		std::fstream file(client->full_path.c_str());
		if (!file)
		{
			error_page(client, 404, c);
			return 0;
		}
		else{
			return 1;
		}
	}
	return 2;
}

int correct_cgi(Client *client, Server *c, std::string meth)
{
	int k;
	if (if_cgi(client, c, ".php") == 1)
	{
		if (client->_location.cgi_pass["php"].empty()){
			error_page(client, 500, c);
			return 0;
		}
		std::fstream f(client->_location.cgi_pass["php"].c_str());
		if(!f)
		{
			error_page(client, 500, c);
			return 0;
		}
		if (meth == "GET"){
			if (get_cgi(client, "php"))
				return 1;
		}
		else if(meth == "POST"){
			if(post_cgi(client, "php"))
				return 1;
		}
	}
	k = if_cgi(client, c, ".perl");
	if (k == 2)
		return k;
	else if (k == 1)
	{
		if (client->_location.cgi_pass["perl"] != "/usr/bin/perl")
		{
			error_page(client, 500, c);
			return 0;
		}
		if (meth == "GET"){
			if (get_cgi(client, "perl"))
				return 1;
		}
		else if(meth == "POST"){
			if(post_cgi(client, "perl"))
				return 1;
		}
	}
	return 0;
}

int post_cgi(Client *client, std::string cgi)
{
	char *const *env = setup_env(client);
	if (client->cgi_status == 0)
	{
		char *_argv[3];
		_argv[0] = strdup(client->_location.cgi_pass[cgi].c_str());
		_argv[1] = strdup((client->full_path.c_str()));
		_argv[2] = NULL;
		srand(time(NULL));
		std::stringstream ss;
		ss << rand();
		std::getline(ss, client->cgi_file);
		int fd1 = open(client->cgi_file.c_str(), O_CREAT | O_RDWR, 0777);
		client->pid = fork();
		if (client->pid == 0)
		{
			int fd0 = open(client->_request.ljasad.c_str(), O_RDONLY);
			dup2(fd1, 1);
			dup2(fd0, 0);
			execve(_argv[0], _argv, env);
			exit(1);
		}
		close(fd1);
		for (int i = 0; i < 9; i++) {
        	delete[] env[i];
    	}
		delete [] env;
		free(_argv[0]);
		free(_argv[1]);
		client->cgi_status = 1;
		client->resp_status = 1;
	}
	return 1;
}

int get_cgi(Client *client, std::string cgi)
{
	if (client->cgi_status == 0){
		char *_argv[3];
		char *const *env = setup_env(client);
		_argv[0] = strdup(client->_location.cgi_pass[cgi].c_str());
		_argv[1] = strdup((client->full_path.c_str()));
		_argv[2] = NULL;
		srand(time(NULL));
		std::stringstream ss;
		ss << rand();
		std::getline(ss, client->cgi_file);
		int fd = open(client->cgi_file.c_str(), O_CREAT | O_RDWR, 0777);
		client->pid = fork();
		if (client->pid == 0)
		{
			dup2(fd, 1);
			execve(_argv[0], _argv, env);
			exit(1);
		}
		dup2(1, fd);
		close(fd);
		for (int i = 0; i < 9; i++) {
        	delete[] env[i];
    	}
		delete[] env;
		free(_argv[0]);
		free(_argv[1]);
		client->cgi_status = 1;
		client->resp_status = 1;
	}
	return 1;
}

std::string toLowerCase(std::string str) {
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        *it = std::tolower(*it);
    }
    return str;
}

int parsee_it(std::string upheader,Client *client, Server *c)
{
	std::string header = toLowerCase(upheader);
	if (header.find("location") != std::string::npos)
	{
		std::string link = header.substr(header.find("location: ") + 10, header.size() - 11);
 		response_header(client, 301, c, link);
		client->chk = 1;
		return 1;
	}
	else if (header.find("content-type") != std::string::npos)
	{
		client->ct = header.substr(header.find("content-type: ") + 14, header.size() - 14);
		client->chk = 1;
	}
	else if (header.find("content-length") != std::string::npos)
	{
		client->cl = std::stoi(header.substr(header.find("content-length: ") + 16, header.size() - 16));
		client->chk = 1;
	}
	else if (header.find("status") != std::string::npos)
	{
		client->res_status = std::stoi(header.substr(header.find("status: ") + 8, header.size() - 8));
		client->chk = 1;
	}
	return 0;
}

int do_cgi(Client *client, Server *c)
{
	int check = waitpid(client->pid, 0, WNOHANG);
	if (check == 0)
		return 2;
	else if (check == -1)
	{
			error_page(client, 500, c);
			std::remove(client->cgi_file.c_str());
			return 1;
	}
	else if (check == client->pid){
		client->resp_status = 0;
		std::string buff;
		std::string bodyy;
		std::fstream f1(client->cgi_file.c_str());
		if (!f1)
		{
			error_page(client, 500, c);
			std::remove(client->cgi_file.c_str());
			return 1;
		}
		while(std::getline(f1, buff))
		{
			bodyy += buff + '\n';
		}
		f1.close();
		std::remove(client->cgi_file.c_str());
		if(bodyy.find("\r\n\r\n") == std::string::npos)
		{
			error_page(client, 502, c);
			return 1;
		}
		std::string header = bodyy;
		header.erase(header.find("\r\n\r\n"), header.size() - header.find("\r\n\r\n"));
		std::stringstream ss;
		bodyy.erase(0, bodyy.find("\r\n\r\n") + 4);
		client->ct = "text/html";
		client->cl = bodyy.size();
		client->res_status = 200;
		client->res_str = "";
		ss << header;
		std::string head;
		client->chk = 0;
		while(std::getline(ss, head))
		{
			if (parsee_it(head, client, c))
				return 1;
		}
		if (client->chk == 0)
		{
			error_page(client, 500, c);
			return 1;
		}
		client->resp_status = 0;
		response_header(client, client->res_status, c, client->res_str);
		std::ofstream fi(client->cgi_file.c_str());
		fi << bodyy;
		client->file.open(client->cgi_file.c_str(), std::ios::out | std::ios::in | std::ios::binary);
		if (!client->file) {
			error_page(client, 500, c);
			return 1;
		}
		client->cgi_status = 0;
	}
	return 0;
}
