#include "fdstreambuf.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include "messages.h"

Hello m_hi;
Put   m_put;
Get   m_get;
Bye   m_bye;

static void handleServer(std::iostream *io)
{
    Message m;

    while ((*io) >> m)
    {
        if (m >> m_hi)
        {
            std::cout << "Received: " << m_hi.json() << std::endl;
        }
        else
        if (m >> m_put)
        {
            std::cout << "Received: " << m_put.json() << std::endl;
        }
        else
        if (m >> m_get)
        {
            std::cout << "Received: " << m_get.json() << std::endl;
        }
        else
        if (m >> m_bye)
        {
            std::cout << "Received: " << m_bye.json() << std::endl;
            break;
        }
        else
        {
            std::cout << "Unknown message: " << m.json() << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: client server_host" << std::endl;
        exit(1);
    }

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dst;
    std::memset(&dst, 0, sizeof(dst) );
    dst.sin_family = PF_INET;
    dst.sin_port = htons(9876);
    struct hostent *hp;
    hp = gethostbyname(argv[1]);
    std::memcpy(&dst.sin_addr, hp->h_addr, hp->h_length);
    connect(sockfd, reinterpret_cast<struct sockaddr *>(&dst), sizeof(dst) );
    wl::fdstreambuf sbuf(sockfd);
    std::iostream io(&sbuf);
    auto th1 = std::thread(handleServer, &io);

    srandom(time(NULL));

    m_hi.my_id = m_put.my_id = m_get.my_id = m_bye.my_id = random();

    io << m_hi; std::cout << "Sending: " << m_hi.json() << std::endl;

    while (!io.fail())
    {
        std::string data;

        std::cout << "Type a command (bye|put|get): ";

        std::getline(std::cin, data);

        if (std::cin.fail() || data == "bye")
        {
            break;
        }

        if (data == "put")
        {
            std::cout << "Enter text to send: ";

            std::getline(std::cin, data);

            m_put.data = data;

            io << m_put; std::cout << "Sending: " << m_put.json() << std::endl;
        }

        if (data == "get")
        {
            std::cout << "Enter question: ";

            std::getline(std::cin, data);

            m_get.data = data;

            io << m_get; std::cout << "Sending: " << m_get.json() << std::endl;
        }
    }

    io << m_bye; std::cout << "Sending: " << m_bye.json() << std::endl;

    th1.join();

    std::cout << "Finish" << std::endl;

    close(sockfd);

    return(0);
}
