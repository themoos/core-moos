
#ifndef MOOSAsyncCommClientH
#define MOOSAsyncCommClientH

#include <map>
#include "MOOS/libMOOS/Comms/MOOSCommClient.h"
#include "MOOS/libMOOS/Utils/MOOSThread.h"
#include "MOOS/libMOOS/Utils/SafeList.h"
#include "MOOS/libMOOS/Comms/ActiveMailQueue.h"

namespace MOOS
{
class ActiveMailQueue;

	class MOOSAsyncCommClient : public CMOOSCommClient
	{
		//you are unlikely to wa
	public:
		typedef CMOOSCommClient BASE;

		/**
		 * default constructor
		 */
		MOOSAsyncCommClient();
		virtual ~MOOSAsyncCommClient();

		/**
		 * Close the client. This is blocking call.
		 * @param Nice (not used -legacy)
		 * @return true on succes
		 */
		virtual bool Close(bool Nice = true );

		/**
		 * Send a single MOOSMsg
		 * @param Msg
		 * @param bKeepMsgSourceName
		 * @return
		 */
	    virtual bool Post(CMOOSMsg & Msg,bool bKeepMsgSourceName=false);


	    /**
	     * Is client running
	     * @return true if it is
	     */
	    virtual bool IsRunning();

	    /**
	     * Flush all unsent data. Does nothing as data is always sent ASAP
	     * @return true on success
	     */
	    virtual bool Flush();

	    /**
	     * Is this an Asynchronous Client?
	     * @return
	     */
	    virtual bool IsAsynchronous();

	    /**
		 * Register a custom call back for a particular message. This call back will be called from its own thread.
		 * @param sMsgName name of message to watch for
		 * @param pfn  pointer to your function should be type bool func(CMOOSMsg &M, void *pParam)
		 * @param pYourParam a void * pointer to the thing we want passed as pParam above
		 * @return true on success
		 */
	    bool AddMessageCallback(const std::string & sMsgName, bool (*pfn)(CMOOSMsg &M, void * pYourParam), void * pYourParam );

		//some thread workers which need to be public so threads can run them
	    //you won't be calling these yourself.
	    bool ReadingLoop();
	    bool WritingLoop();


	protected:

	    /** Called internally when connection needs to be closed*/
	    virtual bool OnCloseConnection();

	    /**
	     * start all the workr threads
	     * @return true on success
	     */
		virtual bool StartThreads();

		/**
		 * Print a banner describing client
		 */
	    virtual void DoBanner();

	    /**
	     * make sure the writing is not happening too fast.
	     * This is an internal function and is used toprevent
	     * rogue users hurting others by swamping the network
	     * @return true on success
	     */
	    bool MonitorAndLimitWriteSpeed();

	    virtual std::string HandShakeKey();

	    /**
	     * perform the management of the incoming data (called internally)
	     * @return
	     */
	    bool DoReading();

	    /**
	     * perform the management of the outgoing data
	     * @return
	     */
	    bool DoWriting();


	    //data members below here
	    CMOOSThread WritingThread_; //handles writing
	    CMOOSThread ReadingThread_; //handles reading

	    double m_dfLastTimingMessage; //time last timing messae was sent
	    double m_dfLastSendTime; 	//time last message was sent
	    unsigned int m_nOverSpeedCount;

	    MOOS::SafeList<CMOOSMsg> OutGoingQueue_; //queue of outgoing mail

	    /**
	     * list of active mail queues. Each Queue invokes a callback. Keyed by message name
	     */
	    std::map<std::string,ActiveMailQueue*  > ActiveQueues_;

	};
};

#endif
