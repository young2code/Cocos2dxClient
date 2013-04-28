#pragma once
#include "cocos2d.h"
#include <rapidjson\document.h>
#include <string>
#include "FSM.h"

namespace cocos2d
{
	class CCLabelTTF;
}

class SnakeCyclesGameScene;

namespace SnakeCycles
{
	enum
	{
		kCellRows = 20,
		kCellColumns = 20,
	};

	enum Symbol
	{
		kSymbolNone = 0, 
		kSymbolHead,
		kSymbolWall, 
	};
		
	enum Color
	{	
		kColorRed = 0,		// kPlayer1
		kColorBlue,			// kPlayer2
		kColorGreen,		// kPlayer3
		kColorWhite,		// kPlayer4

		kColorNone,
	};

	class SymbolNode : public cocos2d::CCNode
	{
	public:
		SymbolNode();
		virtual ~SymbolNode();

		virtual bool init();  
		virtual void draw();

		void SetScene(SnakeCyclesGameScene* scene) { mScene = scene; }

		void SetRowCol(int row, int col) { mRow = row; mCol = col; }
		void GetRowCol(int& row, int& col) const { row = mRow; col = mCol; }

		void SetSymbol(Symbol symbol) { mSymbol = symbol; }
		Symbol GetSymbol() const { return mSymbol; }

		void SetColor(Color color) { mColor = color; }
		Color GetColor() const { return mColor; }

		// implement the "static node()" method manually
	    CREATE_FUNC(SymbolNode);

	private:
		bool containsTouchLocation(cocos2d::CCTouch* touch);

	private:
		Symbol mSymbol;
		Color mColor;
		int mRow;
		int mCol;

		SnakeCyclesGameScene* mScene;
	};

	class BoardLayer : public cocos2d::CCLayer
	{
	public:
		BoardLayer();
		virtual ~BoardLayer();

		virtual bool init();  
		virtual void draw();

		void SetScene(SnakeCyclesGameScene* scene);
		void SetSymbol(int row, int col, Symbol symbol, Color color);
		void ResetSymbols(Color color);

	    // implement the "static node()" method manually
	    CREATE_FUNC(BoardLayer);

	private:
		SymbolNode* mBoard[kCellRows][kCellColumns];
	};

	class HUDLayer : public cocos2d::CCLayer
	{
	public:
		HUDLayer();
		virtual ~HUDLayer();

		virtual bool init();  

		void SetTitle(const char* title, ...);

	    // implement the "static node()" method manually
	    CREATE_FUNC(HUDLayer);

	private:
			cocos2d::CCLabelTTF* mLabelTitle;
	};
};

class SnakeCyclesGameScene : public cocos2d::CCScene
{
private:
	enum State
	{
		kStateWait,
		kStatePlay,
		kStateEnd,
	};

	enum PlayerIndex
	{
		kPlayer1 = 0,
		kPlayer2,
		kPlayer3,
		kPlayer4,

		kPlayerNone,
	};

	enum PlayerState
	{
		kPlayerAlive = 0,
		kPlayerDead,
	};

	enum Direction
	{
		kUP = 0,
		kDOWN,
		kLEFT,
		kRIGHT,
	};

public:
	SnakeCyclesGameScene();
	virtual ~SnakeCyclesGameScene();

    virtual bool init();  

    void menuEchoCallback(CCObject* pSender);

	void OnRecv(rapidjson::Document& data);

	void Update();

    // implement the "static node()" method manually
    CREATE_FUNC(SnakeCyclesGameScene);

private:
	void InitFSM();
	void ShutdownFSM();

	void OnEnterWait(int nPrevState);
	void OnRecvWait(rapidjson::Document& data);
	void OnLeaveWait(int nNextState);

	void OnEnterPlay(int nPrevState);
	void OnRecvPlay(rapidjson::Document& data);
	void OnUpdatePlay();
	void OnLeavePlay(int nNextState);

	void OnEnterEnd(int nPrevState);
	void OnRecvEnd(rapidjson::Document& data);
	void OnLeaveEnd(int nNextState);

	void DummyUpdate(){}

	void UpdatePlayer(rapidjson::Value& player);
	void UpdateSymbol(rapidjson::Value& wall);

	void SendDirection(Direction dir);

	bool CheckWinner(rapidjson::Document& data);

private:
	FSM mFSM;

	PlayerIndex mMyIndex;
	PlayerState mMyState;
	Direction mMyDirection;
	PlayerIndex mWinnerIndex;

	SnakeCycles::BoardLayer* mBoardLayer;
	SnakeCycles::HUDLayer* mHUDLayer;
};

