#ifndef __APP_DELEGATE_H__
#define __APP_DELEGATE_H__

#include "cocos2d.h"
#include "PollingSocket.h"
#include "FSM.h"
#include <string>

/**
@brief    The cocos2d Application.

The reason for implement as private inheritance is to hide some interface call by CCDirector.
*/
class  AppDelegate : public cocos2d::CCApplication
{
public:
    AppDelegate();
    virtual ~AppDelegate();

    /**
    @brief    Implement CCDirector and CCScene init code here.
    @return true    Initialize success, app continue.
    @return false   Initialize failed, app terminate.
    */
    virtual bool applicationDidFinishLaunching();

    /**
    @brief  The function be called when the application enter background
    @param  the pointer of the application
    */
    virtual void applicationDidEnterBackground();

    /**
    @brief  The function be called when the application enter foreground
    @param  the pointer of the application
    */
    virtual void applicationWillEnterForeground();

	void Update(float dt);

	void ConnectToServer(const char* address, const char* name);

	void Send(const rapidjson::Document& data);

	const char* GetMyName() const { return mUserName.c_str() ; }

	void ShowMsgBox(const char* title, const char* body, ...);

private:
	// updater. cocos2dx does not provide a global udpate function.
	class Updater : public cocos2d::CCObject
	{
	public:
		Updater(AppDelegate& app) : mApp(app) {}
		virtual void update(float dt) { mApp.Update(dt); }

	private:
		AppDelegate& mApp;
	};

	// network
	void OnSocketConnect();
	void OnSocketRecv(bool hasParseError, rapidjson::Document& data);
	void OnSocketClose();

	// fsm
	void OnEnterLogin(int prevState);
	void OnUpdateLogin();
	void OnLeaveLogin(int nextState);

	void OnEnterLobby(int prevState);
	void OnUpdateLobby();
	void OnLeaveLobby(int nextState);

	void OnEnterTicTacToeGame(int prevState);
	void OnUpdateTicTacToeGame();
	void OnLeaveTicTacToeGame(int nextState);

private:
	Updater mUpdater;
	PollingSocket mSocket;
	std::string mUserName;

	enum State
	{
		kStateLogin,
		kStateLobby,
		kStateTicTacToeGame,
	};
	FSM mFSM;

	// don't use getRunningScene() as it is not immediate mode.
	cocos2d::CCScene* mCurScene;
};

#endif  // __APP_DELEGATE_H__

