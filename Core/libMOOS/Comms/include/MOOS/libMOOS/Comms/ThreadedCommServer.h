/*
 * ThreadedCommServer.h
 *
 *  Created on: Aug 29, 2011
 *      Author: pnewman
 */

#ifndef THREADEDCOMMSERVER_H_
#define THREADEDCOMMSERVER_H_


#include "MOOS/libMOOS/Comms/MOOSCommServer.h"
#include "MOOS/libMOOS/Utils/SafeList.h"


namespace MOOS
{


struct ClientThreadSharedData
{
  ClientThreadSharedData(const std::string & sN, CMOOSCommPkt * pPktRx, CMOOSCommPkt * pPktTx ):
      _sClientName(sN),_pPktRx(pPktRx),_pPktTx(pPktTx){}

  ClientThreadSharedData(){_pPktRx = NULL; _pPktTx = NULL;_Status =NOT_INITIALISED; };

  //payload(s)
  std::string _sClientName;
  CMOOSCommPkt * _pPktRx;
  CMOOSCommPkt * _pPktTx;

  //little bit of status
  enum Status
  {
      NOT_INITIALISED,
      PKT_READ,
      CONNECTION_CLOSED,
      PKT_WRITE,
  } _Status;

};


class ThreadedCommServer : public CMOOSCommServer
{
public:
    ThreadedCommServer();
    virtual ~ThreadedCommServer();

private:
    typedef CMOOSCommServer BASE;


protected:
    typedef SafeList<ClientThreadSharedData> SHARED_PKT_LIST;


    /**
     * This is a class which handles the Reading and Writing of fully formed MOOSCommPkts
     * It ceaselessly looks to read from a socket - when a Pkt is formed it stuff it on a shared thread safe
     * list. It then waits for a reply pkt to be placed in _SharedDataOutgoing via a call to
     * ::SendToClient()
     */
    class ClientThread : public CMOOSCommObject
    {
    public:
        virtual ~ClientThread();
        /**
         *
         * @param sName name of client this object is encapsulating
         * @param ClientSocket reference to a socket at the other end of which is the client
         * @param SharedData a sae list of ClientThreadSharedData object
         * @return
         */
        ClientThread(const std::string & sName, XPCTcpSocket & ClientSocket,SHARED_PKT_LIST & SharedDataIncoming );


        /**
         * standard trick to get around c++ issue an threads
         * @param pParam (a cunningly disguised instance pointer
         * @return
         */
        static bool RunEntry(void * pParam) {  return  ( (ClientThread*)pParam) -> Run();}

        /**
         * here is the main business of the day - this does the reading and writing in turn
         * @return should not return unless socket closes..
         */
        bool Run();

        /**
         * used to push data to client to send...
         * @param OutGoing an object which was orginally collected from _SharedDataIncoming
         * @return tru on success
         */
        bool SendToClient(ClientThreadSharedData & OutGoing);


        bool HandleClient();

        bool OnClientDisconnect();

        XPCTcpSocket & GetSocket(){return _ClientSocket;};


        bool Kill();

        bool Start();

    protected:


        //a thread object which will start the Run() method
        CMOOSThread _Worker;


        //what is the name of the client we are representing?
        std::string _sClientName;

        //we are simply given a reference to the socket via which the client is talking at constr
        XPCTcpSocket & _ClientSocket;

        //note that this a reference to a list given to us (we don't own it) at construction
        ThreadedCommServer::SHARED_PKT_LIST &  _SharedDataIncoming;

        //note that this one we own - its private to us
        ThreadedCommServer::SHARED_PKT_LIST _SharedDataOutgoing;


    };


    /** Called when a new client connects. Performs handshaking and adds new socket to m_ClientSocketList
    @param pNewClient pointer to the new socket created in ListenLoop;
    @see ListenLoop*/
    virtual bool OnNewClient(XPCTcpSocket * pNewClient,char * sName);

    virtual bool OnClientDisconnect(ClientThreadSharedData &SD);
    virtual bool OnClientDisconnect();

    virtual bool ServerLoop();

    virtual bool TimerLoop();

    virtual bool AddAndStartClientThread(XPCTcpSocket & NewClientSocket,const std::string & sName);

    virtual bool ProcessClient(ClientThreadSharedData &SD);
    virtual bool ProcessClient();

    bool StopAndCleanUpClientThread(std::string sName);




    //all connected clients will push the received Pkts into this list....
    SafeList<ClientThreadSharedData> m_SharedDataListFromClient;

    std::map<std::string,ClientThread*> m_ClientThreads;

};

}

#endif /* THREADEDCOMMSERVER_H_ */
