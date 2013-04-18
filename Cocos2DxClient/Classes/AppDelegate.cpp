#include "cocos2d.h"
#include "CCEGLView.h"
#include "AppDelegate.h"
#include "LoginScene.h"
#include "LobbyScene.h"
#include "CCScheduler.h"

#include "Network.h"
#include "Log.h"

#include <boost\bind.hpp>
#include <rapidjson\document.h>

USING_NS_CC;

AppDelegate::AppDelegate()
	: mUpdater(*this)
{
	Network::Init();

	mFSM.RegisterState(kStateLogin, boost::bind(&AppDelegate::OnEnterLogin, this, _1),
									boost::bind(&AppDelegate::OnUpdateLogin, this),
									boost::bind(&AppDelegate::OnLeaveLogin, this, _1));

	mFSM.RegisterState(kStateLobby, boost::bind(&AppDelegate::OnEnterLobby, this, _1),
									boost::bind(&AppDelegate::OnUpdateLobby, this),
									boost::bind(&AppDelegate::OnLeaveLobby, this, _1));

	mFSM.RegisterState(kStateGame, boost::bind(&AppDelegate::OnEnterGame, this, _1),
								   boost::bind(&AppDelegate::OnUpdateGame, this),
								   boost::bind(&AppDelegate::OnLeaveGame, this, _1));
}

AppDelegate::~AppDelegate()
{
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


void AppDelegate::OnSocketConnect()
{
	LOG("AppDelegate::OnSocketConnect()");
	mFSM.SetState(kStateLobby);
}


void AppDelegate::OnSocketRecv(bool hasParseError, rapidjson::Document& data)
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
	case kStateLogin:
		break;

	case kStateLobby:
		{
			LobbyScene* lobby = static_cast<LobbyScene*>(director->getRunningScene());
			lobby->OnRecv(data);
		}
		break;

	case kStateGame:
		{
			//GameScene* game = static_cast<GameScene*>(director->getRunningScene());
			//game->OnRecv(data);
		}
		break;

	default:
		assert(0);
		return;
	}
}

void AppDelegate::OnSocketClose()
{
	LOG("AppDelegate::OnSocketClose()");
	ShowMsgBox("Error", "Connection closed.");

}

void AppDelegate::Send(const rapidjson::Document& data)
{
	mSocket.AsyncSend(data);
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
	PollingSocket::OnConnectFunc onConnect = boost::bind(&AppDelegate::OnSocketConnect, this);
	PollingSocket::OnRecvFunc onRecv = boost::bind(&AppDelegate::OnSocketRecv, this, _1, _2);
	PollingSocket::OnCloseFunc onClose = boost::bind(&AppDelegate::OnSocketClose, this);
	mSocket.Init(onConnect, onRecv, onClose);

    CCDirector::sharedDirector()->runWithScene(LoginScene::create());
}
void AppDelegate::OnUpdateLogin(){}
void AppDelegate::OnLeaveLogin(int nextState){}

// Lobby
void AppDelegate::OnEnterLobby(int prevState)
{
	CCDirector::sharedDirector()->replaceScene(LobbyScene::create());
}
void AppDelegate::OnUpdateLobby(){}
void AppDelegate::OnLeaveLobby(int nextState){}


// Game
void AppDelegate::OnEnterGame(int prevState){}
void AppDelegate::OnUpdateGame(){}
void AppDelegate::OnLeaveGame(int nextState){}
