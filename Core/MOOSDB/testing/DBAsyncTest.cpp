/*
 * DBAsyncTest.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: pnewman
 */

#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"


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
	std::cerr<<pC->GetDescription()<<" Got mail at"<<std::setw(20)<<std::setprecision(15)<<MOOS::Time()<<std::endl;
	return true;
}


int main(int argc, char * argv[])
{
	MOOS::MOOSAsyncCommClient C1,C2;

	C1.SetOnConnectCallBack(_OnConnectC1,&C1);
	C2.SetOnConnectCallBack(_OnConnectC2,&C2);

	C1.SetOnMailCallBack(_OnMail,&C1);
	C2.SetOnMailCallBack(_OnMail,&C2);

	C1.Run("127.0.0.1",9000L,"C1",10);
	C2.Run("127.0.0.1",9000L,"C2",10);

	while(1)
	{
		MOOSMSG_LIST M;
		C1.Fetch(M);
		MOOSMSG_LIST::iterator q;
		for(q=M.begin();q!=M.end();q++)
		{
			q->Trace();
		}
		MOOSPause(1000);
		CMOOSMsg Msg(MOOS_NOTIFY,"X",MOOS::Time() );
		C1.Post(Msg);
		std::cerr<<"posted at "<<std::setw(20)<<std::setprecision(15)<<Msg.GetTime()<<std::endl;

	}
}
