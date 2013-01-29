/*
 * QueueTest.cpp
 *
 *  Created on: Dec 6, 2012
 *      Author: pnewman
 */
#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"
#include "MOOS/libMOOS/Utils/ThreadPrint.h"
#include "MOOS/libMOOS/Utils/CommandLineParser.h"
#include "MOOS/libMOOS/App/MOOSApp.h"


std::vector<std::string> vars;
std::ofstream logfile("capture.txt");
MOOS::ThreadPrint gPrinter(logfile);

void LogMessage(CMOOSMsg & M)
{
	std::stringstream ss;
	ss.setf(std::ios::fixed);
	ss<<std::setprecision(4)<<M.GetTime()<<" ";
	ss<<std::setprecision(4)<<MOOSLocalTime()<<" ";
	ss<<M.GetKey();
	gPrinter.SimplyPrintTimeAndMessage(ss.str());
}

bool func(CMOOSMsg & M, void *pParam)
{
	LogMessage(M);
	return true;
}

bool on_mail(void * pParam)
{
	CMOOSCommClient * pC = static_cast<CMOOSCommClient*> (pParam);
	MOOSMSG_LIST Mail;
	pC->Fetch(Mail);

	MOOSMSG_LIST::iterator q;
	for(q=Mail.begin();q!=Mail.end();q++)
	{
		LogMessage(*q);
	}

	return true;
}


bool on_connect(void * pParam)
{
	CMOOSCommClient * pC = static_cast<CMOOSCommClient*> (pParam);
	for(unsigned int k = 0;k<vars.size();k++)
		pC->Register(vars[k]);

	return true;
}

int main(int argc, char * argv[])
{
	MOOS::MOOSAsyncCommClient C;
	MOOS::CommandLineParser P(argc,argv);
	P.GetFreeParameters(vars);

	for(unsigned int k = 0;k<vars.size();k++)
	{
		std::cout<<"installing callback for "<<vars[k]<<"\n";
		//C.AddMessageCallback(vars[k]+"_CB",vars[k],func,NULL);
	}

	C.SetOnMailCallBack(on_mail,&C);
	C.SetOnConnectCallBack(on_connect, &C);
	C.Run("localhost",9000,"capture_test");

	while(1)
	{
		MOOSPause(10000);
	}

}
