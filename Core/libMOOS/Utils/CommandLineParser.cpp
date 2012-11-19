/*
 * CommandLineParser.cpp
 *
 *  Created on: Nov 12, 2012
 *      Author: pnewman
 */

#include "MOOS/libMOOS/Utils/CommandLineParser.h"

#include "MOOS/libMOOS/Thirdparty/getpot/getpot.h"


namespace MOOS {


CommandLineParser::CommandLineParser()
{
	// TODO Auto-generated constructor stub

}

CommandLineParser::~CommandLineParser() {
	// TODO Auto-generated destructor stub

}


bool CommandLineParser::Open(int argc, char * argv[])
{
	pcl_ = std::auto_ptr<GetPot>(new GetPot(argc,argv) );
	return true;
}


bool CommandLineParser::GetOption(const std::string option,  double & result)
{
	if(!IsAvailable())
		return false;

	if(!pcl_->search(option.c_str()))
		return false;

	result = pcl_->follow(result,1,option.c_str());

	return true;

}
bool CommandLineParser::GetOption(const std::string option,  std::string  & result)
{
	if(!IsAvailable())
			return false;

	if(!pcl_->search(option.c_str()))
		return false;

	result = pcl_->follow(result.c_str(),1,option.c_str());

	return true;


}
bool CommandLineParser::GetOption(const std::string option,  int & result)
{
	if(!IsAvailable())
		return false;

	if(!pcl_->search(option.c_str()))
		return false;

	result = pcl_->follow(result,1,option.c_str());

	return true;
}

bool CommandLineParser::GetOption(const std::string option,  unsigned int & result)
{
	if(!IsAvailable())
		return false;

	if(!pcl_->search(option.c_str()))
		return false;

	int sr = result;
	sr = pcl_->follow(sr,1,option.c_str());

	if(sr<0)
		return false;

	result = sr;

	return true;

}



bool CommandLineParser::GetVariable(const std::string option,  double & result)
{
	if(!IsAvailable())
		return false;

	result = (*pcl_)(option.c_str(),result);

	return true;

}
bool CommandLineParser::GetVariable(const std::string option,  std::string  & result)
{
	if(!IsAvailable())
		return false;

	result = (*pcl_)(option.c_str(),result.c_str());

	return true;


}
bool CommandLineParser::GetVariable(const std::string option,  int & result)
{
	if(!IsAvailable())
		return false;

	result = (*pcl_)(option.c_str(),result);

	return true;


}

bool CommandLineParser::GetVariable(const std::string option, unsigned  int & result)
{
	if(!IsAvailable())
		return false;

	int sr = result;
	sr = (*pcl_)(option.c_str(),sr);

	if(sr<0)
		return false;

	result= sr;

	return true;
}


bool CommandLineParser::GetFlag(const std::string flag)
{
	if(!IsAvailable())
		return false;
	return    pcl_->search(flag.c_str());
}

bool CommandLineParser::IsAvailable()
{
	return pcl_.get()!=NULL;
}


}//namespace MOOS
