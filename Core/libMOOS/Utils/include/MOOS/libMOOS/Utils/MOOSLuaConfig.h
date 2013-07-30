///////////////////////////////////////////////////////////////////////////
//
//   This file is part of the MOOS project
//
//   MOOS : Mission Oriented Operating Suite A suit of
//   Applications and Libraries for Mobile Robotics Research
//   Copyright (C) Paul Newman
//
//   This software was written by Paul Newman at MIT 2001-2002 and
//   the University of Oxford 2003-2013
//
//   email: pnewman@robots.ox.ac.uk.
//
//   This file was written by Ian Katz at MIT 2010 (ijk5@mit.edu)
//
//   This source code and the accompanying materials
//   are made available under the terms of the GNU Lesser Public License v2.1
//   which accompanies this distribution, and is available at
//   http://www.gnu.org/licenses/lgpl.txtgram is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
////////////////////////////////////////////////////////////////////////////
//
// MOOSLuaConfig.h - interface for the CMOOSLuaConfig class
//
/////////////////////////////////////


#if !defined(AFX_MOOSLUACONFIG_H_INCLUDED_)
#define AFX_MOOSLUACONFIG_H_INCLUDED_


#include "MOOSLuaEnvironment.h"
#include <lua5.1/lua.hpp>
#include <string>


using namespace std;


class CMOOSLuaConfig : public CMOOSLuaEnvironment
{
public:

    CMOOSLuaConfig(std::string sIdentifier,
                   LUA_API_MAP api, 
                   std::string sLuaLibDir);

    
    virtual ~CMOOSLuaConfig() {};


    // 3 functions to get data in the style of the mission config reader
    bool GetConfiguration(std::string sAppName,
                          STRING_LIST &Params);
    
    bool GetConfigurationParam(std::string sAppName,
                               std::string sParam,
                               std::string &sVal);
    
    bool GetValue(std::string sName, std::string &sResult);
    

protected:

    //print an error to the console
    static void Lua_OnErr(string source, string message)
    {
        MOOSTrace("Lua Error from %s:\n    %s\n",
                  source.c_str(), message.c_str());
    }

    
    std::string myToString(int idx);

};

#endif // !defined(AFX_MOOSLUACONFIG_H_INCLUDED_)
