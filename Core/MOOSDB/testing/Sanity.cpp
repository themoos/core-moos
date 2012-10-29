#include "MOOS/libMOOS/App/MOOSApp.h"
#include "MOOS/libMOOS/Thirdparty/getpot/getpot.h"

class TestApp : public CMOOSApp
{
public:
protected:
	double dfA;
};


int main(int argc, char * argv[])
{
	GetPot cl(argc,argv);
	TestApp A;
	A.Run("Sanity","Mission.moos");
	return 0;
}
