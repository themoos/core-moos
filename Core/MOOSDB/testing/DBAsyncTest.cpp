/*
 * DBAsyncTest.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: pnewman
 */

#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"


int main(int argc, char * argv[])
{
	MOOS::MOOSAsyncCommClient C;

	C.Run("127.0.0.1",9000L,"test",10);

	while(1)
	{
		MOOSPause(1000);
	}
}
