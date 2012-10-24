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

MOOS::ThreadPrint gPrinter(std::cout);


bool _OnConnectNULL(void * pParam)
{
	//MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;
	return true;
}
bool _OnConnectRegister(void * pParam)
{
	MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;
	pC->Register("X",0.0);
	return true;
}


bool _OnMailPost(void *pParam)
{
	MOOSMSG_LIST M;
	MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;

	pC->Fetch(M);
	char X[400];
	pC->Notify("X",(void*)X,sizeof(X));
	return true;
}


bool _OnMail(void *pParam)
{
	MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;

	//std::cout<<MOOS::ConsoleColours::Red();
	//std::cerr<<pC->GetMOOSName()<<" Got mail at"<<std::setw(20)<<std::setprecision(15)<<MOOS::Time()<<std::endl;

	MOOSMSG_LIST M;
	pC->Fetch(M);
	MOOSMSG_LIST::iterator q;
	unsigned int k = 0;
	for(q=M.begin();q!=M.end();q++)
	{
		double dfLagMS =(MOOS::Time()-q->GetTime())/1e-3;
		gPrinter.Print(MOOSFormat("%s [%3d] lag:%.3f",pC->GetMOOSName().c_str(),k++,dfLagMS));
	}
	//std::cerr<<MOOS::ConsoleColours::reset();

	return true;
}


int main(int argc, char * argv[])
{
	std::vector<CMOOSCommClient*> Clients(2);
	for(unsigned int i = 0;i< Clients.size();i++)
	{
		CMOOSCommClient  * pNewClient;

		if( 1 || i %2 ==0 )
		{
			pNewClient = new MOOS::MOOSAsyncCommClient;
		}
		else
		{
			pNewClient = new CMOOSCommClient;
		}
		if(i == 0)
		{
			pNewClient->SetOnConnectCallBack(_OnConnectRegister,pNewClient);
		}
		else
		{
			pNewClient->SetOnConnectCallBack(_OnConnectRegister,pNewClient);
		}

		pNewClient->SetOnMailCallBack(_OnMail,pNewClient);

		std::stringstream ss;
		ss<<"C"<<i;
		pNewClient->Run("127.0.0.1",9000L,ss.str().c_str(),2);

		Clients[i] = pNewClient;

		MOOSPause(100);

	}

	Clients[1]->SetOnMailCallBack(_OnMail,Clients[1]);

	unsigned int i = 0;
	std::vector<unsigned char> Data(387);

	while(1 || i++<1000)
	{
		Clients[0]->Notify("X",Data.data(), Data.size());


		MOOSPause(10);


		//Msg.Trace();
	}




}
