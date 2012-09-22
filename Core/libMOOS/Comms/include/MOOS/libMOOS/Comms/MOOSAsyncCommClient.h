
#ifndef MOOSAsyncCommClientH
#define MOOSAsyncCommClientH


#include "MOOS/libMOOS/Comms/MOOSCommClient.h"
#include "MOOS/libMOOS/Utils/MOOSThread.h"

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


	    CMOOSThread WritingThread_;
	    CMOOSThread ReadingThread_;

	};
};

#endif
