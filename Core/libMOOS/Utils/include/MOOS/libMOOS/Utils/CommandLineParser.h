/*
 * CommandLineParser.h
 *
 *  Created on: Nov 12, 2012
 *      Author: pnewman
 */

#ifndef COMMANDLINEPARSER_H_
#define COMMANDLINEPARSER_H_
#include <memory>
#include <string>
/*
 *
 */

class GetPot;

namespace MOOS {

class CommandLineParser {
public:
	CommandLineParser();

	virtual ~CommandLineParser();

	bool Open(int argc, char * argv[]);

	bool GetVariable(const std::string option,  double & result);
	bool GetVariable(const std::string option,  std::string  & result);
	bool GetVariable(const std::string option,  int & result);
	bool GetFlag(const std::string flag);


private:
	std::auto_ptr<GetPot> pcl_;
};






}

#endif /* COMMANDLINEPARSER_H_ */
