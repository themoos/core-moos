/*
 * ConsoleColours.h
 *
 *  Created on: Sep 23, 2011
 *      Author: pnewman
 */

#ifndef CONSOLECOLOURS_H_
#define CONSOLECOLOURS_H_

namespace MOOS
{

/* ability to change console text
 *     //use as follows std::cerr<<MOOS::ConsoleColours()<<red();
 *
 */
struct ConsoleColours
{

    static const char* red() {return "\x1b[31m";};
    static const char* Red() {return "\x1b[1;31m";};

    static const char* green() {return "\x1b[32m";};
    static const char* Green() {return "\x1b[1;32m";};

    static const char* yellow() {return "\x1b[33m";};
    static const char* Yellow() {return "\x1b[1;33m";};

    static const char* blue() {return "\x1b[34m";};
    static const char* Blue() {return "\x1b[1;34m";};

    static const char* magenta() {return "\x1b[35m";};
    static const char* Magenta() {return "\x1b[1;35m";};

    static const char* cyan() {return "\x1b[36m";};
    static const char* Cyan() {return "\x1b[1;36m";};

    static const char* reset() {return "\x1b[0m";};
};

}
#endif
