/*
 * DBAsyncTest.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: pnewman
 */

#include "MOOS/libMOOS/Comms/MOOSCommClient.h"
#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"
#include "MOOS/libMOOS/Utils/ThreadPrint.h"
#include "MOOS/libMOOS/Thirdparty/getpot/getpot.h"
#include <map>
#include <numeric>
#include <iterator>

#include <limits>
#include <iostream>
#include <fstream>

MOOS::ThreadPrint gPrinter(std::cout);

#define NUM_CLIENTS 10
#define PAYLOAD_SIZE 64*1024
#define TEST_PERIOD 20.0
#define MESSAGE_PERIOD_MS 50


CMOOSLock Lock;

std::map<std::string,std::vector<double> >  LagLogs;
bool bEnableCapture=false;

bool _OnConnectNULL(void * pParam)
{
	return true;
}

bool _OnConnectRegister(void * pParam)
{
	CMOOSCommClient* pC = (CMOOSCommClient*) pParam;
	pC->Register("X",0.0);
	return true;
}


bool _OnMail(void *pParam)
{
	CMOOSCommClient* pC = (CMOOSCommClient*)pParam;

	//get the mail
	MOOSMSG_LIST M;
	pC->Fetch(M);

	MOOSMSG_LIST::iterator q;
	//unsigned int k = 0;
	double dfNow = MOOSLocalTime();
	for(q=M.begin();q!=M.end();q++)
	{
		if(!q->IsSkewed(dfNow) && bEnableCapture)
		{

			double dfLagMS =(dfNow-q->GetTime())/1e-3;
			Lock.Lock();
			LagLogs[pC->GetMOOSName()].push_back(dfLagMS);
			Lock.UnLock();

			//gPrinter.Print(MOOSFormat("%s [%3d] lag:%.3f",pC->GetMOOSName().c_str(),k++,dfLagMS));
		}
	}

	return true;
}


int main(int argc, char * argv[])
{

	GetPot cl(argc,argv);

	double dfTestPeriod = cl.follow(20,2,"-p","--test_period");
	unsigned int message_period= cl.follow(100,2,"-m","--message_period_ms");
	unsigned int num_clients= cl.follow(40,2,"-c","--num_clients");
	unsigned int payload_size = cl.follow(40,2,"-s","--paylaod_size");


	std::vector<CMOOSCommClient*> Clients(num_clients);
	for(unsigned int i = 0;i< Clients.size();i++)
	{
		CMOOSCommClient  * pNewClient;

		if( i %2 ==0 )
		{
			//even numbers are modern
			pNewClient = new MOOS::MOOSAsyncCommClient;
		}
		else
		{
			//odd numbers are old skool
			pNewClient = new CMOOSCommClient;
		}

		pNewClient->SetOnConnectCallBack(_OnConnectRegister,pNewClient);

		//everyone gets the same mail call back
		pNewClient->SetOnMailCallBack(_OnMail,pNewClient);

		//name them C0:N
		std::stringstream ss;
		ss<<i;
		pNewClient->Run("127.0.0.1",9000L,ss.str().c_str(),20);

		Clients[i] = pNewClient;

		//start 10 every second.
		MOOSPause(100);
	}

	//let capture start
	bEnableCapture = true;

	unsigned int i = 0;
	std::vector<unsigned char> Data(payload_size);

	double dfStart = MOOSLocalTime();

	while((MOOSLocalTime()-dfStart)<dfTestPeriod)
	{
		if(i%10==0)
			MOOSTrace("%4d messages sent %.1f seconds to go\r",i,dfTestPeriod-(MOOSLocalTime()-dfStart));

		//send 1K
		Clients[0]->Notify("X",Data.data(), Data.size(),MOOSLocalTime());

		//write at 1/MESSAGE_PERIOD_MS Hz
		MOOSPause(message_period);

		i++;
	}

	std::map<std::string,std::vector<double> >::iterator q;

	std::ofstream logfile("asynctest.log");

	std::cerr<<std::setw(15)<<"Client"<<std::setw(15)<<"mean latency (ms)"<<std::setw(15)<<"#msgs"<<std::endl;

	unsigned int umin = std::numeric_limits<unsigned int>::max();
	for(q = LagLogs.begin();q!=LagLogs.end();q++)
	{
		umin = std::min<unsigned int>(umin, q->second.size());
	}

	for(q = LagLogs.begin();q!=LagLogs.end();q++)
	{
		std::vector<double> & rV = q->second;
		double dfAv = std::accumulate(rV.begin(), rV.end(),0.0)/rV.size();

		logfile<<q->first<<" ";
		std::copy(rV.begin(),rV.begin()+umin, std::ostream_iterator<double>(logfile," "));
		logfile<<std::endl;

		std::cerr<<std::setw(15)<<q->first<<std::setw(15)<<std::fixed<<std::setprecision(2)<<dfAv<<std::setw(15)<<rV.size()<<std::endl;
	}

	std::cout<<"detailed log written to "<<"asynctest.log\n";
	std::cout<<num_clients<<" clients handling  "<<payload_size<<" byte packets at "<<1000.0/message_period<<" Hz\n";

}
