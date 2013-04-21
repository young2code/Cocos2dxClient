#include "LobbyScene.h"
#include "GUI\CCEditBox\CCEditBox.h"
#include "AppDelegate.h"

using namespace cocos2d;
using namespace cocos2d::extension;


LobbyScene::LobbyScene()
	: mEditInput(NULL)
	, mLabelOutput(NULL)
{
}

LobbyScene::~LobbyScene()
{
	if (mEditInput)
	{
		mEditInput->release();
		mEditInput = NULL;
	}

	if (mLabelOutput)
	{
		mLabelOutput->release();
		mLabelOutput = NULL;
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
		mEditInput = CCEditBox::create(serverSize, CCScale9Sprite::create("green_edit.png"));
		mEditInput->retain();
		mEditInput->setPosition(ccp(size.width/2, size.height - 100));
		mEditInput->setText("yahoo~");
		mEditInput->setPlaceHolder(mEditInput->getText());
		mEditInput->setPlaceholderFont("Arial", 20);
		mEditInput->setPlaceholderFontColor(ccBLACK);
		addChild(mEditInput);

		// Output
		mLabelOutput = CCLabelTTF::create("...", "Arial", 20, CCSizeMake(0, 0), kCCTextAlignmentCenter);
		mLabelOutput->retain();
		mLabelOutput->setColor(ccORANGE);
		mLabelOutput->setPosition(ccp(size.width/2, size.height - 150));
		addChild(mLabelOutput);

		// Echo
	    CCLabelTTF* echoLabel = CCLabelTTF::create("Echo Test", "Arial", 20);
		CCMenuItemLabel* connectItem = CCMenuItemLabel::create(echoLabel, this, menu_selector(LobbyScene::menuEchoCallback));

		// Start TicTacToe
	    CCLabelTTF* tictactoeLabel = CCLabelTTF::create("Start TicTacToe", "Arial", 20);
		CCMenuItemLabel* tictactoeItem = CCMenuItemLabel::create(tictactoeLabel, this, menu_selector(LobbyScene::menuTicTacToeCallback));
		tictactoeItem->setPositionY(-20);

		// Menu
        CCMenu* menu = CCMenu::create();
		menu->addChild(connectItem);
		menu->addChild(tictactoeItem);
        menu->setPosition(ccp(size.width/2, size.height/2));
        addChild(menu);


        bRet = true;
    } while (0);

    return bRet;
}


void LobbyScene::menuEchoCallback(CCObject* pSender)
{
	AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());
	
	rapidjson::Document data;
	data.SetObject();
	data.AddMember("type", "echo", data.GetAllocator());
	data.AddMember("msg", mEditInput->getText(), data.GetAllocator());

	app->Send(data);
}

void LobbyScene::menuTicTacToeCallback(CCObject* pSender)
{
	AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());
	
	rapidjson::Document data;
	data.SetObject();
	data.AddMember("type", "service_create", data.GetAllocator());
	data.AddMember("name", "tictactoe", data.GetAllocator());

	app->Send(data);
	app->GoToTicTacToe();
}


bool LobbyScene::OnRecv(rapidjson::Document& data)
{
	assert(data["type"].IsString());	
	std::string type(data["type"].GetString());

	if (type == "echo")
	{
		assert(data["msg"].IsString());	
		mLabelOutput->setString(data["msg"].GetString());
		return true;
	}

	return false;
}