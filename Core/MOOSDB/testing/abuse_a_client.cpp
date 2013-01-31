#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"

int count=0;
CMOOSLock L;


bool func(CMOOSMsg & M, void *pParam)
{
	L.Lock();
		count++;
	L.UnLock();
	return true;
}


bool on_connect(void * pParam)
{
	CMOOSCommClient * pC = static_cast<CMOOSCommClient*> (pParam);
	pC->Register("X");
	return true;
}

int main(int argc, char * argv[])
{
	MOOS::MOOSAsyncCommClient A,B;

	A.SetQuiet(true);
	B.SetQuiet(true);

	A.SetOnConnectCallBack(on_connect, &A);
	B.SetOnConnectCallBack(on_connect, &B);

	A.Run("localhost",9000,"A");
	B.Run("localhost",9000,"B");

	A.AddMessageCallback("CBA","X",func,NULL);
	B.AddMessageCallback("CBB","X",func,NULL);


	while(1)
	{

		A.Close();
		B.Close();
		A.Run("localhost",9000,"C");
		B.Run("localhost",9000,"D");

		while(!A.IsConnected())
			MOOSPause(10);

		while(!B.IsConnected())
			MOOSPause(10);

		A.AddMessageCallback("CBA","X",func,NULL);
		B.AddMessageCallback("CBB","X",func,NULL);


		//wait here because "X" is still in the DB so we will be told
		//of its existence the minute we connect...
		MOOSPause(100); //callback will have fired by now..

		//reset count
		std::cerr<<"clearing counter\n";
		count = 0;

		for(int i = 0;i<100;i++)
		{
			//send data from everyone to everyone
			//two hundred notifications and two interested parties
			A.Notify("X",i);
			B.Notify("X",i);
		}

		//this should be fast no need to wait long
		MOOSPause(100);

		//by this point we shoudl have received 400 messages
		std::cerr<<" received "<<count<< " and expected 400\n\n\n";

	}

}
