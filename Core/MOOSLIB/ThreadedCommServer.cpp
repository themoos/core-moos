/*
 * ThreadedCommServer.cpp
 *
 *  Created on: Aug 29, 2011
 *      Author: pnewman
 */

#include "MOOSLIB/ThreadedCommServer.h"
#include "MOOSLIB/MOOSException.h"
#include "XPCTcpSocket.h"



#ifdef _WIN32
#define INVALID_SOCKET_SELECT WSAEINVAL
#else
#define INVALID_SOCKET_SELECT EBADF
#endif


namespace MOOS
{





ThreadedCommServer::ThreadedCommServer()
{
    // TODO Auto-generated constructor stub

}

ThreadedCommServer::~ThreadedCommServer()
{
    // TODO Auto-generated destructor stub
}


/**
 * called when a new client connects. Our job is to make a new thread which will handle
 * all communications with the client. This will stop slow comms slowing everything up
 * @param pNewClient
 * @param sName
 * @return
 */
bool ThreadedCommServer::OnNewClient(XPCTcpSocket * pNewClient,char * sName)
{
    if(!BASE::OnNewClient(pNewClient,sName))
    {
        return false;
    }

    //here we need to start a new thread to handle our comms...
    return AddAndStartClientThread(*pNewClient,sName);

}

/**
 * The mechanics of starting a new thread to handle client communications
 * @param pNewClient
 * @param sName
 * @return
 */
bool ThreadedCommServer::AddAndStartClientThread(XPCTcpSocket & NewClientSocket,const std::string & sName)
{
    std::map<std::string,ClientThread*>::iterator q = m_ClientThreads.find(sName);

    if(q == m_ClientThreads.end() )
    {
        ClientThread* pExistingClientThread = q->second;
        MOOSTrace("worrying condition ::AddAndStartClientThread() killing an existing client thread");

        //stop the thread
        pExistingClientThread->Kill();

        //free up the heap (old skool - good grief)
        delete pExistingClientThread;

        //remove from map
        m_ClientThreads.erase(q);
    }

    //make a new client - and show it where to put data and how to talk to a client
    ClientThread* pNewClientThread =  new  ClientThread(sName,NewClientSocket,m_SharedDataListFromClient);

    //add to map
    m_ClientThreads[sName] = pNewClientThread;

    pNewClientThread->Start();


}

/**
 * This is the main loop - it looks for complete Pkt being placed in the incoming list
 * and invokes a handler
 * @return true on exit
 */
bool ThreadedCommServer::ServerLoop()
{

    //eternally look at our incoming work list....
    while(1)
    {
        ClientThreadSharedData SD;

        m_SharedDataListFromClient.WaitForPush();
        m_SharedDataListFromClient.Pull(SD);

        switch(SD._Status)
        {
        case ClientThreadSharedData::PKT_READ:
            ProcessClient(SD);
            break;

        case ClientThreadSharedData::CONNECTION_CLOSED:
            break;

        default:
            break;
        }

    }

    return true;

}


/**
 * the main handler  - a Pkt has been fetch off the work list (and is in SD)
 * we now invoke a callback and then place the return packet in the
 * same basket
 * @param SD
 * @return
 */
bool ThreadedCommServer::ProcessClient(ClientThreadSharedData &SD)
{
    bool bResult = true;

    try
    {

        //now we act on that packet
        //by way of the user supplied called back
        if(m_pfnRxCallBack!=NULL)
        {

            MOOSMSG_LIST MsgLstRx,MsgLstTx;

            //convert to list of messages
            SD._pPktRx->Serialize(MsgLstRx,false);

            std::string sWho = SD._sClientName;

            //let owner figure out what to do !
            //this is a user supplied call back

            if(m_bQuiet)
                InhibitMOOSTraceInThisThread(false);

            if(!(*m_pfnRxCallBack)(sWho,MsgLstRx,MsgLstTx,m_pRxCallBackParam))
            {
                //client call back failed!!
                MOOSTrace(" CMOOSCommServer::ProcessClient()  pfnCallback failed\n");
            }

            if(m_bQuiet)
                InhibitMOOSTraceInThisThread(true);



            //every packet will no begin with a NULL message the double val
            //of which will be the current time on the DB's machine
            if( 1 || MsgLstTx.size()==0)
            {
                //add a default packet so client doesn't block
                CMOOSMsg NullMsg;
                NullMsg.m_dfVal = MOOSLocalTime();
                MsgLstTx.push_front(NullMsg);
            }

            //stuff reply message into a packet
            SD._pPktTx->Serialize(MsgLstTx,true);

            //send packet back to client...

            //first find the client object
            std::map<std::string,ClientThread*>::iterator q = m_ClientThreads.find(sWho);
            if(q == m_ClientThreads.end())
            {
                MOOSTrace("logical error - FIX ME!");
            }

            ClientThread* pClient = m_ClientThreads[sWho];
            SD._Status = ClientThreadSharedData::PKT_WRITE;

            //add it to the work load
            pClient->SendToClient(SD);


        }
    }
    catch(CMOOSException e)
    {
        MOOSTrace("ProcessClient() Exception: %s\n", e.m_sReason);
        bResult = false;
    }

    return bResult;

}


bool ThreadedCommServer::ClientThread::Run()
{


    //ignore broken pipes as is standard for network apps
#ifndef _WIN32
    signal(SIGPIPE,SIG_IGN);
#endif


    struct timeval timeout;        // The timeout value for the select system call
    fd_set fdset;                // Set of "watched" file descriptors

    while(!_Worker.IsQuitRequested())
    {

        // The socket file descriptor set is cleared and the socket file
        // descriptor contained within tcpSocket is added to the file
        // descriptor set.
        FD_ZERO(&fdset);
        FD_SET(_ClientSocket.iGetSocketFd(), &fdset);

        // The select system call is set to timeout after 1 seconds with no data existing
        // on the socket. This reinitialisation has to be here, within the loop as Linux actually writes over
        // the timeout structure on completion of select (now that was a hard bug to find)
        timeout.tv_sec    = 1;
        timeout.tv_usec = 0;



        // A select is setup to return when data is available on the socket
        // for reading.  If data is not available after 1000 useconds, select
        // returns with a value of 0.  If data is available on the socket,
        // the select returns and data can be retrieved off the socket.
        int iSelectRet = select(_ClientSocket.iGetSocketFd() + 1,
            &fdset,
            NULL,
            NULL,
            &timeout);

        // If select returns a -1, then it failed and the thread exits.
        switch(iSelectRet)
        {
        case -1:
            if(XPCSocket::iGetLastError()==INVALID_SOCKET_SELECT)
            {
                //this can be caused by absenteeism between set up of fdset and select
                //prefer to catch and tolerate than block other threads for duration
                //of select and processing - added by PMN in Jan 2008 to address a
                //race condition which took a long time to show up...
                break;
            }
            else
            {
                return false;
            }

        case 0:
            //timeout...nothing to read - spin
            break;

        default:
            //something to read (somewhere)
            if (FD_ISSET(_ClientSocket.iGetSocketFd(), &fdset) != 0)
            {
                if(!HandleClient())
                {
                    //client disconnected!
                    OnClientDisconnect();
                    break;
                }
            }
            else
            {
                //this is strange and unexpected.....
                MOOSTrace("unexpected logical condition");
            }
            break;

        }

        //zero socket set..
        FD_ZERO(&fdset);

    }
    return 0;
}

bool ThreadedCommServer::ClientThread::OnClientDisconnect()
{
    MOOSTrace("CMOOSCommServer::ClientThread::OnClientDisconnect() -? Write me...");
    return true;
}


bool ThreadedCommServer::ClientThread::Kill()
{
    return _Worker.Stop();
}

bool ThreadedCommServer::ClientThread::HandleClient()
{
    bool bResult = true;



    try
    {
        _ClientSocket.SetReadTime(MOOSTime());

        CMOOSCommPkt PktRx,PktTx;

        //read input
        ReadPkt(&_ClientSocket,PktRx);

        //prepare to send it up the chain
        ClientThreadSharedData SD(_sClientName, &PktRx,&PktTx);
        SD._Status = ClientThreadSharedData::PKT_READ;

        //push this data back to the central thread
        _SharedDataIncoming.Push(SD);

        //wait for data to be returned...
        _SharedDataOutgoing.WaitForPush();
        _SharedDataOutgoing.Pull(SD);

        if(SD._Status!=ClientThreadSharedData::PKT_WRITE)
        {
            MOOSTrace("logical error %s", MOOSHERE);
            return false;
        }

        //send packet to client
        SendPkt(&_ClientSocket,*SD._pPktTx);
    }
    catch (CMOOSException e)
    {
        MOOSTrace("CMOOSCommServer::ClientThread::HandleClient() Exception: %s\n", e.m_sReason);
        bResult = false;
    }

    return bResult;





}




}
