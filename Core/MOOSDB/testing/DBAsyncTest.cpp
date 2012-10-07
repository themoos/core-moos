/*
 * DBAsyncTest.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: pnewman
 */

#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"


bool _OnConnectC1(void * pParam)
{
	MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;
	return true;
}
bool _OnConnectC2(void * pParam)
{
	MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;
	pC->Register("X",0.0);
	return true;
}

bool _OnMail(void *pParam)
{
	MOOS::MOOSAsyncCommClient* pC = (MOOS::MOOSAsyncCommClient*)pParam;

	std::cerr<<MOOS::ConsoleColours::Red();
	std::cerr<<pC->GetMOOSName()<<" Got mail at"<<std::setw(20)<<std::setprecision(15)<<MOOS::Time()<<std::endl;

	MOOSMSG_LIST M;
	pC->Fetch(M);
	MOOSMSG_LIST::iterator q;
	for(q=M.begin();q!=M.end();q++)
	{
	//	q->Trace();
	}
	std::cerr<<MOOS::ConsoleColours::reset();

	return true;
}


int main(int argc, char * argv[])
{
	MOOS::MOOSAsyncCommClient C1,C2,C3;

	C1.SetOnConnectCallBack(_OnConnectC1,&C1);
	C2.SetOnConnectCallBack(_OnConnectC2,&C2);
	C3.SetOnConnectCallBack(_OnConnectC2,&C3);

	C1.SetOnMailCallBack(_OnMail,&C1);
	C2.SetOnMailCallBack(_OnMail,&C2);
	C3.SetOnMailCallBack(_OnMail,&C3);


	C1.Run("127.0.0.1",9000L,"C1",10);
	C2.Run("127.0.0.1",9000L,"C2",10);
	C3.Run("127.0.0.1",9000L,"C3",10);

	while(1)
	{
		MOOSPause(1000);
		CMOOSMsg Msg(MOOS_NOTIFY,"X",MOOS::Time() );
		C1.Post(Msg);C1.Flush();
		std::cerr<<"C1 posted at "<<std::setw(20)<<std::setprecision(15)<<Msg.GetTime()<<std::endl;
		//Msg.Trace();


	}
}
