#ifndef __APP_DELEGATE_H__
#define __APP_DELEGATE_H__

#include "cocos2d.h"
#include "PollingSocket.h"
#include "FSM.h"
#include <string>
#include <rapidjson/document.h>

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

	void GoToLobby();
	void GoToTicTacToe();
	void GoToChecker();
	void GoToSnakeCycles();

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
	void OnSocketConnect(PollingSocket*);
	void OnSocketRecv(PollingSocket*, bool hasParseError, rapidjson::Document& data);
	void OnSocketClose(PollingSocket*);

	// fsm
	void OnEnterLogin(int prevState);
	void OnRecvLogin(rapidjson::Document& data);
	void OnLeaveLogin(int nextState);

	void OnEnterLobby(int prevState);
	void OnRecvLobby(rapidjson::Document& data);
	void OnLeaveLobby(int nextState);

	void OnEnterTicTacToeGame(int prevState);
	void OnRecvTicTacToeGame(rapidjson::Document& data);
	void OnLeaveTicTacToeGame(int nextState);

	void OnEnterCheckerGame(int prevState);
	void OnRecvCheckerGame(rapidjson::Document& data);
	void OnLeaveCheckerGame(int nextState);

	void OnEnterSnakeCyclesGame(int prevState);
	void OnRecvSnakeCyclesGame(rapidjson::Document& data);
	void OnUpdateSnakeCyclesGame();
	void OnLeaveSnakeCyclesGame(int nextState);

	void DummyUpdate() {}

private:
	Updater mUpdater;
	PollingSocket mSocket;
	std::string mUserName;

	enum State
	{
		kStateLogin,
		kStateLobby,
		kStateTicTacToeGame,
		kStateCheckerGame,
		kStateSnakeCyclesGame,
	};
	FSM mFSM;

	// don't use getRunningScene() as it is not immediate mode.
	cocos2d::CCScene* mCurScene;
};

#endif  // __APP_DELEGATE_H__

