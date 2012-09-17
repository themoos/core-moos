#include "MOOS/libMOOS/Utils/MOOSThread.h"
#include "MOOS/libMOOS/Utils/SafeList.h"
#include "MOOS/libMOOS/Utils/KeyboardCapture.h"

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
