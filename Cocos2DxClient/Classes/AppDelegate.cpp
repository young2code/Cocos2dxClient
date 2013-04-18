#include "cocos2d.h"
#include "CCEGLView.h"
#include "AppDelegate.h"
#include "LoginScene.h"
#include "CCScheduler.h"

#include "Network.h"
#include "Log.h"

#include <boost\bind.hpp>
#include <rapidjson\document.h>

USING_NS_CC;

AppDelegate::AppDelegate()
	: mSocketUpdater(mSocket)
{
	Network::Init();

	PollingSocket::OnConnectFunc onConnect = boost::bind(&AppDelegate::OnSocketConnect, this);
	PollingSocket::OnRecvFunc onRecv = boost::bind(&AppDelegate::OnSocketRecv, this, _1, _2);
	PollingSocket::OnCloseFunc onClose = boost::bind(&AppDelegate::OnSocketClose, this);
	mSocket.Init(onConnect, onRecv, onClose);
}

AppDelegate::~AppDelegate()
{
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
	defaultScheduler->scheduleSelector(schedule_selector(AppDelegate::SocketUpdater::update), &mSocketUpdater, 0, false);

    // create a scene. it's an autorelease object
    //CCScene *pScene = HelloWorld::scene();
    CCScene *pScene = LoginScene::scene();

    // run
    pDirector->runWithScene(pScene);
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


void AppDelegate::ConnectToServer(const char* address)
{
	mSocket.AsyncConnect(address);
}


void AppDelegate::SocketUpdater::update(float dt)
{
	mSocket.Poll();
}

void AppDelegate::OnSocketConnect()
{
	LOG("AppDelegate::OnSocketConnect()");
}

void AppDelegate::OnSocketRecv(bool hasParseError, const rapidjson::Document& data)
{
	if (hasParseError)
	{
		LOG("AppDelegate::OnSocketRecv() - parsing json failed.");
		ShowMsgBox("Error", "JSON parsing failed.");
		return;
	}
}

void AppDelegate::OnSocketClose()
{
	LOG("AppDelegate::OnSocketClose()");
	ShowMsgBox("Error", "Connection closed.");

	// Recreate socket.
	PollingSocket::OnConnectFunc onConnect = boost::bind(&AppDelegate::OnSocketConnect, this);
	PollingSocket::OnRecvFunc onRecv = boost::bind(&AppDelegate::OnSocketRecv, this, _1, _2);
	PollingSocket::OnCloseFunc onClose = boost::bind(&AppDelegate::OnSocketClose, this);
	mSocket.Init(onConnect, onRecv, onClose);
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
