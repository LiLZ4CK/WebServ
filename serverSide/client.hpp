#ifndef CLIENT_HPP
#define  CLIENT_HPP

#include <memory>
#include <string.h>
#include <netinet/in.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <list>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include "../parsing/config.hpp"

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define BSIZE 1024
#define MAX_REQUEST_SIZE 2047
class location;

class Request_Client {
    public:
        std::string method;
        std::string path1;
        std::string path2;
        std::map<std::string, std::string> reqm;
        int stat;
        int contentl;
        int chunk;
        bool chunk_stat;
        std::string body;
        std::string ljasad;
        std::string type;
};

class Client {
    public:
        int cookieID;
        int resp_status;
        std::string auto_index;
        socklen_t address_length;
        sockaddr_storage address;
        SOCKET socket;
        size_t cl;
        size_t res_status;
        std::string res_str;
        int delete_status;
        int chk;
        std::string ct;
        char request[MAX_REQUEST_SIZE+1];
        Request_Client _request;
        location _location;
        std::ifstream file;
        std::string full_path;
        int received;
        int cgi_status;
        std::string cgi_file;
        std::string query_str;
        int pid;
        int recv;
        void reset(){
            received = 0;
            resp_status = 0;
        };
};

#endif