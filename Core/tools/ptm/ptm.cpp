#include "MOOS/libMOOS/App/MOOSApp.h"

//this is a trivial commandline interface which sends a single string
//to a moos community. Syntax is
//     ./ptm var_name@app_name string_to_send
// or interactively reading input from stdin:
//     ./ptm var_name@app_name 
//
// Example : send the string "lala" uner the name of x and make it look like it
// came from a app called foo.
//     ./ptm x@foo lala 
// or of course using piping on the command line
//     echo lala | ./ptm x@foo

struct PokeDetails{
    std::string var_name;
    std::string app_name;
    std::string string_to_send;

    std::string to_string(){
        std::stringstream ss;
        ss<<"payload       : "<<string_to_send<<"\n";        
        ss<<"under app name: "<<app_name<<"\n";
        ss<<"as var name   : "<<var_name<<"\n";
        return ss.str();
    }

    bool from_string(std::string in, PokeDetails & details ){//clean up
        //format is x@foo
        MOOSTrimWhiteSpace(in);
        MOOSRemoveChars(in," \t");
        var_name = MOOSChomp(in,"@");
        app_name = in;    
        return !details.app_name.empty() && !details.var_name.empty();
    }
};
    
class PokeTheMOOS :  public CMOOSApp
{
public:
    PokeTheMOOS(const PokeDetails & poke_details):poke_details_(poke_details) {
        m_Comms.SetQuiet(true);
        SetQuiet(true);
    }

    bool OnConnectToServer(){return true;}

    bool OnStartUp(){
        SetAppFreq(2.0);
        EnableIterateWithoutComms(true);
        m_bQuitOnIterateFail = true;
        return true;
    }

    bool Iterate(){
        if(m_Comms.IsConnected()){
            Notify(poke_details_.var_name,poke_details_.string_to_send);
            m_Comms.Flush();
            return false; // only iterate once
        }
        return true;
    }

    PokeDetails poke_details_;
};

int main(int argc , char* argv[])
{
    //here we do some command line parsing...
    MOOS::CommandLineParser P(argc,argv);

    std::string route_details_string= P.GetFreeParameter(0, "");
    PokeDetails details;
    if(!details.from_string(route_details_string,details)){
        std::cerr<<"failed to parse route details in form var_name@app_name\n";
        exit(-1);
    }
    
    details.string_to_send = P.GetFreeParameter(1, "");
    if(details.string_to_send.empty()){
        //ok read interqactively or from a pipe to std::cin
        std::getline(std::cin, details.string_to_send);
    }

    std::cerr<<"ptm will wait to connect and then send :\n";
    std::cerr<<details.to_string();
    PokeTheMOOS ptm(details);
    ptm.Run(details.app_name,argc,argv);
}
