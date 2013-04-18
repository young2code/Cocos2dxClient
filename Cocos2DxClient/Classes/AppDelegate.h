#ifndef __APP_DELEGATE_H__
#define __APP_DELEGATE_H__

#include "cocos2d.h"
#include "PollingSocket.h"

/**
@brief    The cocos2d Application.

The reason for implement as private inheritance is to hide some interface call by CCDirector.
*/
class  AppDelegate : private cocos2d::CCApplication
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

	void ConnectToServer(const char* address);

	void ShowMsgBox(const char* title, const char* body, ...);

public:
	class SocketUpdater : public cocos2d::CCObject
	{
	public:
		SocketUpdater(PollingSocket& socket) : mSocket(socket) {}
		virtual void update(float dt);

	private:
		PollingSocket& mSocket;
	};
	void OnSocketConnect();
	void OnSocketRecv(bool hasParseError, const rapidjson::Document& data);
	void OnSocketClose();

private:
	PollingSocket mSocket;
	SocketUpdater mSocketUpdater;
};

#endif  // __APP_DELEGATE_H__

