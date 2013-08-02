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
// MOOSLuaEnvironment.h - interface for the CMOOSLuaEnvironment class
//
/////////////////////////////////////


#if !defined(AFX_MOOSLUAENVIRONMENT_H_INCLUDED_)
#define AFX_MOOSLUAENVIRONMENT_H_INCLUDED_

#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"
#include <lua5.1/lua.hpp>
#include <string>
#include <map>
#include <set>

using namespace std;

typedef map<string, lua_CFunction> LUA_API_MAP;

enum LuaApiRetcode
{
    LAPI_SUCCESS = 0,
    LAPI_ARGC_FAIL,
    LAPI_RANGE_FAIL,
    LAPI_CMD_FAIL,
    LAPI_DTYPE_FAIL,
};


class CMOOSLuaEnvironment
{
public:
    CMOOSLuaEnvironment(string sIdentifier,
                        LUA_API_MAP api, 
                        void (*OnError)(string, string),
                        string sLuaLibDir);

    virtual ~CMOOSLuaEnvironment();
    
    //put a function in the environment
    int Load(string filename);

    //get the last error message from a load operation
    string LastLoadError();

    //identifier for this environment, used in debug messages
    string Identifier() { return m_sIdentifier; }

    //filename
    string Filename() { return m_sLuaFilename; }

    //re-open the lua environment with a new API
    void SwitchAPI(LUA_API_MAP api, void (*OnError)(string, string));

    //get a meaningful error description
    static string LuaErrorDesc(int resultcode);

    //toggle debugging messages
    void SetDebug(bool enabled) { m_bDebug = enabled; }

protected:

    lua_State* m_luaState;
    string m_sIdentifier;
    string m_sLuaFilename;
    string m_sLuaLibdir;
    string m_sLastError;
    bool m_bDebug;

    void (*m_OnError)(string, string);

    void InitLua(LUA_API_MAP api);
    
    //find out whether a function exists in the environment
    bool LuaFuncExists(string func);

    //find out whether a variable exists in the environment
    bool LuaVariableExists(string varname);

    void StackDump(lua_State* l);
    
    //wrapper for LuaFuncExists that prints an error message
    bool CheckFunction(bool initial, string func);

    //call a function in the lua environment that takes no arguments
    int CallLuaVoid(string func, int num_results);
    
    //after pushing a function onto the stack, call it and handle its result
    int CallLua(int num_args, int num_results, int err_handler_loc);

    //take the error handler off the stack
    void CallLuaCleanup();

    //put an error handling function on the stack and return its index
    int SetLuaErrHandler();
    
    //one liners for adding an entry to a table (string key)
    void BuildLuaTable(string key, bool value, int tableindex);
    void BuildLuaTable(string key, int value, int tableindex);
    void BuildLuaTable(string key, double value, int tableindex);
    void BuildLuaTable(string key, string value, int tableindex);

};

#endif // !defined(AFX_MOOSLUAENVIRONMENT_H_INCLUDED_)

