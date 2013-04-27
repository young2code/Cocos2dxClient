#pragma once
#include "cocos2d.h"
#include <rapidjson\document.h>

namespace cocos2d
{
	namespace extension
	{
		class CCEditBox;
	}
	class CCLabelTTF;
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
    void menuTicTacToeCallback(CCObject* pSender);
    void menuCheckerCallback(CCObject* pSender);

	// network
	bool OnRecv(rapidjson::Document& data);

    // implement the "static node()" method manually
    CREATE_FUNC(LobbyScene);

private:
	cocos2d::extension::CCEditBox* mEditInput;
	cocos2d::CCLabelTTF* mLabelOutput;
};

