#include "server.hpp"

std::string get_content_type(const std::string& path) {
    size_t lastDotIndex = path.rfind('.');
    if (lastDotIndex != std::string::npos) {
        std::string extension = path.substr(lastDotIndex);
        std::map<std::string, std::string> mimeTypes;
        mimeTypes[".css"] = "text/css";
        mimeTypes[".csv"] = "text/csv";
        mimeTypes[".gif"] = "image/gif";
        mimeTypes[".htm"] = "text/html";
        mimeTypes[".html"] = "text/html";
        mimeTypes[".ico"] = "image/x-icon";
        mimeTypes[".jpeg"] = "image/jpeg";
        mimeTypes[".jpg"] = "image/jpeg";
        mimeTypes[".mp4"] = "video/mp4";
        mimeTypes[".js"] = "application/javascript";
        mimeTypes[".json"] = "application/json";
        mimeTypes[".png"] = "image/png";
        mimeTypes[".pdf"] = "application/pdf";
        mimeTypes[".svg"] = "image/svg+xml";
        mimeTypes[".txt"] = "text/plain";
        mimeTypes[".cpp"] = "text/x-c";
        mimeTypes[".c"] = "text/x-c";
        mimeTypes[".hpp"] = "text/x-c";

        std::map<std::string, std::string>::iterator it = mimeTypes.find(extension);
        if (it != mimeTypes.end()) {
            return (*it).second;
        }
    }
    return "application/octet-stream";
}

SOCKET create_socket(const char *host, const char *port)
{
    struct addrinfo hints;
    memset (&hints, 0 ,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    if (getaddrinfo( host, port, &hints, &bind_address)){
        perror("getaddrinfo :");
        return (-1);
    }
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    fcntl(socket_listen, F_SETFL, O_NONBLOCK);
    int optval = 1;
    setsockopt(socket_listen, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (!ISVALIDSOCKET(socket_listen))
    {
        perror("Socket :");
        return (-1);
    }
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        perror("Bind :");
        return (-1);
    }
    freeaddrinfo(bind_address);
    if (listen(socket_listen, 1000) < 0)
    {
        perror("Listen :");
        return (-1);
    }
    return socket_listen;
}
