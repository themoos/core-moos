#include "MOOS/libMOOS/Utils/MOOSThread.h"
#include "MOOS/libMOOS/Utils/SafeList.h"
#include "MOOS/libMOOS/Utils/KeyboardCapture.h"
#include <iostream>
//#include "unistd.h"

namespace MOOS
{

class KeyboardCapture::Impl
{
public:
	CMOOSThread worker_;
	MOOS::SafeList<char> queue_;

};

KeyboardCapture::KeyboardCapture(): impl_(new Impl)
{

}

bool KeyboardCapture::dispatch(void * param)
{
	KeyboardCapture* pMe = (KeyboardCapture*)param;
	return pMe->Capture();
}
bool KeyboardCapture::Capture()
{
//	MOOSPause(1000);
//	std::cout<<"cin :"<< isatty(0)<<std::endl;
//	std::cout<<"cout :"<< isatty(1)<<std::endl;
//	std::cout<<"cerr :"<< isatty(2)<<std::endl;
	while(!impl_->worker_.IsQuitRequested())
	{
		char c = MOOSGetch();
		impl_->queue_.Push(c);
	}
	return true;
}
bool KeyboardCapture::Start()
{
	impl_->worker_.Initialise(dispatch,this);
	return impl_->worker_.Start();
}

bool KeyboardCapture::GetKeyboardInput(char & input)
{
	if(impl_->queue_.Size()==0)
		return false;

	return impl_->queue_.Pull(input);
}


};
