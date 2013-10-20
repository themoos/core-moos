
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
//   This source code and the accompanying materials
//   are made available under the terms of the GNU Lesser Public License v2.1
//   which accompanies this distribution, and is available at
//   http://www.gnu.org/licenses/lgpl.txt
//          
//   This program is distributed in the hope that it will be useful, 
//   but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
////////////////////////////////////////////////////////////////////////////



// MOOSCommPkt.cpp: implementation of the MOOSCommPkt class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
    #pragma warning(disable : 4786)
    #pragma warning(disable : 4503)
#endif


#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"
#include "MOOS/libMOOS/Comms/MOOSCommPkt.h"

#include <assert.h>
#include <cstring>
#include <iostream>


#define DEFAULT_ASSUMMED_MAX_MOOS_MSG_SIZE 40000


#ifdef COMPRESSED_MOOS_PROTOCOL
	
	#define MAX_BELIEVABLE_ZLIB_COMPRESSION 256  //if we look like we are getting a compression ration of more than this then bail - there is an error.

	#define COMPRESSION_PACKET_SIZE_THRESHOLD 1024 //packets below this size won't be compressed.

	#include <zlib.h>
#endif

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSCommPkt::CMOOSCommPkt()
{
    m_pStream = DefaultStream;
    m_nStreamSpace = MOOS_PKT_DEFAULT_SPACE;
    m_pNextData = m_pStream;
    m_bAllocated = false;
    m_nByteCount = 0;
    m_nMsgLen = 0;
	m_dfCompression = 1.0;


}

CMOOSCommPkt::~CMOOSCommPkt()
{
    if(m_bAllocated)
    {
        delete [] m_pStream;
    }
}


int CMOOSCommPkt::GetBytesRequired()
{
    if(m_nByteCount<(int)sizeof(int))
    {
        return sizeof(int)-m_nByteCount;
    }
    else
    {
        return m_nMsgLen-m_nByteCount;
    }
}

bool CMOOSCommPkt::Fill(unsigned char *InData, int nData)
{

    if(m_nByteCount+nData>=m_nStreamSpace)
    {
        InflateTo(2*(m_nStreamSpace+nData));
    }
    memcpy(m_pNextData,InData,nData);
    m_pNextData+=nData;
    m_nByteCount+=nData;
    

    if( m_nByteCount <=(int)sizeof(int))
	{
		if(m_nByteCount==sizeof(int))
		{
			memcpy((void*)(&m_nMsgLen),(void*)m_pStream,sizeof(int));

			//look to swap byte order if this machine is Big End in
			if(!IsLittleEndian())
			{
				m_nMsgLen = SwapByteOrder<int>(m_nMsgLen);
			}
		}
	}

    return true;
}


int CMOOSCommPkt::GetStreamLength()
{
    return m_nMsgLen;
}


/** This function stuffs messages in/from a packet */
bool CMOOSCommPkt::Serialize(MOOSMSG_LIST &List, bool bToStream, bool bNoNULL, double * pdfPktTime)
{
	//note +1 is for indicator regarding compressed or not compressed
	unsigned int nHeaderSize = 2*sizeof(int)+1;
	
    if(bToStream)
    {

        m_nMsgLen=0;
        m_nByteCount = 0;

        //lets figure out how much space we need?
        unsigned int nBufferSize=nHeaderSize; //some head room
        MOOSMSG_LIST::iterator p;
        for(p = List.begin();p!=List.end();p++)
        {
        	nBufferSize+=p->GetSizeInBytesWhenSerialised();
        }

        InflateTo(nBufferSize);

        m_pNextData = m_pStream+nHeaderSize;
        m_nByteCount+= nHeaderSize;

        for(p = List.begin();p!=List.end();p++)
        {

            int nCopied = p->Serialize(m_pNextData,nBufferSize-m_nByteCount);

            if(nCopied==-1)
            {
            	std::cerr<<"big problem failed serialisation: "<<__PRETTY_FUNCTION__<<"\n";
            	return false;
            }

            m_pNextData+=nCopied;
        	m_nByteCount+=nCopied;

        }

		unsigned char bCompressed = 0;

        //finally write how many bytes we have written at the start
        //look for need to swap byte order if required
        m_pNextData = m_pStream;
        int nBC = IsLittleEndian() ? m_nByteCount : SwapByteOrder<int>(m_nByteCount);
        memcpy((void*)m_pNextData,(void*)(&nBC),sizeof(m_nByteCount));
        m_pNextData+=sizeof(m_nByteCount);


        //and then how many messages are included
        //look for need to swap byte order if required
        int nMessages = List.size();
        nMessages = IsLittleEndian() ? nMessages : SwapByteOrder<int>(nMessages);
        memcpy((void*)m_pNextData,(void*)(&nMessages),sizeof(nMessages));
        m_pNextData+=sizeof(nMessages);
		
		//and is this a compressed message or not?
		*m_pNextData = bCompressed;
		m_pNextData+=1;


    }
    else
    {
        
        m_pNextData = m_pStream;
        m_nMsgLen = 0;
        m_nByteCount = 0;

        //first figure out the length of the message
        //look to swap byte order as required
        memcpy((void*)(&m_nMsgLen),(void*)m_pNextData,sizeof(m_nMsgLen));
        m_nMsgLen = IsLittleEndian() ? m_nMsgLen : SwapByteOrder<int>(m_nMsgLen);
        m_pNextData+=sizeof(m_nMsgLen);
        m_nByteCount+=sizeof(m_nMsgLen);


        int nSpaceFree=m_nMsgLen - sizeof(m_nMsgLen);

        //now figure out how many messages are packed in this packet
        //look to swap byte order as required
        int nMessages = 0;
        memcpy((void*)(&nMessages),(void*)m_pNextData,sizeof(nMessages));
        nMessages = IsLittleEndian() ? nMessages : SwapByteOrder<int>(nMessages);
        m_pNextData+=sizeof(nMessages);
        nSpaceFree-=sizeof(nMessages);
        m_nByteCount+=sizeof(nMessages);
		
		//now account for one byet of compression indication
		m_pNextData+=sizeof(unsigned char);
        nSpaceFree-=sizeof(unsigned char);
        m_nByteCount+=sizeof(unsigned char);
		

        for(int i = 0; i<nMessages;i++)
        {
                    
            CMOOSMsg Msg;
            int nUsed = Msg.Serialize(m_pNextData,nSpaceFree,false);

            if(nUsed!=-1) 
            {
                //allows us to not store NULL messages
                bool bOmit = bNoNULL && (Msg.m_cMsgType==MOOS_NULL_MSG);

                if(Msg.m_cMsgType==MOOS_NULL_MSG && pdfPktTime!=NULL && i == 0 )
                {
                    *pdfPktTime     = Msg.GetDouble();                    
                }

                if(!bOmit)
                {
                    List.push_back(Msg);
                }


                m_pNextData+=nUsed;
                m_nByteCount+=nUsed;
                nSpaceFree-=nUsed;

            }
            else
            {
                //bad news...
                break;
            }        
        }
    }

    //here at the last moment we can fill in our totalm length for safe keeping
    m_nMsgLen = m_nByteCount;


    return true;
}

bool CMOOSCommPkt::InflateTo(int nNewStreamSize)
{
	//maybe there is nothing to do....
	if(nNewStreamSize<=MOOS_PKT_DEFAULT_SPACE)
	{
		return true;
	}
    //make more space then..
    m_nStreamSpace = nNewStreamSize;

    unsigned char * pNew = new unsigned char[m_nStreamSpace];

    if(pNew==NULL)
    {
        //absolute disaster..nothing we can do ...whole process must exit
        MOOSTrace("memory allocation failed in CMOOSCommPkt::CopyToStream");
        return false;
    }

    //copy what we have already assembled into new space
    memcpy(pNew,m_pStream,m_nByteCount);

    
    if(m_bAllocated)
    {
        delete [] m_pStream;
    }
    else
    {
        m_bAllocated = true;
    }

    //switch m_pStream to point to the new space;
    m_pStream = pNew;

    //and set the next data point to be m_nMsgLen bytes from start
    //so we will start writin after are existing stuff
    m_pNextData=m_pStream+m_nByteCount;

    return true;
}
