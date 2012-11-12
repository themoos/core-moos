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

bool CommandLineParser::GetVariable(const std::string option,  double & result)
{
	if(!pcl_.get())
		return false;

	if(!pcl_->search(option.c_str()))
			return false;

	result = (*pcl_)(option.c_str(),result);

	return true;

}
bool CommandLineParser::GetVariable(const std::string option,  std::string  & result)
{
	if(!pcl_.get())
		return false;

	if(!pcl_->search(option.c_str()))
			return false;

	result = (*pcl_)(option.c_str(),result.c_str());

	return true;


}
bool CommandLineParser::GetVariable(const std::string option,  int & result)
{
	if(!pcl_.get())
		return false;

	if(!pcl_->search(option.c_str()))
			return false;

	result = (*pcl_)(option.c_str(),result);

	return true;


}
bool CommandLineParser::GetFlag(const std::string flag)
{
	return pcl_->search(flag.c_str());
}


}//namespace MOOS