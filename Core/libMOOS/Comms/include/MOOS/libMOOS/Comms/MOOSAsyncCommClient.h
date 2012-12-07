
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

	public:
		typedef CMOOSCommClient BASE;
		MOOSAsyncCommClient();
		virtual ~MOOSAsyncCommClient();
		virtual bool StartThreads();

		virtual bool Close(bool Nice = true );

	    bool ReadingLoop();

	    bool WritingLoop();

	    virtual bool Post(CMOOSMsg & Msg,bool bKeepMsgSourceName=false);

	    virtual bool OnCloseConnection();

	    virtual bool IsRunning();

	    virtual bool Flush();

	    virtual bool IsAsynchronous();

	    bool AddActiveCallBack(const std::string & sMsgName, bool (*pfn)(CMOOSMsg &M, void * pYourParam), void * pYourParam );

	protected:

	    virtual void DoBanner();

	    bool MonitorAndLimitWriteSpeed();

	    virtual std::string HandShakeKey();

	    bool DoReading();

	    bool DoWriting();

	    CMOOSThread WritingThread_;
	    CMOOSThread ReadingThread_;

	    double m_dfLastTimingMessage;
	    double m_dfLastSendTime;
	    unsigned int m_nOverSpeedCount;

	    MOOS::SafeList<CMOOSMsg> OutGoingQueue_;

	    std::map<std::string,ActiveMailQueue*  > ActiveQueues_;

	};
};

#endif
