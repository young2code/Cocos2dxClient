#include "FSM.h"
#include <cassert>

FSM::FSM(void)
	: mState(kInvalidState)
{
}


FSM::~FSM(void)
{
}


void FSM::RegisterState(int state, OnEnterFunc enter, OnUpdateFunc update, OnLeaveFunc leave)
{
	assert(mCallbacks.end() == mCallbacks.find(state));
	Callbacks callbacks = {enter, update, leave};
	mCallbacks[state] = callbacks;
}


void FSM::UnRegisterState(int state)
{
	CallbackMap::iterator itor = mCallbacks.find(state);
	if (itor != mCallbacks.end())
	{
		mCallbacks.erase(itor);
	}
}


void FSM::Reset(bool callLeave)
{
	if (callLeave)
	{
		CallbackMap::iterator itor = mCallbacks.find(mState);
		if (itor != mCallbacks.end())
		{
			itor->second.leave(kInvalidState);
		}
	}

	mState = kInvalidState;
	mCallbacks.clear();
}


void FSM::SetState(int nextState)
{
	int prevState = mState;
	mState = nextState;

	CallbackMap::iterator itor = mCallbacks.find(prevState);
	if (itor != mCallbacks.end())
	{
		itor->second.leave(nextState);
	}

	itor = mCallbacks.find(nextState);
	if (itor != mCallbacks.end())
	{
		itor->second.enter(prevState);
	}
}


void FSM::Update()
{
	CallbackMap::iterator itor = mCallbacks.find(mState);
	if (itor != mCallbacks.end())
	{
		itor->second.update();
	}
}


