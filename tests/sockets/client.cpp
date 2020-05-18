#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "hob/codec/flat.hpp"
#include "hob/codec/json.hpp"
#include "hob/io/handle.hpp"
#include "hob/io/stream.hpp"
#include "hob/std/thread.hpp"
#include "../hobs.h"

Hello m_hi;
Put   m_put;
Get   m_get;
Bye   m_bye;

hobio::ostream std_ostream;
hobio::json::encoder enc_stdout(std_ostream);

static void handleServer(hob::decoder *io)
{
    hob m;

    while ((*io) >> m)
    {
        std::cout << "Received: ";

        if (m >> m_hi)
        {
            m_hi >> enc_stdout;
        }
        else
        if (m >> m_put)
        {
            m_put >> enc_stdout;
        }
        else
        if (m >> m_get)
        {
            m_get >> enc_stdout;
        }
        else
        if (m >> m_bye)
        {
            m_bye >> enc_stdout;
        }
        else
        {
            std::cout << "Unknown HOB: ";

            m >> enc_stdout;
        }

        std::cout << std::endl;

        if (m == m_bye)
        {
            break;
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

    hobio::iohandle io(sockfd);
    hobio::flat::decoder dec(io);
    hobio::flat::encoder enc(io);

    dec << hobio::flat::VARINT;
    enc << hobio::flat::VARINT;

    enc_stdout << hobio::json::VERBOSE;
    enc_stdout << hobio::flat::VARINT;

    std::thread<hob::decoder *> th1 = std::thread<hob::decoder *>(handleServer, &dec);

    srandom(time(NULL));

    m_hi.my_id = m_put.my_id = m_get.my_id = m_bye.my_id = random();

    m_hi >> enc; std::cout << "Sending: "; m_hi >> enc_stdout; std::cout << std::endl;

    while (io.good())
    {
        std::string data;

        usleep(100000);

        std::cout << "Type a command (bye|put|get|msg): ";

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

            m_put >> enc; std::cout << "Sending: "; m_put >> enc_stdout; std::cout << std::endl;
        }

        if (data == "msg")
        {
            std::cout << "Enter text/json HOBs to send:" << endl;

            hobio::istream std_istream;
            hobio::json::decoder dec_stdin(std_istream);

            dec_stdin >> enc;
        }

        if (data == "get")
        {
            std::cout << "Enter question: ";

            std::getline(std::cin, data);

            m_get.data = data;

            m_get >> enc; std::cout << "Sending: "; m_get >> enc_stdout; std::cout << std::endl;
        }
    }

    m_bye >> enc; std::cout << "Sending: "; m_bye >> enc_stdout; std::cout << std::endl;

    th1.join();

    std::cout << "Finish" << std::endl;

    close(sockfd);

    return(0);
}
