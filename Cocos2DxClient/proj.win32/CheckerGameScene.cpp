#include "CheckerGameScene.h"
#include "AppDelegate.h"
#include <boost/bind.hpp>
#include "Log.h"

using namespace cocos2d;
using namespace Checker;


CheckerGameScene::CheckerGameScene()
	: mMyIndex(kPlayer1)
	, mWinnerIndex(kPlayer1)
	, mBoardLayer(NULL)
	, mHUDLayer(NULL)
	, mSymbolSelected(NULL)
{
	InitFSM();
}

CheckerGameScene::~CheckerGameScene()
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
bool CheckerGameScene::init()
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


void CheckerGameScene::InitFSM()
{
	// we use dummy update as we want to update with network recv events;
#define BIND_CALLBACKS(State) boost::bind(&CheckerGameScene::OnEnter##State, this, _1), \
							  boost::bind(&CheckerGameScene::DummyUpdate, this), \
							  boost::bind(&CheckerGameScene::OnLeave##State, this, _1)

	mFSM.RegisterState(kStateSetPlayers, BIND_CALLBACKS(SetPlayers));
	mFSM.RegisterState(kStateSetTurn, BIND_CALLBACKS(SetTurn));
	mFSM.RegisterState(kStateMyTurn, BIND_CALLBACKS(MyTurn));
	mFSM.RegisterState(kStateWait, BIND_CALLBACKS(Wait));
	mFSM.RegisterState(kStateGameEnd, BIND_CALLBACKS(GameEnd));
	mFSM.RegisterState(kStateGameCanceled, BIND_CALLBACKS(GameCanceled));

#undef BIND_CALLBACKS
}


void CheckerGameScene::ShutdownFSM()
{
	mFSM.Reset(false);
}

void CheckerGameScene::OnRecv(rapidjson::Document& data)
{
	assert(data["type"].IsString());	
	std::string type(data["type"].GetString());

	if (type == "checker")
	{
		assert(data["subtype"].IsString());	
		std::string subtype(data["subtype"].GetString());
		if (subtype == "canceled")
		{
			mFSM.SetState(kStateGameCanceled);
			return;
		}

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
			return;
		}
	}
}

void CheckerGameScene::OnEnterSetPlayers(int nPrevState)
{
	LOG("CheckerGameScene::OnEnterSetPlayers()");

	AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());

	rapidjson::Document playerData;
	playerData.SetObject();
	playerData.AddMember("type", "checker", playerData.GetAllocator());
	playerData.AddMember("name", app->GetMyName(), playerData.GetAllocator());

	app->Send(playerData);

	mHUDLayer->SetTitle("Please wait for the game to start...");
}

void CheckerGameScene::OnUpdateSetPlayers(rapidjson::Document& data)
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

		LOG("CheckerGameScene::OnUpdateSetPlayers - name1[%s], name2[%s], assigned[%d]",
				mPlayers[kPlayer1].name.c_str(), mPlayers[kPlayer2].name.c_str(), assigend);

		mMyIndex = static_cast<PlayerIndex>(assigend);

		assert(data["board"].IsArray());	
		rapidjson::Value& board = data["board"];
		for (int i = 0 ; i < kCellRows*kCellColumns ; ++i)
		{
			assert(board[i].IsInt());
			mBoardLayer->SetSymbol(i, static_cast<Symbol>(board[i].GetInt()));
		}

		mFSM.SetState(kStateSetTurn);
	}
}

void CheckerGameScene::OnLeaveSetPlayers(int nNextState)
{
	LOG("CheckerGameScene::OnLeaveSetPlayers()");
}

void CheckerGameScene::OnEnterSetTurn(int nPrevState)
{
	LOG("CheckerGameScene::OnEnterSetTurn()");
}

void CheckerGameScene::OnUpdateSetTurn(rapidjson::Document& data)
{
	assert(data["subtype"].IsString());	
	std::string subtype(data["subtype"].GetString());

	if (subtype == "setturn")
	{
		assert(data["player"].IsInt());
		PlayerIndex curPlayerIndex = static_cast<PlayerIndex>(data["player"].GetInt());

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
		mWinnerIndex = static_cast<PlayerIndex>(data["winner"].GetInt());
		mFSM.SetState(kStateGameEnd);
	}
}

void CheckerGameScene::OnLeaveSetTurn(int nNextState)
{
	LOG("CheckerGameScene::OnLeaveSetTurn()");
}


void CheckerGameScene::OnEnterMyTurn(int nPrevState)
{
	LOG("CheckerGameScene::OnEnterMyTurn()");
	mSymbolSelected = NULL;
	mHUDLayer->SetTitle("It's your turn. (You are [%s]).", (mMyIndex == kPlayer1 ? "White" : "Red"));
	mBoardLayer->SetEnabled(true);
}

void CheckerGameScene::OnUpdateMyTurn(rapidjson::Document& data)
{
	CheckPlayerMove(data);
}

void CheckerGameScene::OnLeaveMyTurn(int nNextState)
{
	LOG("CheckerGameScene::OnLeaveMyTurn()");
	assert(mSymbolSelected == NULL);
	mBoardLayer->SetEnabled(false);
}

void CheckerGameScene::OnEnterWait(int nPrevState)
{
	LOG("CheckerGameScene::OnEnterWait()");
	mHUDLayer->SetTitle("Wait your turn. (You are [%s]).", (mMyIndex == kPlayer1 ? "White" : "Red"));

}

void CheckerGameScene::OnUpdateWait(rapidjson::Document& data)
{
	CheckPlayerMove(data);
}

void CheckerGameScene::OnLeaveWait(int nNextState)
{
	LOG("CheckerGameScene::OnLeaveWait()");
}

void CheckerGameScene::OnEnterGameEnd(int nPrevState)
{
	LOG("CheckerGameScene::OnEnterGameEnd()");
	
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
void CheckerGameScene::OnUpdateGameEnd(rapidjson::Document& data){}
void CheckerGameScene::OnLeaveGameEnd(int nNextState)
{
	LOG("CheckerGameScene::OnLeaveGameEnd()");
}

void CheckerGameScene::OnEnterGameCanceled(int nPrevState)
{
	LOG("CheckerGameScene::OnEnterGameCanceled()");
	AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());

	app->ShowMsgBox("Game", "Game Canceled!");
	app->GoToLobby();
}
void CheckerGameScene::OnUpdateGameCanceled(rapidjson::Document& data){}
void CheckerGameScene::OnLeaveGameCanceled(int nNextState)
{
	LOG("CheckerGameScene::OnLeaveGameCanceled()");
}

void CheckerGameScene::OnSymbolTouched(Checker::SymbolNode* symbol)
{
	if (mFSM.GetState() == kStateMyTurn)
	{
		Checker::Symbol mySymbol = (mMyIndex == kPlayer1 ? Checker::kSymbolWhite : Checker::kSymbolRed);

		if (symbol->GetSymbol() == mySymbol && mSymbolSelected != symbol)
		{
			if (mSymbolSelected)
			{
				mSymbolSelected->SetSelected(false);
			}
			mSymbolSelected = symbol;
			mSymbolSelected->SetSelected(true);
		}
		else if (mSymbolSelected && symbol->GetSymbol() == kSymbolNone)
		{
			rapidjson::Document data;
			data.SetObject();
			data.AddMember("type", "checker", data.GetAllocator());
			data.AddMember("from", mSymbolSelected->GetIndex(), data.GetAllocator());
			data.AddMember("to", symbol->GetIndex(), data.GetAllocator());

			mSymbolSelected->SetSelected(false);
			mSymbolSelected = NULL;
			
			AppDelegate* app = static_cast<AppDelegate*>(CCApplication::sharedApplication());
			app->Send(data);
		}
	}
}


void CheckerGameScene::CheckPlayerMove(rapidjson::Document& data)
{
	assert(data["subtype"].IsString());	
	std::string subtype(data["subtype"].GetString());

	if (subtype == "move")
	{
		assert(data["player"].IsInt());	
		assert(data["from"].IsInt());	
		assert(data["to"].IsInt());	
		assert(data["victim"].IsInt());	

		int player = data["player"].GetInt();
		int from = data["from"].GetInt();
		int to = data["to"].GetInt();
		int victim = data["victim"].GetInt();

		Symbol toSymbol = (player == kPlayer1 ? kSymbolWhite : kSymbolRed);
		mBoardLayer->SetSymbol(from, kSymbolNone);
		mBoardLayer->SetSymbol(to, toSymbol);

		if (victim >= 0)
		{
			mBoardLayer->SetSymbol(victim, kSymbolNone);
		}
		mFSM.SetState(kStateSetTurn);
	}
}

//////////////////////////////////////////////////////////////////////////
// Board 
SymbolNode::SymbolNode()
	: mSymbol(kSymbolNone)
	, mIndex(0)
	, mEnabled(false)
	, mSelected(false)
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
		break;

	case kSymbolWhite:
	    ccDrawColor4F(1.0f, 1.0f, 1.0f, 1.0f);
		ccDrawCircle(ccp(size.width/2,size.height/2), min(size.width/2, size.height/2), CC_DEGREES_TO_RADIANS(360), 50, false);
		break;

	case kSymbolRed:
	    ccDrawColor4F(1.0f, 0.0f, 0.0f, 1.0f);
		ccDrawCircle(ccp(size.width/2,size.height/2), min(size.width/2, size.height/2), CC_DEGREES_TO_RADIANS(360), 50, false);
		break;

	default:
		assert(0);
		return;
	}

	if (mSelected)
	{
		ccDrawColor4F(0.0f, 1.0f, 0.0f, 1.0f);
		ccDrawRect(ccp(0,0), ccp(size.width,size.height));
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
	LOG("CheckerGameScene::ccTouchEnded() - (%d) - %d", mIndex, mSymbol);
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
	for (int index = 0 ; index < kCellRows*kCellColumns ; ++index)
	{
		mBoard[index] = NULL;
	}
}

BoardLayer::~BoardLayer()
{
	for (int index = 0 ; index < kCellRows*kCellColumns ; ++index)
	{
		mBoard[index]->release();
	}
}

bool BoardLayer::init()
{
	CCLayer::init();

	const float scale = 0.8f;

	CCSize size = CCDirector::sharedDirector()->getWinSize();

	float vertOffset = size.width / kCellColumns;
	float horzOffset = size.height / kCellRows;


	int rowReal = 0; 
	for (int rowPos = kCellRows-1 ; rowPos >= 0 ; --rowPos)
	{
		for (int col = 0 ; col < kCellColumns ; ++col)
		{
			SymbolNode* newSymbol = SymbolNode::create();
			newSymbol->retain();
			newSymbol->setScale(scale);
			newSymbol->setAnchorPoint(ccp(0.5f, 0.5f));
			newSymbol->setContentSize(CCSize(vertOffset, horzOffset));
			newSymbol->setPosition(col*vertOffset + vertOffset/2.0f, rowPos*horzOffset + horzOffset/2.0f);
			addChild(newSymbol);

			int index = rowReal*kCellRows + col;
			newSymbol->SetSymbol(kSymbolNone);
			newSymbol->SetIndex(index);
			newSymbol->SetEnabled(false);

			mBoard[index] = newSymbol;
		}
		++rowReal;
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


void BoardLayer::SetScene(CheckerGameScene* scene)
{
	for (int index = 0 ; index < kCellRows*kCellColumns ; ++index)
	{
		mBoard[index]->SetScene(scene);
	}
}


void BoardLayer::SetEnabled(bool enabled)
{
	for (int index = 0 ; index < kCellRows*kCellColumns ; ++index)
	{
		mBoard[index]->SetEnabled(enabled);
	}
}

void BoardLayer::SetSymbol(int index, Symbol symbol)
{
	assert(index >= 0 && index < kCellRows*kCellColumns);

	mBoard[index]->SetSymbol(symbol);
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
	mLabelTitle = CCLabelTTF::create("Checker", "Arial", 20, CCSizeMake(0, 0), kCCTextAlignmentCenter);
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
