#pragma once
#include <boost/function.hpp>
#include <map>

class FSM
{
public:
	typedef boost::function<void (int)> OnEnterFunc;
	typedef boost::function<void (void)> OnUpdateFunc;
	typedef boost::function<void (int)> OnLeaveFunc;

	enum
	{
		kInvalidState = -1,
	};

public:
	FSM(void);
	~FSM(void);

	void RegisterState(int nState, OnEnterFunc enter, OnUpdateFunc update, OnLeaveFunc leave);
	void UnRegisterState(int nState);

	void Reset(bool callLeave = true);

	void SetState(int nNextState);
	int GetState() const { return mState; }

	void Update();

private:
	struct Callbacks
	{
		OnEnterFunc enter;
		OnUpdateFunc update;
		OnLeaveFunc leave;
	};

private:
	int mState;
	std::map<int, Callbacks> mCallbacks;
};

