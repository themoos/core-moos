///////////////////////////////////////////////////////////////////////////
//
//   MOOS - Mission Oriented Operating Suite 
//  
//   A suit of Applications and Libraries for Mobile Robotics Research 
//   Copyright (C) 2001-2005 Massachusetts Institute of Technology and 
//   Oxford University. 
//    
//   This software was written by Paul Newman at MIT 2001-2002 and Oxford 
//   University 2003-2005. email: pnewman@robots.ox.ac.uk. 
//      
//   This file is part of a  MOOS Core Component. 
//        
//   This program is free software; you can redistribute it and/or 
//   modify it under the terms of the GNU General Public License as 
//   published by the Free Software Foundation; either version 2 of the 
//   License, or (at your option) any later version. 
//          
//   This program is distributed in the hope that it will be useful, 
//   but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
//   General Public License for more details. 
//            
//   You should have received a copy of the GNU General Public License 
//   along with this program; if not, write to the Free Software 
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//   02111-1307, USA. 
//
//////////////////////////    END_GPL    //////////////////////////////////
// MOOSCommObject.cpp: implementation of the CMOOSCommObject class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
    #pragma warning(disable : 4786)
    #pragma warning(disable : 4503)
#endif

#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"
#include "MOOS/libMOOS/Comms/MOOSCommPkt.h"
#include "MOOS/libMOOS/Comms/XPCTcpSocket.h"
#include "MOOS/libMOOS/Comms/MOOSCommObject.h"
#include "MOOS/libMOOS/Utils/MOOSException.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSCommObject::CMOOSCommObject()
{
    m_bFakeDodgyComms = false;
    m_dfDodgeyCommsDelay = 1.0;
    m_dfDodgeyCommsProbability = 0.5;
    m_dfTerminateProbability = 0.0;
}

CMOOSCommObject::~CMOOSCommObject()
{

}

bool CMOOSCommObject::ConfigureCommsTesting(double dfDodgeyCommsProbability,double dfDodgeyCommsDelay, double dfTerminateProbability)
{
    m_bFakeDodgyComms = true;
    m_dfDodgeyCommsProbability = dfDodgeyCommsProbability;
    m_dfDodgeyCommsDelay = dfDodgeyCommsDelay;
    m_dfTerminateProbability = dfTerminateProbability;
    return true;
}

void CMOOSCommObject::SimulateCommsError()
{
    if(MOOSUniformRandom(0.0,1.0)<m_dfDodgeyCommsProbability)
    {
        std::cerr<<MOOS::ConsoleColours::Yellow();
        std::cerr<<"faking slow connection..."<<m_dfDodgeyCommsDelay<<"s sleep\n";
        std::cerr<<MOOS::ConsoleColours::reset();

        MOOSPause((int)(m_dfDodgeyCommsDelay*1000));


    }

    if(MOOSUniformRandom(0.0,1.0)<m_dfTerminateProbability)
    {
       std::cerr<<MOOS::ConsoleColours::Red();
       std::cerr<<"faking application-abort mid transaction\n";
       std::cerr<<MOOS::ConsoleColours::reset();
       exit(-1);
    }
}

bool CMOOSCommObject::ReadPkt(XPCTcpSocket *pSocket, CMOOSCommPkt &PktRx, int nSecondsTimeout)
{
    #define CHUNK_READ 8192
    unsigned char Buffer[CHUNK_READ];
    unsigned char *pBuffer = Buffer;

    //now receive a message back..
    int nRqd=0;
    while((nRqd=PktRx.GetBytesRequired())!=0)
    {
        int nRxd = 0;

        try
        {
            if(nRqd<CHUNK_READ)
            {
                //read in in chunks of 1k
                if(nSecondsTimeout<0)
                {
                    nRxd  = pSocket->iRecieveMessage(pBuffer,nRqd);
                }
                else
                {
                    nRxd  = pSocket->iReadMessageWithTimeOut(pBuffer,nRqd,(double)nSecondsTimeout);
                }
            }
            else
            {
                //read in in chunks of 1k
                if(nSecondsTimeout<0)
                {
                    nRxd  = pSocket->iRecieveMessage(pBuffer,CHUNK_READ);
                }
                else
                {
                    nRxd  = pSocket->iReadMessageWithTimeOut(pBuffer,CHUNK_READ,(double)nSecondsTimeout);
                }
            }
        }
        catch(XPCException e)
        {
            MOOSTrace("Exception %s\n",e.sGetException());
            throw CMOOSException("CMOOSCommObject::ReadPkt() Failed Rx");
        }

        switch(nRxd)
        {
        case -1:
            throw CMOOSException("Gross error....");
            break;
        case 0:
            if(nSecondsTimeout>0)
                throw CMOOSException(MOOSFormat("remote side closed or lazy client ( waited more than %ds )",nSecondsTimeout));
            else
                throw CMOOSException("remote side closed....");
            break;
        default:
            PktRx.Fill(pBuffer,nRxd);
            break;
        }
    }

    return true;
}

bool CMOOSCommObject::SendPkt(XPCTcpSocket *pSocket, CMOOSCommPkt &PktTx)
{
    int nSent = 0;

    try
    {


        if(m_bFakeDodgyComms)
        {
            //this is some very low level cruft that is only hear to provide
        	//some gruesome testing - normal programmers should ignore this
        	//block of code
        	nSent+=pSocket->iSendMessage(PktTx.m_pStream,sizeof(int));
        	SimulateCommsError();
        	nSent+=pSocket->iSendMessage(&(PktTx.m_pStream[sizeof(int)]),PktTx.GetStreamLength()-sizeof(int));
        }
        else
        {
        	nSent = pSocket->iSendMessage(PktTx.m_pStream,PktTx.GetStreamLength());
        }
    }
    catch(XPCException e)
    {
        MOOSTrace("MOOSCommObject::SendPkt Exception caught %s\n",e.sGetException());
        throw CMOOSException("CMOOSCommObject::SendPkt() Failed Tx");
    }
    
    if(nSent!=PktTx.GetStreamLength())
    {                
        throw CMOOSException("CMOOSCommObject::SendPkt() Failed Tx");
    }

    return true;
}

bool CMOOSCommObject::SendMsg(XPCTcpSocket *pSocket,CMOOSMsg &Msg)
{
    MOOSMSG_LIST MsgList;
    
    MsgList.push_front(Msg);

    CMOOSCommPkt Pkt;

    Pkt.Serialize(MsgList,true);
	

    return SendPkt(pSocket,Pkt);
}


bool CMOOSCommObject::ReadMsg(XPCTcpSocket *pSocket,CMOOSMsg &Msg, int nSecondsTimeout)
{
    MOOSMSG_LIST MsgList;
    
    CMOOSCommPkt Pkt;

    if(ReadPkt(pSocket,Pkt,nSecondsTimeout))
    {

        Pkt.Serialize(MsgList,false);

        if(!MsgList.empty())
        {
            Msg = MsgList.front();
        }
        
    }


    return !MsgList.empty();
}


bool CMOOSCommObject::SocketsInit()
{

#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD( 2, 2 );
 
    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */

        printf("failed socket init\n");
        return false;
    }
 
    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */
 
    if ( LOBYTE( wsaData.wVersion ) != 2 ||
            HIBYTE( wsaData.wVersion ) != 2 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                     
        */

        printf("failed socket init\n");

        WSACleanup( );
        return false; 
    }
#endif
    return true;
}
