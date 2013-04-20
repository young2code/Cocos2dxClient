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
	mFSM.RegisterState(kStateSetTurn, BIND_CALLBACKS(SetTurn));
	mFSM.RegisterState(kStateMyTurn, BIND_CALLBACKS(MyTurn));
	mFSM.RegisterState(kStateWait, BIND_CALLBACKS(Wait));
	mFSM.RegisterState(kStateGameEnd, BIND_CALLBACKS(GameEnd));
	mFSM.RegisterState(kStateGameCanceled, BIND_CALLBACKS(GameCanceled));

#undef BIND_CALLBACKS

	//mFSM.SetState(kStateSetPlayers);
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
		case kStateSetTurn:			OnUpdateSetTurn(data);		break;
		case kStateMyTurn:			OnUpdateMyTurn(data);		break;
		case kStateWait:			OnUpdateWait(data);			break;
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

		mFSM.SetState(kStateSetTurn);
	}
}

void TicTacToeGameScene::OnLeaveSetPlayers(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveSetPlayers()");
}

void TicTacToeGameScene::OnEnterSetTurn(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterSetTurn()");
}

void TicTacToeGameScene::OnUpdateSetTurn(rapidjson::Document& data)
{
	assert(data["subtype"].IsString());	
	std::string subtype(data["subtype"].GetString());

	if (subtype == "setturn")
	{
		assert(data["player"].IsInt());
		PlayerIndex curPlayerIndex = static_cast<PlayerIndex>(data["player"].GetInt() - 1);

		if (curPlayerIndex == mMyIndex)
		{
			mFSM.SetState(kStateMyTurn);
		}
		else
		{
			mFSM.SetState(kStateWait);
		}
	}
}

void TicTacToeGameScene::OnLeaveSetTurn(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveSetTurn()");
}


void TicTacToeGameScene::OnEnterMyTurn(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterMyTurn()");
	mHUDLayer->SetTitle("It's your turn.");
}
void TicTacToeGameScene::OnUpdateMyTurn(rapidjson::Document& data){}
void TicTacToeGameScene::OnLeaveMyTurn(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveMyTurn()");
}

void TicTacToeGameScene::OnEnterWait(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterWait()");
	mHUDLayer->SetTitle("Wait your turn.");

}
void TicTacToeGameScene::OnUpdateWait(rapidjson::Document& data){}
void TicTacToeGameScene::OnLeaveWait(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveWait()");
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
SymbolNode::SymbolNode()
	: mSymbol(kSymbolNone)
{
}


SymbolNode::~SymbolNode()
{
}

bool SymbolNode::init()
{
	CCNode::init();
	return true;
}

void SymbolNode::draw()
{
	CCSize size = getContentSize();

	switch(mSymbol)
	{
	case kSymbolNone:
	    ccDrawColor4F(0.0f, 1.0f, 0.0f, 1.0f);
		ccDrawRect(ccp(0,0), ccp(size.width,size.height));
		return;

	case kSymbolOOO:
		return;

	case kSymbolXXX:
		return;

	default:
		assert(0);
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
// Board 
BoardLayer::BoardLayer()
{
	for (int row = 0 ; row < kCellRows ; ++row)
	{
		for (int col = 0 ; col < kCellColumns ; ++col)
		{
			mBoard[row][col] = NULL;
		}
	}
}

BoardLayer::~BoardLayer()
{
	for (int row = 0 ; row < kCellRows ; ++row)
	{
		for (int col = 0 ; col < kCellColumns ; ++col)
		{
			SymbolNode* symbol = mBoard[row][col];
			symbol->release();
		}
	}
}

bool BoardLayer::init()
{
	CCLayer::init();

	const float scale = 0.8f;

	CCSize size = CCDirector::sharedDirector()->getWinSize();

	float vertOffset = size.width / kCellColumns;
	float horzOffset = size.height / kCellRows;


	for (int row = 0 ; row < kCellRows ; ++row)
	{
		for (int col = 0 ; col < kCellColumns ; ++col)
		{
			SymbolNode* newSymbol = SymbolNode::create();
			newSymbol->retain();
			newSymbol->setScale(scale);
			newSymbol->setAnchorPoint(ccp(0.5f, 0.5f));
			newSymbol->setContentSize(CCSize(vertOffset, horzOffset));
			newSymbol->setPosition(col*vertOffset + vertOffset/2.0f, row*horzOffset + horzOffset/2.0f);
			addChild(newSymbol);
			mBoard[row][col] = newSymbol;
		}
	}

	setScale(scale);
	setAnchorPoint(ccp(0.5f, 0.5f));

	return true;
}

void BoardLayer::draw()
{
	CCSize size = CCDirector::sharedDirector()->getWinSize();

	float vertOffset = size.width / kCellColumns;
	float horzOffset = size.height / kCellRows;

    ccDrawColor4F(1.0f, 0.0f, 0.5f, 1.0f);
	for (int row = 0 ; row <= kCellRows ; ++row)
	{
		float y = row * horzOffset;
	    ccDrawLine(ccp(0, y), ccp(size.width, y));

		for (int col = 0 ; col <= kCellColumns ; ++col)
		{
			float x = col * vertOffset;
			ccDrawLine(ccp(x, 0), ccp(x, size.height));
		}
	}
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
	mLabelTitle->setString(title);
}
