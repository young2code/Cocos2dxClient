#include "LoginScene.h"
#include "GUI\CCEditBox\CCEditBox.h"
#include "AppDelegate.h"

using namespace cocos2d;
using namespace cocos2d::extension;

CCScene* LoginScene::scene()
{
    CCScene * scene = NULL;
    do 
    {
        // 'scene' is an autorelease object
        scene = CCScene::create();
        CC_BREAK_IF(! scene);

        // 'layer' is an autorelease object
        LoginScene *layer = LoginScene::create();
        CC_BREAK_IF(! layer);

        // add layer as a child to scene
        scene->addChild(layer);
    } while (0);

    // return the scene
    return scene;
}


LoginScene::LoginScene()
	: mEditServer(NULL)
{
}

LoginScene::~LoginScene()
{
	if (mEditServer)
	{
		mEditServer->release();
		mEditServer = NULL;
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

        CC_BREAK_IF(! CCLayer::init());

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
		mEditServer->setPlaceHolder(mEditServer->getText());
		mEditServer->setPlaceholderFont("Arial", 20);
		mEditServer->setPlaceholderFontColor(ccBLACK);
		addChild(mEditServer);

		// UserName
		CCSize nameSize(size.width / 3, 30);
		CCEditBox* name = CCEditBox::create(nameSize, CCScale9Sprite::create("green_edit.png"));
		name->setPosition(ccp(size.width/2, size.height - 150));
		name->setPlaceHolder("Enter your name.");
		name->setPlaceholderFont("Arial", 20);
		name->setPlaceholderFontColor(ccBLUE);
		addChild(name);

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
	app->ConnectToServer(mEditServer->getText());
}

