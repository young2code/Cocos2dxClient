#include "FSM.h"


FSM::FSM(void)
	: mState(kInvalidState)
{
}


FSM::~FSM(void)
{
}


void FSM::RegisterState(int nState, OnEnterFunc enter, OnUpdateFunc update, OnLeaveFunc leave)
{
	Callbacks callbacks = {enter, update, leave};
	mCallbacks[nState] = callbacks;
}


void FSM::UnRegisterState(int nState)
{
	auto itor = mCallbacks.find(nState);
	if (itor != mCallbacks.end())
	{
		mCallbacks.erase(itor);
	}
}


void FSM::Reset(bool callLeave)
{
	if (callLeave)
	{
		auto itor = mCallbacks.find(mState);
		if (itor != mCallbacks.end())
		{
			itor->second.leave(kInvalidState);
		}
	}

	mState = kInvalidState;
	mCallbacks.clear();
}


void FSM::SetState(int nNextState)
{
	auto itor = mCallbacks.find(mState);
	if (itor != mCallbacks.end())
	{
		itor->second.leave(nNextState);
	}

	itor = mCallbacks.find(nNextState);
	if (itor != mCallbacks.end())
	{
		itor->second.enter(mState);
	}

	mState = nNextState;
}


void FSM::Update()
{
	auto itor = mCallbacks.find(mState);
	if (itor != mCallbacks.end())
	{
		itor->second.update();
	}
}


