/*
 * ActiveMailQueue.h
 *
 *  Created on: Dec 6, 2012
 *      Author: pnewman
 */

#ifndef ACTIVEMAILQUEUE_H_
#define ACTIVEMAILQUEUE_H_
#include "MOOS/libMOOS/Comms/MOOSMsg.h"
#include "MOOS/libMOOS/Utils/MOOSThread.h"
#include "MOOS/libMOOS/Utils/SafeList.h"

/*
 *
 */
namespace MOOS {

class ActiveMailQueue {
public:
	ActiveMailQueue(const std::string &sName);
	virtual ~ActiveMailQueue();
	bool Push(const CMOOSMsg & M);
    void SetCallback(bool (*pfn)(CMOOSMsg &M, void * pParamCaller), void * pCallerParam);
    bool DoWork();
    bool Stop();
    bool Start();
    std::string GetName();
protected:
	MOOS::SafeList<CMOOSMsg> queue_;

    /** the user supplied Callback*/
    bool (*pfn_)(CMOOSMsg &M, void* pParam);
    void * caller_param_;

    CMOOSThread thread_;

    //this is a mick name for the Queue
    std::string Name_;

};

}

#endif /* ACTIVEMAILQUEUE_H_ */
