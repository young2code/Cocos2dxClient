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
	enum Symbol
	{
		kSymbolNone = 0, 
		kSymbolHead,
		kSymbolWall, 
	};
		
	enum
	{
		kCellRows = 50,
		kCellColumns = 50,
	};

	enum Color
	{	
		kRed = 0,	// kPlayer1
		kBlue,		// kPlayer2
		kGreen,		// kPlayer3
		kWhite,		// kPlayer4

		kNone,
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
		void SetSymbol(int row, int col, Symbol symbol);

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
public:
	SnakeCyclesGameScene();
	virtual ~SnakeCyclesGameScene();

    virtual bool init();  

    void menuEchoCallback(CCObject* pSender);

	void OnRecv(rapidjson::Document& data);

	void OnSymbolTouched(const SnakeCycles::SymbolNode* symbol);

    // implement the "static node()" method manually
    CREATE_FUNC(SnakeCyclesGameScene);

private:
	void InitFSM();
	void ShutdownFSM();

	void OnEnterWait(int nPrevState);
	void OnUpdateWait(rapidjson::Document& data);
	void OnLeaveWait(int nNextState);

	void OnEnterPlay(int nPrevState);
	void OnUpdatePlay(rapidjson::Document& data);
	void OnLeavePlay(int nNextState);

	void OnEnterEnd(int nPrevState);
	void OnUpdateEnd(rapidjson::Document& data);
	void OnLeaveEnd(int nNextState);

	void DummyUpdate(){}

	bool CheckWinner(rapidjson::Document& data);

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

		kPlayerMax,
	};

	struct Player
	{
		std::string name;
	};

	FSM mFSM;

	Player mPlayers[kPlayerMax];
	PlayerIndex mMyIndex;
	PlayerIndex mWinnerIndex;

	SnakeCycles::BoardLayer* mBoardLayer;
	SnakeCycles::HUDLayer* mHUDLayer;
};

