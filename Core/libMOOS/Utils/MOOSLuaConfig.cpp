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

#include "MOOS/libMOOS/Utils/MOOSLuaConfig.h"
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"

CMOOSLuaConfig::CMOOSLuaConfig(string sIdentifier,
                               LUA_API_MAP api, 
                               string sLuaLibDir)
    : CMOOSLuaEnvironment(sIdentifier, api, CMOOSLuaConfig::Lua_OnErr, sLuaLibDir) {}


bool CMOOSLuaConfig::GetConfiguration(std::string sAppName, STRING_LIST &Params)
{
    bool bGotRet = false;
    string sKey, sVal;

    //get var that we want
    lua_getglobal(m_luaState, sAppName.c_str());

    //make sure the requested section (table) was found
    if (LUA_TTABLE != lua_type(m_luaState, -1))
    {
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_pushnil(m_luaState); // push dummy key and iterate... based on common example

    while(lua_next(m_luaState, -2)) 
    {
        //avoid calling lua_tostring on a non-string key... messes up lua_next 
        if (LUA_TSTRING == lua_type(m_luaState, -2))
        {
            sKey = lua_tostring(m_luaState, -2);
            
            switch (lua_type(m_luaState, -1))
            {
                
            case LUA_TSTRING:
            case LUA_TNUMBER:
            case LUA_TBOOLEAN:
                sVal = myToString(-1);
                MOOSRemoveChars(sVal, " \t\r"); // emulate shitty behavior of original MOOS
                Params.push_front(sKey + "=" + sVal);
                break;

            case LUA_TTABLE:
                //return all entries under same key name
                lua_pushnil(m_luaState); //dummy key again

                while (lua_next(m_luaState, -2))
                {
                    sVal = myToString(-1);
                    MOOSRemoveChars(sVal," \t\r"); // emulate shitty behavior of original MOOS
                    Params.push_front(sKey + "=" + sVal);
                    lua_pop(m_luaState, 1); // pop value and keep key
                }
                break;
                
            case LUA_TNIL: //blank line
                Params.push_front(sKey + "=");
                break;

            default:                // do nothing
                break;

            } // switch
            
//            lua_pop(m_luaState, 1); //pop value

        } // if key is a string

        lua_pop(m_luaState, 1); // pop the value, keep the key
    } // while

        
    lua_pop(m_luaState, 1); //pop the table

    return true;

}


bool CMOOSLuaConfig::GetConfigurationParam(std::string sAppName, string sParam, string &sVal)
{
    bool bGotVar = false;
    bool bGotRet = false;
    string sKey;

    //get var that we want
    lua_getglobal(m_luaState, sAppName.c_str());

    //make sure the requested section (table) was found
    if (LUA_TTABLE != lua_type(m_luaState, -1))
    {
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_pushnil(m_luaState); // push dummy key and iterate... based on common example

    while(lua_next(m_luaState, -2)) 
    {
        //avoid calling lua_tostring on a non-string key... messes up lua_next 
        if (LUA_TSTRING == lua_type(m_luaState, -2))
        {
            sKey = lua_tostring(m_luaState, -2);
            
            if (MOOSStrCmp(sKey, sParam))
            {
                switch (lua_type(m_luaState, -1))
                {
                case LUA_TSTRING:
                case LUA_TNUMBER:
                case LUA_TBOOLEAN:
                    bGotVar = true;
                    bGotRet = true;
                    sVal = myToString(-1);
                    break;
                    
                case LUA_TTABLE:
                    //return last entry if there is one
                    bGotVar = true;
                    lua_pushnil(m_luaState); //dummy key again

                    while (lua_next(m_luaState, -2))
                    {
                        bGotRet = true;
                        sVal = myToString(-1);
                        lua_pop(m_luaState, 1); // pop value and keep key
                    }
                    break;

                default: //break 
                    bGotVar = true;
                    break;


                } // switch

                if (bGotVar) 
                {
                    lua_pop(m_luaState, 2); //pop key & value
                    break;
                } 

            } // if right key

        } // if key is a string

        lua_pop(m_luaState, 1); // pop the value, keep the key
    } // while

        
    lua_pop(m_luaState, 1); //pop the table
    
    MOOSRemoveChars(sVal," \t\r");
    return bGotRet;

}

//this just looks up the value of a global
bool CMOOSLuaConfig::GetValue(std::string sName, std::string &sResult)
{
    //get it from the globals section
    return GetConfigurationParam("_G", sName, sResult);
}

std::string CMOOSLuaConfig::myToString(int idx)
{
    if (idx < 0)
    {
        idx = lua_gettop(m_luaState) + idx + 1;
    }

    std::string sResult = "";

    // table ? make it a string
    if (lua_istable(m_luaState, idx)) 
    {
        std::stringstream os;
        sResult += "{\n";
        lua_pushnil(m_luaState);
        while(lua_next(m_luaState, -2)) 
        {
            sResult += "[" + myToString(-2)
                + "]=" + myToString(-1) + ",\n";

            lua_pop(m_luaState, 1); // pop the value, keep the key
        }
        sResult += "}\n";
        return sResult;

    } 
    else 
    {
        //run lua's tostring function
        lua_getglobal(m_luaState, "tostring");
        lua_pushvalue(m_luaState, idx);
        lua_pcall(m_luaState, 1, 1, 0);
        sResult = lua_tostring(m_luaState, -1);
        lua_pop(m_luaState, 1);
        return sResult;
    }
}
