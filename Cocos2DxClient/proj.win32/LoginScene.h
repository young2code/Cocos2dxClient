#pragma once
#include "cocos2d.h"

namespace cocos2d
{
	namespace extension
	{
		class CCEditBox;
	}
}

class LoginScene : public cocos2d::CCLayer
{
public:
	LoginScene();
	virtual ~LoginScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommand to return the exactly class pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    void menuConnectCallback(CCObject* pSender);

    // implement the "static node()" method manually
    CREATE_FUNC(LoginScene);

private:
	cocos2d::extension::CCEditBox* mEditServer;
};

