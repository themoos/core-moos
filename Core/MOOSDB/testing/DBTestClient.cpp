/*
 * DBTestClient.cpp
 *
 *  Created on: Sep 19, 2011
 *      Author: pnewman
 */

#include "MOOSLIB/MOOSApp.h"

class DBTestClient : public CMOOSApp
{
    bool OnStartUp()
    {
        return true;
    }
    bool OnNewMail(MOOSMSG_LIST & NewMail)
    {
        return true;
    }
    bool Iterate()
    {
        return true;
    }
    bool OnConnectToServer()
    {
        return true;
    }
};

int main (int argc, char* argv[])
{
    DBTestClient TC1;
    TC1.Run("TC1","Mission.moos");
}
