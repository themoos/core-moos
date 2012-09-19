
#ifndef MOOSAsyncCommClientH
#define MOOSAsyncCommClientH


#include "MOOS/libMOOS/Comms/MOOSCommClient.h"
#include "MOOS/libMOOS/Utils/MOOSThread.h"

namespace MOOS
{
	class MOOSAsyncCommClient : public CMOOSCommClient
	{

	public:
		virtual bool StartThreads();
	    bool WritingLoop();
	    bool DoWriting();

	    bool ReadingLoop();
	    bool DoReading();

	    CMOOSThread WritingThread_;
	    CMOOSThread ReadingThread_;

	};
}

#endif
