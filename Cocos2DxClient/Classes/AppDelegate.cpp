#include "cocos2d.h"
#include "CCEGLView.h"
#include "AppDelegate.h"
#include "LoginScene.h"
#include "LobbyScene.h"
#include "TicTacToeGameScene.h"
#include "CheckerGameScene.h"
#include "SnakeCyclesGameScene.h"
#include "CCScheduler.h"

#include "Network.h"
#include "Log.h"

#include <boost\bind.hpp>
#include <rapidjson\document.h>

USING_NS_CC;

AppDelegate::AppDelegate()
	: mUpdater(*this)
	, mCurScene(NULL)
{
	Network::Init();

#define BIND_CALLBACKS(State) boost::bind(&AppDelegate::OnEnter##State, this, _1), \
							  boost::bind(&AppDelegate::DummyUpdate, this), \
							  boost::bind(&AppDelegate::OnLeave##State, this, _1)

	mFSM.RegisterState(kStateLogin, BIND_CALLBACKS(Login));
	mFSM.RegisterState(kStateLobby, BIND_CALLBACKS(Lobby));
	mFSM.RegisterState(kStateTicTacToeGame, BIND_CALLBACKS(TicTacToeGame));
	mFSM.RegisterState(kStateCheckerGame, BIND_CALLBACKS(CheckerGame));
	mFSM.RegisterState(kStateSnakeCyclesGame, BIND_CALLBACKS(SnakeCyclesGame));

#undef BIND_CALLBACKS

}

AppDelegate::~AppDelegate()
{
	if (mCurScene)
	{
		mCurScene->release();
		mCurScene = NULL;
	}

	mFSM.Reset(false);
	mSocket.Shutdown(false);
	Network::Shutdown();
}

bool AppDelegate::applicationDidFinishLaunching()
{
    // initialize director
    CCDirector *pDirector = CCDirector::sharedDirector();
    pDirector->setOpenGLView(CCEGLView::sharedOpenGLView());

    // turn on display FPS
    //pDirector->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    pDirector->setAnimationInterval(1.0 / 60);

	// polling socket events
    CCScheduler* defaultScheduler = CCDirector::sharedDirector()->getScheduler();
	defaultScheduler->scheduleSelector(schedule_selector(AppDelegate::Updater::update), &mUpdater, 0, false);

	// Start Login screen
	mFSM.SetState(kStateLogin);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
    CCDirector::sharedDirector()->stopAnimation();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    CCDirector::sharedDirector()->startAnimation();
}


void AppDelegate::Update(float dt)
{
	mSocket.Poll();
	mFSM.Update();
}


void AppDelegate::ConnectToServer(const char* address, const char* name)
{
	mUserName = name;
	mSocket.AsyncConnect(address);
}


void AppDelegate::OnSocketConnect(PollingSocket*)
{
	LOG("AppDelegate::OnSocketConnect()");
	assert(mFSM.GetState() == kStateLogin);
	mFSM.SetState(kStateLobby);
}


void AppDelegate::OnSocketRecv(PollingSocket*, bool hasParseError, rapidjson::Document& data)
{
	if (hasParseError)
	{
		LOG("AppDelegate::OnSocketRecv() - parsing json failed.");
		ShowMsgBox("Error", "JSON parsing failed.");
		return;
	}

	CCDirector* director = CCDirector::sharedDirector();
	switch (mFSM.GetState())
	{
	case kStateLogin:			OnUpdateLogin(data);			break;
	case kStateLobby:			OnUpdateLobby(data);			break;
	case kStateTicTacToeGame:	OnUpdateTicTacToeGame(data);	break;
	case kStateCheckerGame:		OnUpdateCheckerGame(data);		break;
	case kStateSnakeCyclesGame: OnUpdateSnakeCyclesGame(data);	break;

	default:
		assert(0);
		return;
	}
}

void AppDelegate::OnSocketClose(PollingSocket*)
{
	LOG("AppDelegate::OnSocketClose()");
	ShowMsgBox("Error", "Connection closed.");

	mFSM.SetState(kStateLogin);
}

void AppDelegate::Send(const rapidjson::Document& data)
{
	mSocket.AsyncSend(data);
}

void AppDelegate::GoToLobby()
{
	mFSM.SetState(kStateLobby);
}

void AppDelegate::GoToTicTacToe()
{
	mFSM.SetState(kStateTicTacToeGame);
}

void AppDelegate::GoToChecker()
{
	mFSM.SetState(kStateCheckerGame);
}

void AppDelegate::GoToSnakeCycles()
{
	mFSM.SetState(kStateSnakeCyclesGame);
}


void AppDelegate::ShowMsgBox(const char* title, const char* body, ...)
{
	char buffer[256] = {0,};
	va_list args;
	va_start(args, body);
	vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer)-1, body, args);
	va_end(args);

	MessageBoxA(NULL, buffer, title, MB_ICONWARNING | MB_OK | MB_DEFBUTTON1);
}

// Login
void AppDelegate::OnEnterLogin(int prevState)
{
	PollingSocket::OnConnectFunc onConnect = boost::bind(&AppDelegate::OnSocketConnect, this, _1);
	PollingSocket::OnRecvFunc onRecv = boost::bind(&AppDelegate::OnSocketRecv, this, _1, _2, _3);
	PollingSocket::OnCloseFunc onClose = boost::bind(&AppDelegate::OnSocketClose, this, _1);
	mSocket.InitWait(onConnect, onRecv, onClose);

	mCurScene = LoginScene::create();
	mCurScene->retain();

	CCDirector* director = CCDirector::sharedDirector();
	if (director->getRunningScene())
	{
		director->replaceScene(mCurScene);
	}
	else
	{
		director->runWithScene(mCurScene);
	}
}

void AppDelegate::OnUpdateLogin(rapidjson::Document& data)
{
	// wait for conneciton is complete.
}

void AppDelegate::OnLeaveLogin(int nextState)
{
	mCurScene->release();
	mCurScene = NULL;
}

// Lobby
void AppDelegate::OnEnterLobby(int prevState)
{
	mCurScene = LobbyScene::create();
	mCurScene->retain();

	CCDirector::sharedDirector()->replaceScene(mCurScene);
}

void AppDelegate::OnUpdateLobby(rapidjson::Document& data)
{
	static_cast<LobbyScene*>(mCurScene)->OnRecv(data);
}

void AppDelegate::OnLeaveLobby(int nextState)
{
	mCurScene->release();
	mCurScene = NULL;
}


// Game
void AppDelegate::OnEnterTicTacToeGame(int prevState)
{
	mCurScene = TicTacToeGameScene::create();
	mCurScene->retain();

	CCDirector::sharedDirector()->replaceScene(mCurScene);
}

void AppDelegate::OnUpdateTicTacToeGame(rapidjson::Document& data)
{
	static_cast<TicTacToeGameScene*>(mCurScene)->OnRecv(data);
}

void AppDelegate::OnLeaveTicTacToeGame(int nextState)
{
	mCurScene->release();
	mCurScene = NULL;
}


void AppDelegate::OnEnterCheckerGame(int prevState)
{
	mCurScene = CheckerGameScene::create();
	mCurScene->retain();

	CCDirector::sharedDirector()->replaceScene(mCurScene);
}

void AppDelegate::OnUpdateCheckerGame(rapidjson::Document& data)
{
	static_cast<CheckerGameScene*>(mCurScene)->OnRecv(data);
}

void AppDelegate::OnLeaveCheckerGame(int nextState)
{
	mCurScene->release();
	mCurScene = NULL;

}



void AppDelegate::OnEnterSnakeCyclesGame(int prevState)
{
	mCurScene = SnakeCyclesGameScene::create();
	mCurScene->retain();

	CCDirector::sharedDirector()->replaceScene(mCurScene);
}

void AppDelegate::OnUpdateSnakeCyclesGame(rapidjson::Document& data)
{
	static_cast<SnakeCyclesGameScene*>(mCurScene)->OnRecv(data);
}

void AppDelegate::OnLeaveSnakeCyclesGame(int nextState)
{
	mCurScene->release();
	mCurScene = NULL;
}

