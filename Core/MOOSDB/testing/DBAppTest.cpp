#include "MOOS/libMOOS/App/MOOSApp.h"
#include "MOOS/libMOOS/Thirdparty/getpot/getpot.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"


void PrintHelpAndExit()
{
    MOOSTrace("\n\nThis test tests the behaviour and reponse of a DB/App pair when the application"
    		"publishes in every iteration the variable it is subscribing to. Behaviour in this case"
    		"is governed (when using Asynchronous comms clients) by the MaximumAppTick parameter. \n");
    MOOSTrace("  -m (--maxapptick)                 : MaximumAppTick in Hz\n");
    MOOSTrace("  -a (--apptick)                    : MOOSAppTick in Hz\n");

}
class TestApp : public CMOOSApp
{
public:
	TestApp(int argc, char * argv[])
	{
		GetPot cl(argc,argv);
		request_max_freq_ = cl.follow(0.0,2,"-m","--maxlimit");
		request_app_freq_ = cl.follow(20.0,2,"-a","--apptick");
	}
	bool OnNewMail(MOOSMSG_LIST & List)
	{

		return true;
	}
	bool Iterate()
	{
		double dfAlpha = 0.6;
		static double dfRate = 0;
		static bool crazy_state = false;

		dfRate  = dfAlpha*dfRate+ (1.0-dfAlpha)/(MOOS::Time()-GetLastIterateTime());

		int print_if =  crazy_state ? (int)request_max_freq_ : (int)request_app_freq_;

		if(GetIterateCount()%print_if==0)
		{
			std::cout<<MOOSFormat(" Iterate Running at %.1f Hz\n",dfRate);
		}

		//every 5 seconds, in five seconds bursts publish what we want to
		//receive ourselves
		if(int(MOOSTime()-GetAppStartTime()) % 10 > 5)
		{
			if(!crazy_state)
			{
				std::cout<<"starting to post \"X\" from iterate...\n";
				crazy_state = true;
			}
			Notify("X",MOOS::Time());
		}
		else
		{
			if(crazy_state)
			{
				std::cout<<"stopping sending \"X\" in iterate...\n";
				crazy_state = false;
			}
		}

		return true;
	}
	bool OnStartUp()
	{
		SetAppFreq(request_app_freq_,request_max_freq_);
		return true;
	}

	bool OnConnectToServer()
	{
		Register("X",0.0);
		return true;
	}

protected:
	double request_max_freq_;
	double request_app_freq_;
};

int main(int argc, char * argv[])
{
	TestApp A(argc,argv);
	A.Run("Recurse","Mission.moos");
}
