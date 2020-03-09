#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "fdstreambuf.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <list>
#include <thread.hpp>
#include "../hobs.h"

HOB::Dump dump_mode = HOB::JSON;

Hello                  m_hi                    ;
Put                    m_put                   ;
Get                    m_get                   ;
Bye                    m_bye                   ;
MyStruct               m_MyStruct              ;
AnotherStruct          m_AnotherStruct         ;
NoParamMessage         m_NoParamMessage        ;
NumericNoParamMessage  m_NumericNoParamMessage ;
NumericMessage         m_NumericMessage        ;
NumericExtraParameters m_NumericExtraParameters;
ComplexStruct          m_ComplexStruct         ;

static std::list<std::iostream *> streamList;

static void wall(HOB &m, ostream *s)
{
    // for (auto s : streamList)
    {
        if (m >> m_hi)
        {
            (*s) << m_hi; std::cout << "Sending: " << m_hi(HOB::JSON) << std::endl;
        }
        else
        if (m >> m_put)
        {
            m_get.data = "recv some data";

            (*s) << m_get; std::cout << "Sending: " << m_get(HOB::JSON) << std::endl;
        }
        else
        if (m >> m_get)
        {
            m_put.data = "sent some data";

            (*s) << m_put; std::cout << "Sending: " << m_put(HOB::JSON) << std::endl;
        }
        else
        if (m >> m_bye)
        {
            (*s) << m_bye; std::cout << "Sending: " << m_bye(HOB::JSON) << std::endl;
        }
    }
}

bool handle_message(HOB &m)
{
    bool handled = true;

    if ((m == m_MyStruct              ) ||
        (m == m_AnotherStruct         ) ||
        (m == m_NoParamMessage        ) ||
        (m == m_NumericNoParamMessage ) ||
        (m == m_NumericMessage        ) ||
        (m == m_NumericExtraParameters) ||
        (m == m_ComplexStruct         ))
    {
        printf("Known HOB\n");
    }

    if (m == m_AnotherStruct)
    {
        LOG(m_AnotherStruct);
    }

#if 0
    if ((m == m_AnotherStruct) || (m == m_NumericNoParamMessage))
    {
        printf("Skipping\n");

        LOG(m);

        m.skip();

        printf("Skipped\n");

        return;
    }
#endif

    if (m >> m_MyStruct)
    {
        LOG(m_MyStruct);
    }
    else
    if (m >> m_AnotherStruct)
    {
        LOG(m_AnotherStruct);

        if (m_AnotherStruct)
        {
            if (m_AnotherStruct & AnotherStruct::_bnil)
            {
                printf("bnil changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_bone)
            {
                printf("bone changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_bmin)
            {
                printf("bmin changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_bmax)
            {
                printf("bmax changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_snil)
            {
                printf("snil changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_sone)
            {
                printf("sone changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_smin)
            {
                printf("smin changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_smax)
            {
                printf("smax changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_inil)
            {
                printf("inil changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_ione)
            {
                printf("ione changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_imin)
            {
                printf("imin changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_imax)
            {
                printf("imax changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_lnil)
            {
                printf("lnil changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_lone)
            {
                printf("lone changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_lmin)
            {
                printf("lmin changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_lmax)
            {
                printf("lmax changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_bar )
            {
                printf("bar  changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_dat )
            {
                printf("dat  changed\n");

                if (m_AnotherStruct.dat)
                {
                    if (m_AnotherStruct.dat & MyStruct::_anEnum)
                    {
                        printf("dat.anEnum changed\n");

                        m_AnotherStruct.dat -= MyStruct::_anEnum;
                    }
                    if (m_AnotherStruct.dat & MyStruct::_aChar)
                    {
                        printf("dat.aChar changed\n");

                        m_AnotherStruct.dat -= MyStruct::_aChar;
                    }
                    if (m_AnotherStruct.dat & MyStruct::_aFloat)
                    {
                        printf("dat.aFloat changed\n");

                        m_AnotherStruct.dat -= MyStruct::_aFloat;
                    }
                    if (m_AnotherStruct.dat & MyStruct::_optional)
                    {
                        printf("dat.optional changed\n");

                        m_AnotherStruct.dat -= MyStruct::_optional;
                    }

                    if (m_AnotherStruct.dat & MyStruct::_anEnum)
                    {
                        printf("dat.anEnum still changed\n");
                    }
                    if (m_AnotherStruct.dat & MyStruct::_aChar)
                    {
                        printf("dat.aChar still changed\n");
                    }
                    if (m_AnotherStruct.dat & MyStruct::_aFloat)
                    {
                        printf("dat.aFloat still changed\n");
                    }
                    if (m_AnotherStruct.dat & MyStruct::_optional)
                    {
                        printf("dat.optional still changed\n");
                    }
                }
            }
            if (m_AnotherStruct & AnotherStruct::_aMap)
            {
                printf("aMap changed\n");
            }

            printf("\n");

            ~m_AnotherStruct;
        }
    }
    else
    if (m >> m_NoParamMessage)
    {
        LOG(m_NoParamMessage);
    }
    else
    if (m >> m_NumericNoParamMessage)
    {
        LOG(m_NumericNoParamMessage);
    }
    else
    if (m >> m_NumericMessage)
    {
        LOG(m_NumericMessage);
    }
    else
    if (m >> m_ComplexStruct)
    {
        LOG(m_ComplexStruct);
    }
    else
    if (m_NumericExtraParameters << m)
    {
        LOG(m_NumericExtraParameters);
    }
    else
    {
        printf("Unknown HOB\n");
        LOG(m);

        handled = false;
    }

    return handled;
}

class Client
{
public:
    int fd;
    std::string remote;

    Client(int fd_, const std::string &remote_)
        : fd(fd_)
        , remote(remote_)
    {}
};

static void handleClient(Client client)
{
    wl::fdstreambuf sbuf(client.fd);
    std::iostream io(&sbuf);
    streamList.push_back(&io);

    HOB m;

    while (io >> m)
    {
        std::cout << "Received: ";

        if (m >> m_hi)
        {
            std::cout << m_hi(HOB::JSON);
        }
        else
        if (m >> m_put)
        {
            std::cout << m_put(HOB::JSON);
        }
        else
        if (m >> m_get)
        {
            std::cout << m_get(HOB::JSON);
        }
        else
        if (m >> m_bye)
        {
            std::cout << m_bye(HOB::JSON);
        }
        else 
        if (!handle_message(m))
        {
            std::cout << "Unknown HOB: " << m(HOB::JSON);
        }

        std::cout << std::endl;

        wall(m, &io);

        if (m == m_bye)
        {
            break;
        }
    }

    streamList.remove(&io);
    close(client.fd);
    std::cout << "Disconnect from " << client.remote << std::endl;
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

        Client client(accept(sockfd, reinterpret_cast<struct sockaddr *>(&dst), &size),
                      inet_ntoa(dst.sin_addr));

        std::cout << "Connect from " << client.remote << std::endl;

        std::thread<Client> th = std::thread<Client>(handleClient, client);

        th.detach();
    }

    return(0);
}
