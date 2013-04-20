#include "TicTacToeGameScene.h"
#include "GUI\CCEditBox\CCEditBox.h"
#include "AppDelegate.h"
#include <boost/bind.hpp>
#include "Log.h"

using namespace cocos2d;
using namespace cocos2d::extension;
using namespace TicTacToe;


TicTacToeGameScene::TicTacToeGameScene()
	: mMyIndex(kPlayer1)
	, mCurTurn(kPlayer1)
	, mBoardLayer(NULL)
	, mHUDLayer(NULL)
{
	InitFSM();
}

TicTacToeGameScene::~TicTacToeGameScene()
{
	ShutdownFSM();

	if (mBoardLayer)
	{
		mBoardLayer->release();
		mBoardLayer = NULL;
	}

	if (mHUDLayer)
	{
		mHUDLayer->release();
		mHUDLayer = NULL;
	}
}


// on "init" you need to initialize your instance
bool TicTacToeGameScene::init()
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

		// Board
        mBoardLayer = BoardLayer::create();
		mBoardLayer->retain();
		addChild(mBoardLayer);

		//HUD
		mHUDLayer = HUDLayer::create();
		mHUDLayer->retain();
		addChild(mHUDLayer);

        bRet = true;
    } while (0);

    return bRet;
}


void TicTacToeGameScene::InitFSM()
{
	// we use dummy update as we want to update with network recv events;
#define BIND_CALLBACKS(State) boost::bind(&TicTacToeGameScene::OnEnter##State, this, _1), \
							  boost::bind(&TicTacToeGameScene::DummyUpdate, this), \
							  boost::bind(&TicTacToeGameScene::OnLeave##State, this, _1)

	mFSM.RegisterState(kStateSetPlayers, BIND_CALLBACKS(SetPlayers));
	mFSM.RegisterState(kStateMyTurn, BIND_CALLBACKS(MyTurn));
	mFSM.RegisterState(kStateYourTurn, BIND_CALLBACKS(YourTurn));
	mFSM.RegisterState(kStateGameEnd, BIND_CALLBACKS(GameEnd));
	mFSM.RegisterState(kStateGameCanceled, BIND_CALLBACKS(GameCanceled));

#undef BIND_CALLBACKS

	mFSM.SetState(kStateSetPlayers);
}


void TicTacToeGameScene::ShutdownFSM()
{
	mFSM.Reset(false);
}

bool TicTacToeGameScene::OnRecv(rapidjson::Document& data)
{
	assert(data["type"].IsString());	
	std::string type(data["type"].GetString());

	if (type == "tictactoe")
	{
		switch(mFSM.GetState())
		{
		case kStateSetPlayers:		OnUpdateSetPlayers(data);	break;
		case kStateMyTurn:			OnUpdateMyTurn(data);		break;
		case kStateYourTurn:		OnUpdateYourTurn(data);		break;
		case kStateGameEnd:			OnUpdateGameEnd(data);		break;
		case kStateGameCanceled:	OnUpdateGameCanceled(data);	break;
		default:
			assert(0);
			return false;
		}
		return true;
	}
	return false;
}

void TicTacToeGameScene::OnEnterSetPlayers(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterSetPlayers()");

	AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());

	rapidjson::Document playerData;
	playerData.SetObject();
	playerData.AddMember("type", "tictactoe", playerData.GetAllocator());
	playerData.AddMember("name", app->GetMyName(), playerData.GetAllocator());

	app->Send(playerData);

}

void TicTacToeGameScene::OnUpdateSetPlayers(rapidjson::Document& data)
{
	assert(data["subtype"].IsString());	
	std::string subtype(data["subtype"].GetString());

	if (subtype == "setplayers")
	{
		assert(data["player1_name"].IsString());	
		assert(data["player2_name"].IsString());	

		mPlayers[kPlayer1].name = data["player1_name"].GetString();
		mPlayers[kPlayer2].name = data["player2_name"].GetString();

		assert(data["assigned_to"].IsInt());	
		int assigend = data["assigned_to"].GetInt();

		LOG("TicTacToeGameScene::OnUpdateSetPlayers - name1[%s], name2[%s], assigned[%d]",
				mPlayers[kPlayer1].name.c_str(), mPlayers[kPlayer2].name.c_str(), assigend);

		mMyIndex = (assigend == 1 ? kPlayer1 : kPlayer2);
		State nextState = (mMyIndex == kPlayer1 ? kStateMyTurn : kStateYourTurn);

		LOG("TicTacToeGameScene::OnUpdateSetPlayers - mMyIndex[%d], nextState[%d]", mMyIndex, nextState);
		mFSM.SetState(nextState);
	}
}

void TicTacToeGameScene::OnLeaveSetPlayers(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveSetPlayers()");
}

void TicTacToeGameScene::OnEnterMyTurn(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterMyTurn()");
}
void TicTacToeGameScene::OnUpdateMyTurn(rapidjson::Document& data){}
void TicTacToeGameScene::OnLeaveMyTurn(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveMyTurn()");
}

void TicTacToeGameScene::OnEnterYourTurn(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterYourTurn()");
}
void TicTacToeGameScene::OnUpdateYourTurn(rapidjson::Document& data){}
void TicTacToeGameScene::OnLeaveYourTurn(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveYourTurn()");
}

void TicTacToeGameScene::OnEnterGameEnd(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterGameEnd()");
}
void TicTacToeGameScene::OnUpdateGameEnd(rapidjson::Document& data){}
void TicTacToeGameScene::OnLeaveGameEnd(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveGameEnd()");
}

void TicTacToeGameScene::OnEnterGameCanceled(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterGameCanceled()");
}
void TicTacToeGameScene::OnUpdateGameCanceled(rapidjson::Document& data){}
void TicTacToeGameScene::OnLeaveGameCanceled(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveGameCanceled()");
}


//////////////////////////////////////////////////////////////////////////
// Board 
BoardLayer::BoardLayer()
{
}

BoardLayer::~BoardLayer()
{
}

bool BoardLayer::init()
{
	CCLayer::init();

	return true;
}


//////////////////////////////////////////////////////////////////////////
// Hud 
HUDLayer::HUDLayer()
	: mLabelTitle(NULL)
{
}

HUDLayer::~HUDLayer()
{
	if (mLabelTitle)
	{
		mLabelTitle->release();
		mLabelTitle = NULL;
	}
}

bool HUDLayer::init()
{
	CCLayer::init();

	CCSize size = CCDirector::sharedDirector()->getWinSize();

	// Output
	mLabelTitle = CCLabelTTF::create("TicTacToe", "Arial", 20, CCSizeMake(0, 0), kCCTextAlignmentCenter);
	mLabelTitle->retain();
	mLabelTitle->setColor(ccORANGE);
	mLabelTitle->setPosition(ccp(size.width/2, 20));
	addChild(mLabelTitle);

	return true;
}

void HUDLayer::SetTitle(const char* title)
{
}
