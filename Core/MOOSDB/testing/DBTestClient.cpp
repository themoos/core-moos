/*
 * DBTestClient.cpp
 *
 *  Created on: Sep 19, 2011
 *      Author: pnewman
 */
#include <iostream>
#include "MOOSLIB/MOOSApp.h"
#include "MOOSThirdparty/GetPot/getpot"
#include "MOOSGenLib/ConsoleColours.h"
#include <queue>



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

        MOOSTrace("I have %d jobs\n",vPublish.size());

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
    }



private:
    std::vector<std::string> _vSubscribe;
    double _AppTick;
    double _CommsTick;

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
        int _nCount;
        std::string _sName;
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
