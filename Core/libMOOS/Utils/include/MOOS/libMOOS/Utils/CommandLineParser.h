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

	/** return true if command line parameters have been set*/
	bool IsAvailable();

	// -x=7 or -name=fred  (s var name=value)
	bool GetVariable(const std::string option,  double & result);
	bool GetVariable(const std::string option,  std::string  & result);
	bool GetVariable(const std::string option,  int & result);
	bool GetVariable(const std::string option,  unsigned int & result);

	//-x 7  -name fred    (so no equals)
	bool GetOption(const std::string option,  double & result);
	bool GetOption(const std::string option,  std::string  & result);
	bool GetOption(const std::string option,  int & result);
	bool GetOption(const std::string option,  unsigned int & result);

	// -k -t  -s   (so test is a single flag is set)
	bool GetFlag(const std::string flag);

private:
	std::auto_ptr<GetPot> pcl_;
};








}

#endif /* COMMANDLINEPARSER_H_ */
