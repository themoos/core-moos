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


#include "MOOS/libMOOS/Utils/MOOSLuaEnvironment.h"
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"


/**
 * Lua environment initialization
 *
 * Export some useful functions to lua
 */
CMOOSLuaEnvironment::CMOOSLuaEnvironment(string sIdentifier,
                                         LUA_API_MAP api, 
                                         void (*OnError)(string, string),
                                         string sLuaLibDir)
{
    m_bDebug = false;

    m_sLuaFilename = "";
    m_sLuaLibdir = sLuaLibDir;
    m_sIdentifier = sIdentifier;
    m_sLastError = "(no error)";

    m_OnError = OnError;

    InitLua(api);
    
}


CMOOSLuaEnvironment::~CMOOSLuaEnvironment()
{
    lua_close(m_luaState);
}

//set up a lua environment around the supplied lua file
int CMOOSLuaEnvironment::Load(string filename)
{
    const char *msg;

    //try to load; on error, record message and bail immediately
    int ret = luaL_loadfile(m_luaState, filename.c_str());
    if (ret) 
    {
        msg = lua_tostring(m_luaState, -1);
        if (msg == NULL) msg = "(error with no message)";
        m_sLastError = msg;
        lua_pop(m_luaState, 1); // remove error message from stack

        return ret;
    }

    //"run" file, to load functions etc
    if (lua_pcall(m_luaState, 0, LUA_MULTRET, 0)) 
    {
        msg = lua_tostring(m_luaState, -1);
        if (msg == NULL) msg = "(error with no message)";
        m_sLastError = msg;
        lua_pop(m_luaState, 1); // remove error message from stack

        return 1;
    }

    //successful; record filename
    m_sLuaFilename = filename;
    m_sLastError = "(No Error)";
    
    return 0;
}

//if a lua error occurred on load, this returns the text of the error message
string CMOOSLuaEnvironment::LastLoadError()
{
    return m_sLastError;
}

//create an empty lua state with libraries and API functions
void CMOOSLuaEnvironment::InitLua(LUA_API_MAP api)
{
    m_luaState = luaL_newstate();
    luaL_openlibs(m_luaState);

    //register API functions as per the map: "funcInLua" => HAPI_func_pointer
    for (LUA_API_MAP::iterator it = api.begin(); it != api.end(); it++)
    {
        lua_register(m_luaState, it->first.c_str(), it->second);
    }

    //initialize the lua libdir to the stored val: edit package.path
    lua_getglobal(m_luaState, "package"); // pull up package on stack
    if (!lua_istable(m_luaState, -1))
    {
        printf("CMOOSLuaEnvironment::SetLibDir: package is not a table\n");
    }
    else
    {
        //incorporate both files and packages in the lib dir
        string libs = m_sLuaLibdir + "/?.lua;" + m_sLuaLibdir + "/?/init.lua";
        lua_pushstring(m_luaState, libs.c_str());
        lua_setfield(m_luaState, -2, "path");
    }
    lua_pop(m_luaState, 1); //remove "package" from stack
}

//close down a lua environment and open it with a different API
//TODO: possible problems can be caused by switching the API at a bad time
//      - can eliminate this by compiling during the test phase and loading the 
//        compiled version
void CMOOSLuaEnvironment::SwitchAPI(LUA_API_MAP api, void (*OnError)(string, string))
{
    MOOSTrace("Switching Lua API in %s\n", m_sIdentifier.c_str());
    lua_close(m_luaState);
    
    InitLua(api);
    
    m_OnError = OnError;

    //reload file in the new environment
    if ("" != m_sLuaFilename)
    {
        Load(m_sLuaFilename);
    }
}

//print a helpful stack dump
void CMOOSLuaEnvironment::StackDump(lua_State* l)
{
    int i;
    int top = lua_gettop(l);

    MOOSTrace("total in stack %d\n", top);

    for (i = top; i >= 1; i--)
    {  
        int t = lua_type(l, i);
        switch (t) 
        {
        case LUA_TSTRING:  /* strings */
            MOOSTrace("  string: '%s'\n", lua_tostring(l, i));
            break;
        case LUA_TBOOLEAN:  /* booleans */
            MOOSTrace("  boolean %s\n",lua_toboolean(l, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:  /* numbers */
            MOOSTrace("  number: %g\n", lua_tonumber(l, i));
            break;
        default:  /* other values */
            MOOSTrace("  %s\n", lua_typename(l, t));
            break;
        }

    }
    MOOSTrace("\n");  /* end the listing */
}

//look for function in lua environment
bool CMOOSLuaEnvironment::LuaFuncExists(string func)
{
    lua_getglobal(m_luaState, func.c_str());
    bool ret = lua_isfunction(m_luaState, -1); 
    lua_pop(m_luaState, 1);
    return ret;
}

//look for variable in lua environment... exists and is not function
bool CMOOSLuaEnvironment::LuaVariableExists(string func)
{
    lua_getglobal(m_luaState, func.c_str());
    bool ret = !lua_isnil(m_luaState, -1) && !lua_isfunction(m_luaState, -1);
    lua_pop(m_luaState, 1);
    return ret;
}

//turn error code into (more) helpful message
string CMOOSLuaEnvironment::LuaErrorDesc(int resultcode)
{
    switch (resultcode)
     {
     case 0:
         return "Success";
     case 1:
         return "Error running file";
     case LUA_ERRRUN:
         return "Runtime error";
     case LUA_ERRSYNTAX:
         return "Syntax error";
     case LUA_ERRMEM:
         return "Memory allocation error";
     case LUA_ERRERR:
         return "Improperly configured error alert mechanism.  FAIL FAIL.";
     case LUA_ERRFILE:
         return "File access error";
     default:
         return MOOSFormat("Unknown Lua error with code '%d'", resultcode);
     }
}

//put an error handler on the stack and let us know where it ended up
int CMOOSLuaEnvironment::SetLuaErrHandler()
{
    if (m_bDebug)
    {
        MOOSTrace("Dump before setting %s error handler: \n", m_sIdentifier.c_str());
        StackDump(m_luaState);
    }

    //set up traceback on stack
    lua_getglobal(m_luaState, "debug");
    lua_getfield(m_luaState, -1, "traceback");
    lua_remove(m_luaState, -2);

    return lua_gettop(m_luaState);
}

//call lua with error handling
// err_handler_loc is a stack location
int CMOOSLuaEnvironment::CallLua(int num_args, int num_results, int err_handler_loc)
{
    if (m_bDebug)
    {
        MOOSTrace("Dump before %s call (err handler at %d): \n", m_sIdentifier.c_str(), err_handler_loc);
        StackDump(m_luaState);
    }

    int ret = lua_pcall(m_luaState, num_args, num_results, err_handler_loc);

    if (ret)
    {
        //report the non-success error, let the owner handle it
        (*m_OnError)(Identifier(), MOOSFormat("%s: %s", 
                                              LuaErrorDesc(ret).c_str(),
                                              lua_tostring(m_luaState, -1)));
    }

    return ret;

}

//call a lua function that doesn't take arguments
int CMOOSLuaEnvironment::CallLuaVoid(string func, int num_results)
{
    if (m_bDebug)
    {
        MOOSTrace("Calling %s:%s\n", m_sIdentifier.c_str(), func.c_str());
    }
    int myErrHandler = SetLuaErrHandler();
    //printf("error handler at pos %d\n", myErrHandler);

    //put function on stack and go for it
    lua_getglobal (m_luaState, func.c_str());

    return CallLua(0, num_results, myErrHandler);
}

//clean up the stack after a lua call
void CMOOSLuaEnvironment::CallLuaCleanup()
{ 
    if (m_bDebug)
    {
        MOOSTrace("Dump before %s cleanup: \n", m_sIdentifier.c_str());
        StackDump(m_luaState);
    }
    lua_pop(m_luaState, 1);
}

//these functions are for building the fields of a table which is already on the stack
void CMOOSLuaEnvironment::BuildLuaTable(string key, bool value, int tableindex)
{
    lua_pushstring(m_luaState, key.c_str());
    lua_pushboolean(m_luaState, value);
    lua_settable(m_luaState, tableindex);
}

void CMOOSLuaEnvironment::BuildLuaTable(string key, int value, int tableindex)
{
    lua_pushstring(m_luaState, key.c_str());
    lua_pushinteger(m_luaState, value);
    lua_settable(m_luaState, tableindex);
}

void CMOOSLuaEnvironment::BuildLuaTable(string key, double value, int tableindex)
{
    lua_pushstring(m_luaState, key.c_str());
    lua_pushnumber(m_luaState, value);
    lua_settable(m_luaState, tableindex);
}

void CMOOSLuaEnvironment::BuildLuaTable(string key, string value, int tableindex)
{
    lua_pushstring(m_luaState, key.c_str());
    lua_pushstring(m_luaState, value.c_str());
    lua_settable(m_luaState, tableindex);
}

