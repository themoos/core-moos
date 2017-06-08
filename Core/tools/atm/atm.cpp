/*
 * ktm.cpp
 *
 *  Created on: Feb 6, 2014
 *      Author: pnewman
 */

#include <string>
#include <cstring>
#include <stdexcept>
#include <iomanip>

#include "MOOS/libMOOS/Utils/CommandLineParser.h"
#include "MOOS/libMOOS/Utils/ThreadPrint.h"
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"
#include "MOOS/libMOOS/Comms/MulticastNode.h"
#include "MOOS/libMOOS/Comms/EndToEndAudit.h"

const int kFlusheveryN = 1;
const int64_t kDefaultIgnoreDelayBelow=3000;

const std::string kDefaultLogFile = "default.moos_audit.log";
const std::string kDefaultAuditDirectory = "moos_comms_audits";

std::string multicast_address = "224.1.1.8";
int multicast_port = 4000;
std::string name_pattern;
void PrintHelpAndExit()
{
    std::cerr<<"audit the MOOS (atm)\n";
    std::cerr<<"--channel=<address>     address to listen  for audit comms on \n";
    std::cerr<<"--port=<int>            port to listen on \n";
    std::cerr<<"--ignore_delay_below_us ignore delays less than this in uS\n";
    std::cerr<<"--log                   log statistics\n";
    std::cerr<<"--log_directory=<dir>   log directory";
    std::cerr<<"--log_name=<name>       log file name";

    exit(0);
}


void MakeLoggingDirectory(const std::string & logging_directory_name){
    if(!MOOSCreateDirectory(logging_directory_name)){
        std::cerr<<"failed to create audit directory "<<logging_directory_name<<"\n";
        exit(-1);
    }
}

void OpenLogFile(std::ofstream & ofs,
                 const std::string & logging_directory,
                 std::string & logfile_name){
    MakeLoggingDirectory(logging_directory);

    std::string log_file_path;
    if(logfile_name.empty()){
        log_file_path = logging_directory+"/"+MOOSGetTimeStampString()+".moos_audit.log";
    }else{
        log_file_path = logging_directory+"/"+logfile_name;
    }
    ofs.open(log_file_path.c_str(),std::ios::ate | std::ios::out);
    if(!ofs){
        std::cerr<<"failed to open log file "<<log_file_path<<"\n";
        exit(-1);
    }

    std::cerr<<"appending audit data to to "<<log_file_path<<"\n";
}


void PrettyPrint(std::ostream & output_stream,const MOOS::EndToEndAudit::MessageStatistic & ms){
    output_stream<<std::left<<std::setw(7)<<std::setprecision(4);
    output_stream<<(ms.receive_time-ms.source_time)/1000.0<<" ms delay ";
    output_stream<<ms.source_client<<" |----  "<<ms.message_name<<"["<<ms.message_size<<"]  ----> "<<ms.destination_client<<std::endl;
}

void LogToFile(std::ofstream & output_stream,const MOOS::EndToEndAudit::MessageStatistic & ms){

    output_stream<<std::left<<std::setw(15);
    output_stream<<ms.receive_time-ms.source_time<<",";

    output_stream<<std::left<<std::setw(15);
    output_stream<<ms.message_name<<",";

    output_stream<<std::left<<std::setw(15);
    output_stream<<ms.source_client<<",";

    output_stream<<std::left<<std::setw(15);
    output_stream<<ms.destination_client<<",";

    output_stream<<std::left<<std::setw(20);
    output_stream<<ms.receive_time<<",";

    output_stream<<std::left<<std::setw(15);
    output_stream<<ms.cpu_load<<"\n";

    static int lines_logged = 0;
    if(lines_logged++%kFlusheveryN==0){
        output_stream.flush();
        std::cerr<<"\r"<<std::left<<std::setw(15)
                <<lines_logged<<" audit records written";
    }

}


void WriteHeader(std::ofstream & output_stream){
    output_stream<<std::left<<std::setw(15);
    output_stream<<"%delay(uS)"<<",";

    output_stream<<std::left<<std::setw(15);
    output_stream<<"name"<<",";

    output_stream<<std::left<<std::setw(15);
    output_stream<<"src"<<",";

    output_stream<<std::left<<std::setw(15);
    output_stream<<"dest"<<",";

    output_stream<<std::left<<std::setw(20);
    output_stream<<"rx_time"<<",";

    output_stream<<std::left<<std::setw(15);
    output_stream<<"cpu_load"<<"\n";
}


bool ShouldBeLogged(const MOOS::EndToEndAudit::MessageStatistic & message_statistic,
                    int64_t ignore_delays_below){

    int64_t delta = message_statistic.receive_time-message_statistic.source_time;

    if(delta<ignore_delays_below)
        return false;

    return true;
}

int main(int argc, char *argv[])
{

    MOOS::CommandLineParser P(argc,argv);

    if(P.GetFlag("--help","-h"))
        PrintHelpAndExit();

    //sort out logging!
    std::string logging_directory = kDefaultAuditDirectory;
    std::string log_file_name=kDefaultLogFile;
    P.GetVariable("--log_directory",logging_directory);
    P.GetVariable("--log_name",log_file_name);
    bool logging = P.GetFlag("--log");
    std::ofstream output_stream;
    if(logging){
        OpenLogFile(output_stream,
                    logging_directory,
                    log_file_name);

        WriteHeader(output_stream);
    }


    P.GetVariable("--channel",multicast_address);
    P.GetVariable("--port",multicast_port);
    P.GetVariable("--name",name_pattern);

    int ignore_delay_below_us = kDefaultIgnoreDelayBelow;
    P.GetVariable("--ignore_delay_below_us",ignore_delay_below_us);

    MOOS::MulticastNode multicast_listener;
    multicast_listener.Configure(multicast_address,multicast_port,1);
    multicast_listener.Run(false,true);

    int number_messages_rxed = 0;
    int number_messages_logged = 0;
    double last_display_time = MOOSTime();
    double average_latency_us = 0;
    double alpha = 0.01;
    while(1)
    {
        std::string sReply;

        if(multicast_listener.Read(sReply,1000)){
            MOOS::EndToEndAudit::MessageStatistic ms;
            ms.FromString(sReply);

            number_messages_rxed++;
            int64_t latency = ms.receive_time-ms.source_time;
            average_latency_us=alpha*latency+(1.0-alpha)*average_latency_us;

            if(ShouldBeLogged(ms,ignore_delay_below_us)){
                if(logging){
                    LogToFile(output_stream,ms);
                    number_messages_logged++;
                }else{
                    PrettyPrint(std::cerr,ms);
                }
            }
        }

        if(MOOSTime()-last_display_time>10.0){
            last_display_time = MOOSTime();
            std::cerr<<MOOSFormat("processed %.8d messages and logged %.6d. "
                                  "Mean delay %.2fuS\n",
                                  number_messages_rxed,
                                  number_messages_logged,
                                  average_latency_us);
        }


    }

    return 0;


}

