/*
 * DBTestClient.cpp
 *
 *  Created on: Sep 19, 2011
 *      Author: pnewman
 */
#include <iostream>
#include "MOOS/libMOOS/App/MOOSApp.h"
#include "MOOS/libMOOS/Thirdparty/getpot/getpot.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"
#include <queue>

void PrintHelp()
{

    MOOSTrace("\n\nGeneral MOOS settings:\n");
    MOOSTrace("  -n (--name)                       : MOOS name\n");
    MOOSTrace("  -a (--apptick)                    : MOOSAppTick in Hz\n");
    MOOSTrace("  -c (--commstick)                  : MOOSCommsTick in Hz\n");
    MOOSTrace("  -s var1 [var2,var3...]            : list of subscriptions in form var_name@period eg -s x y z\n");
    MOOSTrace("  -p var1@t1 [var2@t2,var3@t3....]  : list of publciations in form var_name@period eg x@0.5 y@2.0\n");

    MOOSTrace("\n\nNetwork failure simulation:\n");
    MOOSTrace("  -N (--simulate_network_failure)   : enable simulation of network/app failure\n");
    MOOSTrace("  -P (--network_failure_prob)       : probability of each DB interaction having network failure [0.1]\n");
    MOOSTrace("  -t (--network_failure_time)       : duration of network failure [3s]\n");
    MOOSTrace("  -k (--application_failure_prob)   : probability of application failing during DB-communication [0]\n");

    MOOSTrace("\n\nExample Usage:\n");
    MOOSTrace("  ./uDBTestClient --name C1 -s x -p y@0.5 z@2.0 --apptick 10 --commstick 20 \n");
    MOOSTrace("  ./uDBTestClient --name C2 -s z --simulate_network_failure -P 0.05 -t 10.0 --application_failure_prob 0.05 \n");


}

class DBTestClient : public CMOOSApp
{
public:
    DBTestClient(){};
    DBTestClient(int argc, char* argv[])
    {
        GetPot cl(argc,argv);


        _AppTick = cl.follow(10.0,2,"-a","--apptick");
        _CommsTick = cl.follow(10.0,2,"-c","--commstick");


        _vSubscribe = cl.nominus_followers("-s");
        std::vector<std::string> vPublish = cl.nominus_followers("-p");

        _SimulateNetworkFailure = cl.search(2,"-N","--simulate_network_failure");
        _NetworkStallProb = cl.follow(0.1,2,"-P","--network_failure_prob");
        _NetworkStallTime = cl.follow(3.0,2,"-t","--network_failure_time");
        _ApplicationExitProb = cl.follow(0.0,2,"-k","--application_failure_prob");


        if(cl.search(2,"-h","--help"))
        {
            PrintHelp();
            exit(0);
        }



        std::vector<std::string>::iterator q;

        for(q = vPublish.begin();q!=vPublish.end();q++)
        {
            std::string sEntry  = *q;
            std::string sName = MOOSChomp(sEntry,"@");
            if (!MOOSIsNumeric(sEntry) || sEntry.empty())
            {
                std::cerr<<MOOS::ConsoleColours::red()<<"badly formatted publish directive name1@period1 name2@period 2\n"<<MOOS::ConsoleColours::reset();
                exit(1);
            }
            double dfPeriod = 1.0/atof(sEntry.c_str());

            _Jobs.push(Job(dfPeriod,sName));
            std::cerr<<MOOS::ConsoleColours::green()<<"adding job: publishing "<<sName<<" every "<<dfPeriod<<" seconds\n"<< MOOS::ConsoleColours::reset();

        }

    }

    bool OnStartUp()
    {
        SetAppFreq(_AppTick);
        SetCommsFreq(_CommsTick);
        if(_SimulateNetworkFailure)
        {
            MOOSTrace("%f %f\n\n",_NetworkStallProb,_NetworkStallTime);
            m_Comms.ConfigureCommsTesting(_NetworkStallProb,_NetworkStallTime,_ApplicationExitProb);
        }

        return true;
    }
    bool OnNewMail(MOOSMSG_LIST & NewMail)
    {
        MOOSMSG_LIST::iterator q;
        for(q = NewMail.begin();q!=NewMail.end();q++)
        {
            std::cerr<<MOOS::ConsoleColours::cyan()<<"received: "<<q->GetKey()<<"="<<q->GetDouble()<<"\n";
        }
        std::cerr<<MOOS::ConsoleColours::reset();

        return true;
    }
    bool Iterate()
    {
        while(_Jobs.size() && _Jobs.top().isActive())
        {
            Job Active = _Jobs.top();
            _Jobs.pop();
            m_Comms.Notify(Active._sName,Active._nCount,MOOSTime());
            std::cerr<<MOOS::ConsoleColours::Yellow()<<"publishing: "<<Active._sName<<"="<<Active._nCount<<std::endl<<MOOS::ConsoleColours::reset();
            Active.Reschedule();
            _Jobs.push(Active);
        }


        return true;
    }
    bool OnConnectToServer()
    {
        DoSubscriptions();
        return true;
    }

    bool DoSubscriptions()
    {
        std::vector<std::string>::iterator q;

        std::cerr<<MOOS::ConsoleColours::Green()<<"\nsubscribing\n";
        for(q = _vSubscribe.begin();q!=_vSubscribe.end();q++)
        {
            m_Comms.Register(*q,0.0);
            std::cerr<<"  "<<*q<<"\n";
        }
        std::cerr<<MOOS::ConsoleColours::reset();

        return true;
    }



private:
    std::vector<std::string> _vSubscribe;
    double _AppTick;
    double _CommsTick;

    bool _SimulateNetworkFailure ;
    double _NetworkStallProb ;
    double _NetworkStallTime ;
    double _ApplicationExitProb;

    struct Job
    {
        Job(double dfPeriod, std::string sName):_dfPeriod(dfPeriod),_sName(sName),_nCount(0)
        {
            _dfTimeScheduled = MOOSTime()+_dfPeriod;
        }

        bool operator < (const  Job &  a) const
        {
            return _dfTimeScheduled> a._dfTimeScheduled;
        }

        void Reschedule()
        {
            _nCount++;
            _dfTimeScheduled = MOOSTime()+_dfPeriod;
        }

        bool isActive() const
        {
            return _dfTimeScheduled < MOOSTime();
        }

        double _dfPeriod;
        double _dfTimeScheduled;
        std::string _sName;
        int _nCount;
    };
    std::priority_queue<Job> _Jobs;



};

int main (int argc, char* argv[])
{
    GetPot cl(argc,argv);
    std::string sClientName = cl.follow("TC1",2,"-n","--name");

    DBTestClient TC1(argc,argv);

    TC1.Run(sClientName.c_str(),"Mission.moos");
}
