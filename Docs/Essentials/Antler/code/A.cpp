
#include <MOOSLIB/MOOSApp.h>

class CA:public CMOOSApp
{
    bool OnStartUp()
    {
        MOOSTrace("Example A : This a vanilla MOOSApp");
        std::string sPrintThis = "not good";
        if(m_MissionReader.GetConfigurationParam("PrintThis",sPrintThis))
        {
            MOOSTrace("Found print directive: print : \"\"\n",sPrintThis.c_str());
        }
        else
        {
            MOOSTrace("Failed to find print directive - bad\n");
        }
        return true;
    }
};

int main(int argc,char *argv[])
{
    char * sMissionFile = "Mission.moos";
    char * sMOOSName = "pTestAppA";
    switch(argc)
    {
    case 3:
        sMOOSName = argv[2];
    case 2:
        sMissionFile = argv[1];
    case 1:
        break;
    default:
        for(int i = 3;i<argc;i++)
        {
            MOOSTrace("arg[%d] = %s\n",i,argv[i])
        }
    }
    CA A;
    A.Run(sMOOSName,sMissionFile);
}