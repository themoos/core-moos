/**
///////////////////////////////////////////////////////////////////////////
//
//   This file is part of the MOOS project
//
//   MOOS : Mission Oriented Operating Suite A suit of 
//   Applications and Libraries for Mobile Robotics Research 
//   Copyright (C) Paul Newman
//    
//   This software was written by Paul Newman at MIT 2001-2002 and 
//   the University of Oxford 2003-2013 
//   
//   email: pnewman@robots.ox.ac.uk. 
//              
//   This source code and the accompanying materials
//   are made available under the terms of the GNU Lesser Public License v2.1
//   which accompanies this distribution, and is available at
//   http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
//          
//   This program is distributed in the hope that it will be useful, 
//   but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
////////////////////////////////////////////////////////////////////////////
**/




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
bool bLog=false;
double dfStartTime=0;
void PrintHelpAndExit()
{
	std::cerr<<"./capture --moos_host=[localhost] --moos_port=[9000] [--log]  X Y Z\n";
	exit(0);
}

void LogMessage(CMOOSMsg & M)
{
	double dfDelay = MOOSLocalTime()-M.GetTime();
	if(dfDelay>30e-3)
	{
		std::cerr<<MOOSTime()-dfStartTime<<" ouch! delay is "<<dfDelay<<"\n";
	}
	if(bLog)
	{
		std::stringstream ss;
		ss.setf(std::ios::fixed);
		ss<<std::setprecision(4)<<M.GetTime()<<" ";
		ss<<std::setprecision(4)<<MOOSLocalTime()<<" ";
		ss<<M.GetKey();
		gPrinter.SimplyPrintTimeAndMessage(ss.str());
	}

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

    int policy;
    struct sched_param param;

if(0)
{
	pthread_getschedparam(pthread_self(), &policy, &param);
	std::cout<<"default priority"<< param.sched_priority<<"\n";
	param.sched_priority = sched_get_priority_max(policy);
	std::cout<<"max priority"<< param.sched_priority<<"\n";
	if(pthread_setschedparam(pthread_self(), policy, &param)!=0)
	{
		std::cout<<"it did not work\n";
	}
}

	dfStartTime = MOOSLocalTime();
	MOOS::MOOSAsyncCommClient C;
	MOOS::CommandLineParser P(argc,argv);

	if(P.GetFlag("--help"))
		PrintHelpAndExit();

	P.GetFreeParameters(vars);

	std::string sHost = "localhost";
	unsigned int port = 9000;
	P.GetVariable("--moos_host",sHost);
	P.GetVariable("--moos_port",port);

	bLog = P.GetFlag("--log");



	for(unsigned int k = 0;k<vars.size();k++)
	{
		std::cout<<"installing callback for "<<vars[k]<<"\n";
		//C.AddMessageCallback(vars[k]+"_CB",vars[k],func,NULL);
	}

	C.SetOnMailCallBack(on_mail,&C);
	C.SetOnConnectCallBack(on_connect, &C);
	C.Run(sHost,port,"capture_test");

	while(1)
	{
		MOOSPause(10000);
	}

}
