/*
 * erm.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: pnewman
 */

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
	TestApp()
	{
		//request_max_freq_ = 20.0;
		//request_app_freq_ = 10.0;
	}
	virtual ~TestApp()
	{
		std::cerr<<"~TestApp\n";
	}
	bool OnNewMail(MOOSMSG_LIST & List)
	{/*
		static double dfRate = 0;
		static double dfLastTime = 0.0;
		static double dfLastPrintTime = 0.0;

		double dfAlpha = 0.6;

		dfRate  = dfAlpha*dfRate+ (1.0-dfAlpha)/(MOOS::Time()-dfLastTime);

		if(MOOS::Time()-dfLastPrintTime>1.0)
		{
			dfLastPrintTime = MOOS::Time();
			std::cout<<MOOSFormat(" OnNewMail Running at %.1f Hz\n",dfRate);
		}

		dfLastTime = MOOS::Time();
*/
		return true;
	}
	bool Iterate()
	{
		/*
		double dfAlpha = 0.6;
		static double dfRate = 0;
		static double dfLastTime = 0.0;
		static double dfLastPrintTime = 0.0;
		static bool crazy_state = false;

		dfRate  = dfAlpha*dfRate+ (1.0-dfAlpha)/(MOOS::Time()-dfLastTime);

		if(MOOS::Time()-dfLastPrintTime>1.0)
		{
			dfLastPrintTime = MOOS::Time();
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

		dfLastTime = MOOS::Time();
		*/
		return true;
	}


	bool OnStartUp()
	{

		SetAppFreq(10,20);
		//SetAppFreq(request_app_freq_,request_max_freq_);
		SetIterateMode(REGULAR_ITERATE_AND_MAIL);
		return true;
	}

	bool OnConnectToServer()
	{
		//Register("X",0.0);
		return true;
	}

protected:
	double request_max_freq_;
	double request_app_freq_;
};

int main(int argc, char ** argv)
{
	GetPot cl(argc,argv);

	TestApp A;

	A.Run("Recurse","Mission.moos");
}
