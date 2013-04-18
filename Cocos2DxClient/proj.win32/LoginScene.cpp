#include "LoginScene.h"
#include "GUI\CCEditBox\CCEditBox.h"
#include "AppDelegate.h"

using namespace cocos2d;
using namespace cocos2d::extension;

LoginScene::LoginScene()
	: mEditServer(NULL)
	, mEditUserName(NULL)
{
}

LoginScene::~LoginScene()
{
	if (mEditServer)
	{
		mEditServer->release();
		mEditServer = NULL;
	}

	if (mEditUserName)
	{
		mEditUserName->release();
		mEditUserName = NULL;
	}
}


// on "init" you need to initialize your instance
bool LoginScene::init()
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
        CCLabelTTF* title = CCLabelTTF::create("Please Login", "Arial", 24);
        title->setPosition(ccp(size.width / 2, size.height - 50));
        addChild(title, 1);

		// Server Address
		CCSize serverSize(size.width / 3, 30);
		mEditServer = CCEditBox::create(serverSize, CCScale9Sprite::create("green_edit.png"));
		mEditServer->retain();
		mEditServer->setPosition(ccp(size.width/2, size.height - 100));
		mEditServer->setText("127.0.0.1:54347");
		mEditServer->setFontColor(ccBLACK);
		addChild(mEditServer);

		// UserName
		CCSize nameSize(size.width / 3, 30);
		mEditUserName = CCEditBox::create(nameSize, CCScale9Sprite::create("green_edit.png"));
		mEditUserName->retain();
		mEditUserName->setPosition(ccp(size.width/2, size.height - 150));
		mEditUserName->setText("anonymous");
		mEditUserName->setFontColor(ccBLUE);
		addChild(mEditUserName);

		// Connect
	    CCLabelTTF* connectLabel = CCLabelTTF::create("Connect", "Arial", 20);
		CCMenuItemLabel* connectItem = CCMenuItemLabel::create(connectLabel, this, menu_selector(LoginScene::menuConnectCallback));
        CCMenu* menu = CCMenu::create(connectItem, NULL);
        menu->setPosition(ccp(size.width/2, size.height/2));
        addChild(menu, 1);


        bRet = true;
    } while (0);

    return bRet;
}

void LoginScene::menuConnectCallback(CCObject* pSender)
{
    AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());
	app->ConnectToServer(mEditServer->getText(), mEditUserName->getText());
}

