/*
 * MOOSAsyncCommClient.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: pnewman
 */

#include <cmath>
#include <string>
#include <set>
#include <limits>
#include <iostream>
#include <iomanip>
#include <cassert>


#include "MOOS/libMOOS/Utils/MOOSUtils.h"
#include "MOOS/libMOOS/Utils/MOOSException.h"
#include "MOOS/libMOOS/Utils/MOOSScopedLock.h"

#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"
#include "MOOS/libMOOS/Comms/XPCTcpSocket.h"

namespace MOOS
{

bool AsyncCommsReaderDispatch(void * pParam)
{
	MOOSAsyncCommClient *pMe = (MOOSAsyncCommClient*)pParam;
	return pMe->ReadingLoop();
}

bool AsyncCommsWriterDispatch(void * pParam)
{
	MOOSAsyncCommClient *pMe = (MOOSAsyncCommClient*)pParam;

	return pMe->WritingLoop();
}

///default constructor
MOOSAsyncCommClient::MOOSAsyncCommClient()
{
	m_dfLastTimingMessage = 0.0;
}
///default destructor
MOOSAsyncCommClient::~MOOSAsyncCommClient()
{

}


std::string MOOSAsyncCommClient::HandShakeKey()
{
	return "asynchronous";
}


bool MOOSAsyncCommClient::StartThreads()
{
	m_bQuit = false;

	std::cerr<<"starting threads...";
    if(!WritingThread_.Initialise(AsyncCommsWriterDispatch,this))
        return false;

    if(!ReadingThread_.Initialise(AsyncCommsReaderDispatch,this))
            return false;

    if(!WritingThread_.Start())
        return false;

    if(!ReadingThread_.Start())
        return false;
	std::cerr<<"OK\n";

	return true;
}

bool MOOSAsyncCommClient::Flush()
{
	return DoWriting();
}

bool MOOSAsyncCommClient::Post(CMOOSMsg & Msg)
{
	if(!BASE::Post(Msg))
		return false;
	return Flush();
}

bool MOOSAsyncCommClient::WritingLoop()
{
	std::cerr<<"WritingLoop() Begins\n";

	while(!WritingThread_.IsQuitRequested())
	{
		//this is the connect loop...
		std::cerr<<"making socket "<<m_lPort<<std::endl;

		m_pSocket = new XPCTcpSocket(m_lPort);

		if(ConnectToServer())
		{

			while(!WritingThread_.IsQuitRequested())
			{

				if(!DoWriting())
					break;

				MOOSPause((int)(1000.0/m_nFundamentalFreq));

			}
		}
		//wait one second before try to connect again
		MOOSPause(1000);
	}


	//clean up on exit....
	if(m_pSocket!=NULL)
	{
		if(m_pSocket)
			delete m_pSocket;
		m_pSocket = NULL;
	}

	if(m_bQuiet)
		MOOSTrace("CMOOSAsyncCommClient::ClientLoop() quits\n");

	m_bConnected = false;

	return true;
}

bool MOOSAsyncCommClient::ReadingLoop()
{
	//not we will rely on our sibling writing thread to handle
	//the connected and reconnecting...
	while(!ReadingThread_.IsQuitRequested())
	{
		if(IsConnected())
		{
			DoReading();
		}
		else
		{
			MOOSPause(100);
		}
	}
	return true;
}


bool MOOSAsyncCommClient::DoWriting()
{

	//this existence of this object makes this scope
	//a critical section - this allows ::flush to be called
	//safely
	MOOS::ScopedLock WL(m_WorkLock);

	//this is the IO Loop
	try
	{

		if(!IsConnected())
			return false;

		//note the symmetry here... a warm feeling
		CMOOSCommPkt PktTx;

		m_OutLock.Lock();
		{
			//if nothing to send we send a NULL packet
			//just to tick things over..
			//we need to do this for old MOOSDB's
			if(m_OutBox.empty())
			{
				//default msg is MOOS_NULL_MSG
				CMOOSMsg Msg;
				Msg.m_sSrc = m_sMyName;
				m_OutBox.push_front(Msg);
			}

			//and once in a while we shall send a timing
			//message (this is the new style of timing
			if((MOOS::Time()-m_dfLastTimingMessage)>1)
			{
				std::cerr<<"instigating async timing\n";

				CMOOSMsg Msg(MOOS_TIMING,"_async_timing",0.0,MOOS::Time());
				m_OutBox.push_front(Msg);
				m_dfLastTimingMessage= Msg.GetTime();
			}

			//convert our out box to a single packet
			try
			{
				PktTx.Serialize(m_OutBox,true);
			}
			catch (CMOOSException e)
			{
				//clear the outbox
				m_OutBox.clear();
				throw CMOOSException("Serialisation Failed - this must be a lot of mail...");
			}

			//clear the outbox
			m_OutBox.clear();


		}
		m_OutLock.UnLock();

		//finally the send....
		SendPkt(m_pSocket,PktTx);

	}
	catch(CMOOSException e)
	{
		MOOSTrace("Exception in ClientLoop() : %s\n",e.m_sReason);
		OnCloseConnection();
		return false;//jump out to connect loop....
	}

	return true;


}

bool MOOSAsyncCommClient::IsRunning()
{
	return WritingThread_.IsThreadRunning() || ReadingThread_.IsThreadRunning();
}


bool MOOSAsyncCommClient::DoReading()
{

	try
	{
		CMOOSCommPkt PktRx;

		ReadPkt(m_pSocket,PktRx);

		double dfLocalRxTime =MOOSLocalTime();

		m_InLock.Lock();
		{
			if(m_InBox.size()>m_nInPendingLimit)
			{
				MOOSTrace("Too many unread incoming messages [%d] : purging\n",m_InBox.size());
				MOOSTrace("The user must read mail occasionally");
				m_InBox.clear();
			}

			//convert reply into a list of mesasges :-)
			//but no NULL messages
			//we ask serialise also to return the DB time
			//by looking in the first NULL_MSG in the packet - this is how timing worked
			//on vanilla clients
			double dfServerPktTxTime=std::numeric_limits<double>::quiet_NaN();

			//extract... and please leave NULL messages there
			PktRx.Serialize(m_InBox,false,false,&dfServerPktTxTime);

			//now Serialize simply adds to the front of a list so looking
			//at the first element allows us to check for timing information
			//as supported by the threaded server class
			if(m_InBox.front().IsType(MOOS_TIMING))
			{
				std::cerr<<"did async timing\n";
				//we do have a fancy new DB at the end.....
				if(m_bDoLocalTimeCorrection)
				{
					CMOOSMsg TimingMsg = m_InBox.front();
					m_InBox.pop_front();

					UpdateMOOSSkew(TimingMsg.GetDouble(),
							TimingMsg.GetTime(),
							MOOSLocalTime());

				}
			}
			else if(m_bDoLocalTimeCorrection && m_InBox.front().IsType(MOOS_NULL_MSG))
			{
				//looks like we have an old fashioned DB which sends timing
				//info at the front of every packet in a null message
				//we have no corresponding outgoing packet so not much we can
				//do other than imagine it tooks as long to send to the
				//DB as to receive...
				double dfTimeSentFromDB = m_InBox.front().GetDouble();
				double dfSkew = dfTimeSentFromDB-dfLocalRxTime;
				double dfTimeSentToDBApprox =dfTimeSentFromDB+dfSkew;

				m_InBox.pop_front();

				UpdateMOOSSkew(dfTimeSentToDBApprox,
						dfTimeSentFromDB,
						dfLocalRxTime);

			}

			m_bMailPresent = !m_InBox.empty();
		}
		m_InLock.UnLock();

		if(m_pfnMailCallBack!=NULL && m_bMailPresent)
		{
			bool bUserResult = (*m_pfnMailCallBack)(m_pMailCallBackParam);
			if(!bUserResult)
				MOOSTrace("user mail callback returned false..is all ok?\n");
		}
	}
	catch(CMOOSException e)
	{
		MOOSTrace("Exception in ReadLoop() : %s\n",e.m_sReason);
	}

	return true;
}

};


