#pragma once
#include "cocos2d.h"

namespace cocos2d
{
	namespace extension
	{
		class CCEditBox;
	}
}

class LobbyScene : public cocos2d::CCScene
{
public:
	LobbyScene();
	virtual ~LobbyScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // a selector callback
    void menuEchoCallback(CCObject* pSender);

    // implement the "static node()" method manually
    CREATE_FUNC(LobbyScene);

private:
	cocos2d::extension::CCEditBox* mEditEcho;
};

