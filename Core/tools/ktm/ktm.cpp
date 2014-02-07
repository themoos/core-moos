/*
 * ktm.cpp
 *
 *  Created on: Feb 6, 2014
 *      Author: pnewman
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

#include "MOOS/libMOOS/Utils/CommandLineParser.h"
#include "MOOS/libMOOS/Utils/ThreadPrint.h"
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"



bool SetupSocket(const std::string & sAddress, int Port);

//such a simple program lets be old skool - file scope vars;
int socket_fd;
std::string sPhrase = "I need you to die now\n";
std::string multicast_address = "224.1.1.3";
int multicast_port = 4000;
unsigned char hops = 0;
struct sockaddr_in mc_addr;


void PrintHelpAndExit()
{
    std::cerr<<"Kill The MOOS (ktm)\n";
    std::cerr<<"--channel=<address>    address to issue kill on \n";
    std::cerr<<"--port=<int>           port to issue kill on \n";
    std::cerr<<"--phrase=<string>      pass phrase which Suicide listeners are keyed to\n";
    std::cerr<<"--all                  pass to network, its a massacre\n";
    std::cerr<<"example    \n";
    std::cerr<<"    ./ktm  --phrase=\"die now\""<<"\n";

    exit(0);

}

int main(int argc, char *argv[])
{
    MOOS::CommandLineParser P(argc,argv);


    P.GetVariable("--channel",multicast_address);
    P.GetVariable("--port",multicast_port);
    P.GetVariable("--phrase",sPhrase);

    if(P.GetFlag("--all"))
    {
        std::cerr<<MOOS::ConsoleColours::Red()<<"THIS WILL KILL ALL APPS WITHIN 1 HOP...[N/y]\n";
        std::cerr<<MOOS::ConsoleColours::reset();
        char answer;
        std::cin>>answer;
        if(answer!='y')
            return 0;

        hops=4;
    }

    if(P.GetFlag("--help","-h"))
        PrintHelpAndExit();

    if(!SetupSocket(multicast_address,multicast_port))
        return -1;

    int nSent = sendto(socket_fd,
              sPhrase.c_str(),
              sPhrase.size(),
              0,
              (struct sockaddr *)&mc_addr,
              sizeof(mc_addr));

    if(nSent>0)
        std::cerr<<"sent kill instruction to "<<multicast_address<<":"<< multicast_port<<"\n";


}


bool SetupSocket(const std::string & sAddress, int Port)
{
    try
    {
        //set up a simply DGRAM socket
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(socket_fd<0)
        {
            throw std::runtime_error("SetupSocket()::socket()");
        }

        /* construct a multicast address structure */
        //struct sockaddr_in mc_addr;
        memset(&mc_addr, 0, sizeof(mc_addr));
        mc_addr.sin_family = AF_INET;
        mc_addr.sin_addr.s_addr = inet_addr(sAddress.c_str());
        mc_addr.sin_port = htons(Port);


        if(setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_TTL, &hops, sizeof(hops))==-1)
        {
            return false;
        }


    }
    catch(std::exception & e)
    {
        std::cerr<<"issue with socket creation :"<<e.what()<<"\n";
        return false;
    }


    return true;
}
