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
	, mWinnerIndex(kPlayer1)
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
		mBoardLayer->SetScene(this);

		//HUD
		mHUDLayer = HUDLayer::create();
		mHUDLayer->retain();
		addChild(mHUDLayer);

		mFSM.SetState(kStateSetPlayers);

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

	mHUDLayer->SetTitle("Please wait for the game to start...");
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
	else if (subtype == "result")
	{
		assert(data["winner"].IsInt());
		mWinnerIndex = static_cast<PlayerIndex>(data["winner"].GetInt() - 1);
		mFSM.SetState(kStateGameEnd);
	}
}

void TicTacToeGameScene::OnLeaveSetTurn(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveSetTurn()");
}


void TicTacToeGameScene::OnEnterMyTurn(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterMyTurn()");
	mHUDLayer->SetTitle("It's your turn. (You are [%s]).", (mMyIndex == kPlayer1 ? "O" : "X"));
	mBoardLayer->SetEnabled(true);
}

void TicTacToeGameScene::OnUpdateMyTurn(rapidjson::Document& data)
{
	CheckPlayerMove(data);
}

void TicTacToeGameScene::OnLeaveMyTurn(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveMyTurn()");
	mBoardLayer->SetEnabled(false);
}

void TicTacToeGameScene::OnEnterWait(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterWait()");
	mHUDLayer->SetTitle("Wait your turn. (You are [%s]).", (mMyIndex == kPlayer1 ? "O" : "X"));

}

void TicTacToeGameScene::OnUpdateWait(rapidjson::Document& data)
{
	CheckPlayerMove(data);
}

void TicTacToeGameScene::OnLeaveWait(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveWait()");
}

void TicTacToeGameScene::OnEnterGameEnd(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterGameEnd()");
	
	CCDirector::sharedDirector()->drawScene();

	AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());

	if (mWinnerIndex < 0)
	{
		app->ShowMsgBox("Game", "Game Over : Draw!");
	}
	else if (mMyIndex == mWinnerIndex)
	{
		app->ShowMsgBox("Game", "Game Over : You Won!");
	}
	else 
	{
		app->ShowMsgBox("Game", "Game Over : You Lost!");
	}

	app->GoToLobby();

}
void TicTacToeGameScene::OnUpdateGameEnd(rapidjson::Document& data){}
void TicTacToeGameScene::OnLeaveGameEnd(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveGameEnd()");
}

void TicTacToeGameScene::OnEnterGameCanceled(int nPrevState)
{
	LOG("TicTacToeGameScene::OnEnterGameCanceled()");
	AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());

	app->ShowMsgBox("Game", "Game Canceled!");
	app->GoToLobby();
}
void TicTacToeGameScene::OnUpdateGameCanceled(rapidjson::Document& data){}
void TicTacToeGameScene::OnLeaveGameCanceled(int nNextState)
{
	LOG("TicTacToeGameScene::OnLeaveGameCanceled()");
}

void TicTacToeGameScene::OnSymbolTouched(const TicTacToe::SymbolNode* symbol)
{
	if (mFSM.GetState() == kStateMyTurn)
	{
		if (symbol->GetSymbol() == kSymbolNone)
		{
			int row, col;
			symbol->GetRowCol(row, col);

			rapidjson::Document data;
			data.SetObject();
			data.AddMember("type", "tictactoe", data.GetAllocator());
			data.AddMember("row", row, data.GetAllocator());
			data.AddMember("col", col, data.GetAllocator());
			
			AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());
			app->Send(data);
		}
	}
}


void TicTacToeGameScene::CheckPlayerMove(rapidjson::Document& data)
{
	assert(data["subtype"].IsString());	
	std::string subtype(data["subtype"].GetString());

	if (subtype == "move")
	{
		assert(data["player"].IsInt());	
		assert(data["row"].IsInt());	
		assert(data["col"].IsInt());	

		int player = data["player"].GetInt();
		int row = data["row"].GetInt();
		int col = data["col"].GetInt();

		Symbol symbol = (player == 1 ? kSymbolOOO : kSymbolXXX);
		mBoardLayer->SetSymbol(row, col, symbol);

		mFSM.SetState(kStateSetTurn);
	}
}

//////////////////////////////////////////////////////////////////////////
// Board 
SymbolNode::SymbolNode()
	: mSymbol(kSymbolNone)
	, mRow(0)
	, mCol(0)
	, mEnabled(false)
	, mScene(NULL)
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
		//ccDrawColor4F(0.0f, 1.0f, 0.0f, 1.0f);
		//ccDrawRect(ccp(0,0), ccp(size.width,size.height));
		return;

	case kSymbolOOO:
	    ccDrawColor4F(1.0f, 1.0f, 0.0f, 1.0f);
		ccDrawCircle(ccp(size.width/2,size.height/2), min(size.width/2, size.height/2), CC_DEGREES_TO_RADIANS(360), 50, false);
		return;

	case kSymbolXXX:
	    ccDrawColor4F(1.0f, 0.0f, 0.0f, 1.0f);
		ccDrawLine(ccp(0,0), ccp(size.width,size.height));
		ccDrawLine(ccp(size.width,0), ccp(0,size.height));
		return;

	default:
		assert(0);
		return;
	}
}

void SymbolNode::onEnter()
{
	CCObject* test = dynamic_cast<CCObject*>(this);
	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);	
	CCNode::onEnter();
}

void SymbolNode::onExit()
{
	CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
	CCNode::onExit();
}


bool SymbolNode::ccTouchBegan(CCTouch* touch, CCEvent* event)
{
	return mEnabled && containsTouchLocation(touch);
}

void SymbolNode::ccTouchMoved(CCTouch* touch, CCEvent* event)
{
}

void SymbolNode::ccTouchEnded(CCTouch* touch, CCEvent* event)
{
	LOG("TicTacToeGameScene::ccTouchEnded() - (%d,%d) - %d", mRow, mCol, mSymbol);
	mScene->OnSymbolTouched(this);
}

bool SymbolNode::containsTouchLocation(CCTouch* touch)
{
    CCSize s = getContentSize();
    CCRect rect = CCRectMake(-s.width / 2, -s.height / 2, s.width, s.height);
    return rect.containsPoint(convertTouchToNodeSpaceAR(touch));
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
			mBoard[row][col]->release();
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

			newSymbol->SetSymbol(kSymbolNone);
			newSymbol->SetRowCol(row, col);
			newSymbol->SetEnabled(false);

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


void BoardLayer::SetScene(TicTacToeGameScene* scene)
{
	for (int row = 0 ; row < kCellRows ; ++row)
	{
		for (int col = 0 ; col < kCellColumns ; ++col)
		{
			mBoard[row][col]->SetScene(scene);
		}
	}
}


void BoardLayer::SetEnabled(bool enabled)
{
	for (int row = 0 ; row < kCellRows ; ++row)
	{
		for (int col = 0 ; col < kCellColumns ; ++col)
		{
			mBoard[row][col]->SetEnabled(enabled);
		}
	}
}

void BoardLayer::SetSymbol(int row, int col, Symbol symbol)
{
	assert(row >= 0 && row < kCellRows);
	assert(col >= 0 && col < kCellColumns);

	mBoard[row][col]->SetSymbol(symbol);
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

void HUDLayer::SetTitle(const char* title, ...)
{
	char buffer[128] = {0,};
	va_list args;
	va_start(args, title);
	vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer)-1, title, args);
	va_end(args);

	mLabelTitle->setString(buffer);
}
