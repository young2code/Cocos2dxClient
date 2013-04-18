#include "LobbyScene.h"
#include "GUI\CCEditBox\CCEditBox.h"
#include "AppDelegate.h"

using namespace cocos2d;
using namespace cocos2d::extension;


LobbyScene::LobbyScene()
	: mEditEcho(NULL)
{
}

LobbyScene::~LobbyScene()
{
	if (mEditEcho)
	{
		mEditEcho->release();
		mEditEcho = NULL;
	}
}


// on "init" you need to initialize your instance
bool LobbyScene::init()
{
    bool bRet = false;
    do 
    {
        //////////////////////////////////////////////////////////////////////////
        // super init first
        //////////////////////////////////////////////////////////////////////////

        CC_BREAK_IF(! CCScene::init());

        //////////////////////////////////////////////////////////////////////////
        // add your codes below...
        //////////////////////////////////////////////////////////////////////////

        CCSize size = CCDirector::sharedDirector()->getWinSize();

		// Title
        CCLabelTTF* title = CCLabelTTF::create("Wait for other players..", "Arial", 24);
        title->setPosition(ccp(size.width / 2, size.height - 50));
        addChild(title, 1);

		// Echo
		CCSize serverSize(size.width / 3, 30);
		mEditEcho = CCEditBox::create(serverSize, CCScale9Sprite::create("green_edit.png"));
		mEditEcho->retain();
		mEditEcho->setPosition(ccp(size.width/2, size.height - 100));
		mEditEcho->setText("yahoo~");
		mEditEcho->setPlaceHolder(mEditEcho->getText());
		mEditEcho->setPlaceholderFont("Arial", 20);
		mEditEcho->setPlaceholderFontColor(ccBLACK);
		addChild(mEditEcho);

		// Output
		CCLabelTTF* output = CCLabelTTF::create("Hello World", "Arial", 20, CCSizeMake(0, 0), kCCTextAlignmentCenter);

		addChild(output);

		//// Connect
	 //   CCLabelTTF* connectLabel = CCLabelTTF::create("Connect", "Arial", 20);
		//CCMenuItemLabel* connectItem = CCMenuItemLabel::create(connectLabel, this, menu_selector(LobbyScene::menuConnectCallback));
  //      CCMenu* menu = CCMenu::create(connectItem, NULL);
  //      menu->setPosition(ccp(size.width/2, size.height/2));
  //      addChild(menu, 1);


        bRet = true;
    } while (0);

    return bRet;
}

void LobbyScene::menuEchoCallback(CCObject* pSender)
{
 //   AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());
	//app->Send()
}

