#include "fdstreambuf.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <list>
#include <thread>
#include "messages.h"

Hello m_hi;
Put   m_put;
Get   m_get;
Bye   m_bye;

static std::list<std::iostream *> streamList;

static void wall(Message &m, ostream *s)
{
    // for (auto s : streamList)
    {
        if (m >> m_hi)
        {
            (*s) << m_hi; std::cout << "Sending: " << Message::Json(m_hi) << std::endl;
        }
        else
        if (m >> m_put)
        {
            m_get.data = "recv some data";

            (*s) << m_get; std::cout << "Sending: " << Message::Json(m_get) << std::endl;
        }
        else
        if (m >> m_get)
        {
            m_put.data = "sent some data";

            (*s) << m_put; std::cout << "Sending: " << Message::Json(m_put) << std::endl;
        }
        else
        if (m >> m_bye)
        {
            (*s) << m_bye; std::cout << "Sending: " << Message::Json(m_bye) << std::endl;
        }
    }
}

static void handleClient(int fd, std::string remote)
{
    wl::fdstreambuf sbuf(fd);
    std::iostream io(&sbuf);
    streamList.push_back(&io);

    Message m;

    while (io >> m)
    {
        std::cout << "Received: ";

        if (m >> m_hi)
        {
            std::cout << Message::Json(m_hi);
        }
        else
        if (m >> m_put)
        {
            std::cout << Message::Json(m_put);
        }
        else
        if (m >> m_get)
        {
            std::cout << Message::Json(m_get);
        }
        else
        if (m >> m_bye)
        {
            std::cout << Message::Json(m_bye);
        }
        else
        {
            std::cout << "Unknown message: " << Message::Json(m);
        }

        std::cout << std::endl;

        wall(m, &io);

        if (m == m_bye)
        {
            break;
        }
    }

    streamList.remove(&io);
    close(fd);
    std::cout << "Disconnect from " << remote << std::endl;
}

int main(int argc, char *argv[])
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in me;

    std::memset(&me, 0, sizeof(me) );
    me.sin_family = PF_INET;
    me.sin_port = htons(9876);
    me.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&me), sizeof(me)))
    {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, 1))
    {
        perror("listen");
        exit(1);
    }

    while (true)
    {
        struct sockaddr_in dst;
        socklen_t size = sizeof(dst);
        int cfd = accept(sockfd, reinterpret_cast<struct sockaddr *>(&dst), &size);
        std::string remote(inet_ntoa(dst.sin_addr) );
        std::cout << "Connect from " << remote << std::endl;
        auto th = std::thread(handleClient, cfd, remote);
        th.detach();
    }

    return(0);
}
