
#ifndef MOOSAsyncCommClientH
#define MOOSAsyncCommClientH


#include "MOOS/libMOOS/Comms/MOOSCommClient.h"
#include "MOOS/libMOOS/Utils/MOOSThread.h"
#include "MOOS/libMOOS/Utils/SafeList.h"

namespace MOOS
{
	class MOOSAsyncCommClient : public CMOOSCommClient
	{

	public:
		typedef CMOOSCommClient BASE;
		MOOSAsyncCommClient();
		virtual ~MOOSAsyncCommClient();
		virtual bool StartThreads();
	    bool WritingLoop();
	    bool DoWriting();

	    bool ReadingLoop();
	    bool DoReading();

	    virtual std::string HandShakeKey();

	    virtual bool Post(CMOOSMsg & Msg);

	    virtual bool OnCloseConnection();

	    virtual void DoBanner();

	    virtual bool IsRunning();

	    virtual bool Flush();

	protected:


	    CMOOSThread WritingThread_;
	    CMOOSThread ReadingThread_;

	    double m_dfLastTimingMessage;

	    /** Mutex around Close Connection - two threads could call it
	     *  method
	    @see CMOOSLock
	    */
	    CMOOSLock m_CloseConnectionLock;

	    MOOS::SafeList<CMOOSMsg> OutGoingQueue_;


	};
};

#endif
