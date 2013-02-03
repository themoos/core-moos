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


#include "MOOS/libMOOS/Utils/CommandLineParser.h"
#include "MOOS/libMOOS/Comms/XPCTcpSocket.h"
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"

#include "MOOS/libMOOS/Utils/ThreadPrint.h"

#include <fstream>




void PrintHelpAndExit()
{
	exit(0);
}

struct Msg
{
	double time;
	double val;
	unsigned char space[100];
};

int main(int argc, char * argv[])
{


	MOOS::CommandLineParser P(argc,argv);

	if(P.GetFlag("--help"))
		PrintHelpAndExit();

	std::string sHost = "localhost";
	int nPort = 9007;

	P.GetVariable("--host",sHost);
	P.GetVariable("--port",nPort);

	if(P.GetFlag("--send"))
	{
		XPCTcpSocket* pSocket;
		while(1)
		{
			try
			{
				//try to connect
				pSocket =  new XPCTcpSocket((long)nPort);
				pSocket->vConnect(sHost.c_str());
				break;
			}
			catch(XPCException & e)
			{
				std::cerr<<e.sGetException()<<"\n";
			}
			delete pSocket;
			MOOSPause(1000);
		}

		Msg M;
		M.val = 0.0;
		while(1)
		{
			M.time=MOOSLocalTime();
			pSocket->iSendMessage(&M,sizeof(M));
			MOOSPause(10);
		}
	}

	if(P.GetFlag("--receive"))
	{
		std::ofstream f("cm2sock.txt");
		MOOS::ThreadPrint gPrinter(f);

		XPCTcpSocket* pListenSocket = new XPCTcpSocket((long)nPort);

		try
		{
			pListenSocket->vSetReuseAddr(1);
			pListenSocket->vBindSocket();
		}
		catch(XPCException & e)
		{
			std::cerr<<e.sGetException()<<"\n";
			return -1;
		}


		while(1)
		{
			Msg M;
			XPCTcpSocket* pSocket;
			try
			{
				pListenSocket->vListen(5);
				pSocket = pListenSocket->Accept();

				while(1)
				{
					if(pSocket->iRecieveMessage(&M,sizeof(M))==sizeof(M))
					{
						double dfT = MOOSLocalTime()-M.time;
						gPrinter.SimplyPrintTimeAndMessage(MOOSFormat("%f",dfT));
						if(dfT>10e-3)
							std::cerr<<dfT<<"\n";
					}
				}

			}
			catch(XPCException & e)
			{
				std::cerr<<e.sGetException()<<"\n";
				delete pSocket;
			}

		}
	}


}
