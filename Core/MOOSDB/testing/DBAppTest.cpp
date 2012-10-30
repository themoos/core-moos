#include "MOOS/libMOOS/App/MOOSApp.h"
#include "MOOS/libMOOS/Thirdparty/getpot/getpot.h"

class TestApp : public CMOOSApp
{
public:
protected:
	double dfA;
	double dfB;
};

int main(int argc, char * argv[])
{
	GetPot cl(argc,argv);

	CMOOSApp A;
	//A.Run("DBApptest","Mission.moos");
	return 0;
}
