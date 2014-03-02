#include "MOOS/libMOOS/Utils/ConsoleColours.h"
#include "MOOS/libMOOS/App/MOOSApp.h"


#ifndef _WIN32
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>

unsigned int GetScreenWidth()
{
   struct winsize ws;
   ioctl(0, TIOCGWINSZ, &ws);
   return ws.ws_col;
}
#else
unsigned int GetScreenWidth()
{
    return 80;
}
#endif



class MonitorTheMOOS :  public CMOOSApp
{
    bool OnConnectToServer()
    {
        return Register("DB_VARSUMMARY",0.0);
    }
    bool OnSummary(CMOOSMsg & M)
    {
        std::string sAll = M.GetString();
        while(!sAll.empty())
        {
            std::string sLine = MOOSChomp(sAll,"\n");
            std::string sVar = MOOSChomp(sLine," ");

            if(known_.find(sVar)==known_.end())
            {
                std::cerr<<MOOS::ConsoleColours::Magenta();
            }
            else if (known_[sVar]!=sLine)
            {
                std::cerr<<MOOS::ConsoleColours::Green();
            }
            else if(live_only_)
            {
                continue;
            }
            std::string output =sVar+" "+sLine;

            if(output.size()>GetScreenWidth()-3)
                output=output.substr(0,GetScreenWidth()-3)+"...";

            std::cout<<output<<std::endl<<MOOS::ConsoleColours::reset();

            known_[sVar]=sLine;

        }
        std::cout<<"\n";
        return true;
    }

    void OnPrintHelpAndExit()
    {
        std::cout<<"  --live_only                 : only display updating variables\n";
    }

    bool OnStartUp()
    {
        live_only_=m_CommandLineParser.GetFlag("--live_only");
        SetIterateMode(REGULAR_ITERATE_AND_COMMS_DRIVEN_MAIL);

        AddActiveQueue("summary",this,&MonitorTheMOOS::OnSummary);
        return AddMessageRouteToActiveQueue("summary","DB_VARSUMMARY");
    }

protected:
    std::map<std::string,std::string> known_;
    bool live_only_;

};


int main(int argc , char* argv[])
{
    MonitorTheMOOS mtm;
    mtm.Run("mtm",argc,argv);
}
