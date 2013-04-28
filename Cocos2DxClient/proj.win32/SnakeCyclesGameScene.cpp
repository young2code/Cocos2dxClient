#include "SnakeCyclesGameScene.h"
#include "AppDelegate.h"
#include <boost/bind.hpp>
#include "Log.h"

using namespace cocos2d;
using namespace SnakeCycles;


SnakeCyclesGameScene::SnakeCyclesGameScene()
	: mMyIndex(kPlayerMax)
	, mWinnerIndex(kPlayerMax)
	, mBoardLayer(NULL)
	, mHUDLayer(NULL)
{
	InitFSM();
}

SnakeCyclesGameScene::~SnakeCyclesGameScene()
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
bool SnakeCyclesGameScene::init()
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

		mFSM.SetState(kStateWait);

        bRet = true;
    } while (0);

    return bRet;
}


void SnakeCyclesGameScene::InitFSM()
{
	// we use dummy update as we want to update with network recv events;
#define BIND_CALLBACKS(State) boost::bind(&SnakeCyclesGameScene::OnEnter##State, this, _1), \
							  boost::bind(&SnakeCyclesGameScene::DummyUpdate, this), \
							  boost::bind(&SnakeCyclesGameScene::OnLeave##State, this, _1)

	mFSM.RegisterState(kStateWait, BIND_CALLBACKS(Wait));
	mFSM.RegisterState(kStatePlay, BIND_CALLBACKS(Play));
	mFSM.RegisterState(kStateEnd, BIND_CALLBACKS(End));

#undef BIND_CALLBACKS
}


void SnakeCyclesGameScene::ShutdownFSM()
{
	mFSM.Reset(false);
}

void SnakeCyclesGameScene::OnRecv(rapidjson::Document& data)
{
	assert(data["type"].IsString());	
	std::string type(data["type"].GetString());

	if (type == "snakecycles")
	{            
		if (CheckWinner(data))
		{
			return;
		}

		switch(mFSM.GetState())
		{
		case kStateWait:	OnUpdateWait(data);		break;
		case kStatePlay:	OnUpdatePlay(data);		break;
		case kStateEnd:		OnUpdateEnd(data);		break;

		default:
			assert(0);
			return;
		}
	}
}


bool SnakeCyclesGameScene::CheckWinner(rapidjson::Document& data)
{
	assert(data["subtype"].IsString());	
	std::string subtype(data["subtype"].GetString());

	if (subtype == "winner")
	{
		assert(data["winner"].IsInt());	
		mWinnerIndex = static_cast<PlayerIndex>(data["winner"].GetInt());

		mFSM.SetState(kStateEnd);

		return true;
	}
	return false;
}


// Wait
void SnakeCyclesGameScene::OnEnterWait(int nPrevState)
{
	LOG("SnakeCyclesGameScene::OnEnterWait()");
	mHUDLayer->SetTitle("Wait for other players..");
}

void SnakeCyclesGameScene::OnUpdateWait(rapidjson::Document& data)
{
	assert(data["subtype"].IsString());	
	std::string subtype(data["subtype"].GetString());

	if (subtype == "countdown")
	{
		assert(data["number"].IsInt());	
		int number = data["number"].GetInt();
		mHUDLayer->SetTitle("Countdown : %d", number);
	}
	else if(subtype == "play")
	{
		// Init game data here..

		mFSM.SetState(kStatePlay);
	}
}

void SnakeCyclesGameScene::OnLeaveWait(int nNextState)
{
	LOG("SnakeCyclesGameScene::OnLeaveWait()");
}


// Play
void SnakeCyclesGameScene::OnEnterPlay(int nPrevState)
{
	LOG("SnakeCyclesGameScene::OnEnterPlay()");
	mHUDLayer->SetTitle("Play!");
}

void SnakeCyclesGameScene::OnUpdatePlay(rapidjson::Document& data)
{
}

void SnakeCyclesGameScene::OnLeavePlay(int nNextState)
{
	LOG("SnakeCyclesGameScene::OnLeavePlay()");
}


// End
void SnakeCyclesGameScene::OnEnterEnd(int nPrevState)
{
	LOG("SnakeCyclesGameScene::OnEnterEnd()");

	if (mWinnerIndex != kPlayerMax)
	{
		mHUDLayer->SetTitle("Winner : %d", mWinnerIndex);
	}
	else
	{
		mHUDLayer->SetTitle("No Winner");
	}
}

void SnakeCyclesGameScene::OnUpdateEnd(rapidjson::Document& data)
{
	assert(data["subtype"].IsString());	
	std::string subtype(data["subtype"].GetString());

	if (subtype == "wait")
	{
		mFSM.SetState(kStateWait);
	}
}

void SnakeCyclesGameScene::OnLeaveEnd(int nNextState)
{
	LOG("SnakeCyclesGameScene::OnLeaveEnd()");
}




//////////////////////////////////////////////////////////////////////////
// Board 
SymbolNode::SymbolNode()
	: mSymbol(kSymbolNone)
	, mColor(kNone)
	, mRow(0)
	, mCol(0)
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

	switch(mColor)
	{
	case kRed:		ccDrawColor4F(1.0f, 0.0f, 0.0f, 1.0f); break;
	case kBlue:		ccDrawColor4F(0.0f, 0.0f, 1.0f, 1.0f); break;
	case kGreen:	ccDrawColor4F(0.0f, 1.0f, 0.0f, 1.0f); break;
	case kWhite:	ccDrawColor4F(1.0f, 1.0f, 1.0f, 1.0f); break;

	case kNone:
		break;

	default:
		assert(0);
		return;
	}

	switch(mSymbol)
	{
	case kSymbolNone:
		return;

	case kSymbolHead:
		ccDrawCircle(ccp(size.width/2,size.height/2), min(size.width/2, size.height/2), CC_DEGREES_TO_RADIANS(360), 50, false);
		return;

	case kSymbolWall:	    
		ccDrawRect(ccp(0,0), ccp(size.width,size.height));
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

			newSymbol->SetSymbol(kSymbolNone);
			newSymbol->SetRowCol(rowReal, col);

			mBoard[rowReal][col] = newSymbol;
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


void BoardLayer::SetScene(SnakeCyclesGameScene* scene)
{
	for (int row = 0 ; row < kCellRows ; ++row)
	{
		for (int col = 0 ; col < kCellColumns ; ++col)
		{
			mBoard[row][col]->SetScene(scene);
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
	mLabelTitle = CCLabelTTF::create("SnakeCycles", "Arial", 20, CCSizeMake(0, 0), kCCTextAlignmentCenter);
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
