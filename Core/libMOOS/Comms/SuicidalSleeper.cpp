/*
 * SuicidalSleeper.cpp
 *
 *  Created on: Feb 4, 2014
 *      Author: pnewman
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string>
#include <arpa/inet.h>
#include <stdexcept>


#include "MOOS/libMOOS/Comms/SuicidalSleeper.h"
#include "MOOS/libMOOS/Utils/MOOSThread.h"

namespace MOOS {

class SuicidalSleeper::Impl
{
public:
    Impl()
    {
        multicast_group_IP_address_ = "224.1.1.3";
        multicast_port_  = 4000;
    }

    bool Run()
    {
        thread_.Initialise(dispatch_, this);
        return thread_.Start();
    }

    bool Work()
    {
        if(!SetupAndJoinMulticast())
            return false;

        //std::cerr<<"suicidal watch on "<<multicast_group_IP_address_<<":"<<multicast_port_<<"\n";
        fd_set fds;
        struct timeval timeout;

        while(!thread_.IsQuitRequested())
        {

            /* Set time limit. */
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            /* Create a descriptor set containing our two sockets.  */
            FD_ZERO(&fds);
            FD_SET(socket_fd_, &fds);
            int nSelectReturn = select(socket_fd_+1,
                    &fds,
                    NULL,
                    NULL,
                    &timeout);

            switch(nSelectReturn)
            {
            case -1:
                //oh dear
                break;
            case 0:
                //std::cerr<<"no input\n";
                //timeout
                break;
            default:
                {
                    // most likley our socket has return with something to read...
                    if (FD_ISSET (socket_fd_, &fds)!=0)
                    {
                        //do the read from the socket...
                        char raw[128];
                        //do the read from the socket...
                        int n = read(socket_fd_, raw, sizeof(raw));

                        if(n>0)
                        {
                            //we have a message;
                            std::string msg = std::string(raw);

                            if(msg=="die now I mean it\n")
                            {
                                std::cerr<<"that was the suicide code..\n";
                                exit(0);
                            }
                            else
                            {
                                std::cerr<<msg<<" is not the code...";
                            }
                        }
                    }
                }
            }//switch on select

        }//while thread active

        return true;
    }

    static bool dispatch_(void * pParam)
    {
        SuicidalSleeper::Impl* pMe = (SuicidalSleeper::Impl*)pParam;
        return pMe->Work();
    }


    bool SetupAndJoinMulticast()
    {
        try
        {
            //set up a simply DGRAM socket
            socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
            if(socket_fd_<0)
            {
                throw std::runtime_error("SetupAndJoinMulticast()::socket()");
            }

            //we want to be able to resuse it (multiple folk are interested)
            int reuse = 1;

            if (setsockopt(socket_fd_, SOL_SOCKET,SO_REUSEADDR/* SO_REUSEPORT*/, &reuse, sizeof(reuse)) == -1)
            {
                throw std::runtime_error("SetupAndJoinMulticast()::setsockopt::reuse");
            }

            /* construct a multicast address structure */
            struct sockaddr_in mc_addr;
            memset(&mc_addr, 0, sizeof(mc_addr));
            mc_addr.sin_family = AF_INET;
            mc_addr.sin_addr.s_addr = inet_addr(multicast_group_IP_address_.c_str());
            mc_addr.sin_port = htons(multicast_port_);

            if (bind(socket_fd_, (struct sockaddr*) &mc_addr, sizeof(mc_addr)) == -1)
            {
                throw std::runtime_error("SetupAndJoinMulticast()::setsockopt::bind");
            }

            //join the multicast group
            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = inet_addr(multicast_group_IP_address_.c_str());
            mreq.imr_interface.s_addr = INADDR_ANY;
            if(setsockopt(socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))==-1)
            {
                throw std::runtime_error("SetupAndJoinMulticast()::setsockopt::ADD_MEMBERSHIP");
            }
        }
        catch(std::exception & e)
        {

        }


        return true;
    }



    int socket_fd_;
    int multicast_port_;
    std::string multicast_group_IP_address_;

    CMOOSThread thread_;
};


SuicidalSleeper::SuicidalSleeper(): Impl_(new Impl) {
    // TODO Auto-generated constructor stub

}

SuicidalSleeper::~SuicidalSleeper() {
    // TODO Auto-generated destructor stub
    delete Impl_;
}


bool SuicidalSleeper::Run()
{
    return Impl_->Run();
}
}
