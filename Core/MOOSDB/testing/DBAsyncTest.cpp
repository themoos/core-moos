/*
 * DBAsyncTest.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: pnewman
 */

#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"

#include "MOOS/libMOOS/Utils/SafeList.h"

MOOS::SafeList<double> _gTimes;

bool _OnConnectNULL(void * pParam)
{
	MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;
	return true;
}
bool _OnConnectRegister(void * pParam)
{
	MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;
	pC->Register("X",0.0);
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
	for(q=M.begin();q!=M.end();q++)
	{
		_gTimes.Push((MOOS::Time()-q->GetTime())/1e-3);
		std::cerr<<pC->GetMOOSName()<<"lag:"<<std::setprecision(3)<<(MOOS::Time()-q->GetTime())/1e-3<<"ms\n";
	}
	//std::cerr<<MOOS::ConsoleColours::reset();

	return true;
}


int main(int argc, char * argv[])
{
	std::vector<MOOS::MOOSAsyncCommClient*> Clients(3);

	for(unsigned int i = 0;i< Clients.size();i++)
	{
		MOOS::MOOSAsyncCommClient* pNewClient = new MOOS::MOOSAsyncCommClient;

		if(i == 0)
		{
			pNewClient->SetOnConnectCallBack(_OnConnectNULL,pNewClient);
		}
		else
		{
			pNewClient->SetOnConnectCallBack(_OnConnectRegister,pNewClient);
		}

		pNewClient->SetOnMailCallBack(_OnMail,pNewClient);

		std::stringstream ss;
		ss<<"C"<<i;
		pNewClient->Run("127.0.0.1",9000L,ss.str().c_str(),1);

		Clients[i] = pNewClient;

		MOOSPause(100);

	}




	unsigned int i = 0;
	while(i++<1)
	{
		CMOOSMsg Msg(MOOS_NOTIFY,"X",MOOS::Time() );
		Clients[0]->Post(Msg);
		std::cerr<<"C0 posted at "<<std::setw(20)<<std::setprecision(15)<<Msg.GetTime()<<std::endl;
		MOOSPause(100);
		//Msg.Trace();
	}

	MOOSPause(1000);

	i = 0;
	while(_gTimes.Size())
	{
		double T;
		_gTimes.Pull(T);
		std::cout<<i++<<":"<<T<<std::endl;
	}


}
