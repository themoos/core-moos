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

#include "MOOS/libMOOS/Comms/MulticastNode.h"


bool SetupSocket(const std::string & sAddress, int Port);
bool WaitForSocket(int fd, int TimeoutSeconds);

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
    std::cerr<<"--all                  pass to network, it is a massacre\n";
    std::cerr<<"--query                find out who would jump\n";


    std::cerr<<"example    \n";
    std::cerr<<"    ./ktm  --phrase=\"die now\""<<"\n";

    exit(0);

}

int main(int argc, char *argv[])
{

    MOOS::CommandLineParser P(argc,argv);

    if(P.GetFlag("--help","-h"))
        PrintHelpAndExit();

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

        hops=1;
    }

    MOOS::MulticastNode FDUPA;
    FDUPA.Configure(multicast_address,multicast_port,hops);
    FDUPA.Run(true,true);


    bool bQuery = P.GetFlag("--query");
    if(bQuery)
    {
        std::cerr<<"searching for suicidal sleepers on "<<multicast_address<<":"<< multicast_port<<"\n";
        sPhrase="?"+sPhrase;
    }
    else
    {
        std::cerr<<"sending suicide instruction to "<<multicast_address<<":"<< multicast_port<<"\n";
    }

    FDUPA.Write(sPhrase);

    std::string sReply;
    while(FDUPA.Read(sReply,1000))
    {
        if(sReply==sPhrase)
            continue;

        std::cout<<(bQuery?MOOS::ConsoleColours::Yellow(): MOOS::ConsoleColours::Red())<<sReply;

    }
    std::cout<<MOOS::ConsoleColours::reset();

    return 0;


}

