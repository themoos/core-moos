/*
 * SuicidalSleeper.h
 *
 *  Created on: Feb 4, 2014
 *      Author: pnewman
 */

#ifndef SUICIDALSLEEPER_H_
#define SUICIDALSLEEPER_H_

namespace MOOS {

class SuicidalSleeper {
public:
    SuicidalSleeper();
    virtual ~SuicidalSleeper();

    bool Run();

private:
    class Impl;
    Impl* Impl_;
};

}

#endif /* SUICIDALSLEEPER_H_ */
