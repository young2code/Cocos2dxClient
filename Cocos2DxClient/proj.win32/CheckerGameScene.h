#pragma once
#include "cocos2d.h"
#include <rapidjson\document.h>
#include <string>
#include "FSM.h"

namespace cocos2d
{
	class CCLabelTTF;
}

class CheckerGameScene;

namespace Checker
{
	enum Symbol
	{
		kSymbolNone = 0, 
		kSymbolWhite,
		kSymbolRed, 
	};

	enum CellCount
	{
		kCellRows = 8,
		kCellColumns = 8,
	};

	class SymbolNode : public cocos2d::CCNode, public cocos2d::CCTargetedTouchDelegate
	{
	public:
		SymbolNode();
		virtual ~SymbolNode();

		virtual bool init();  
		virtual void draw();

		virtual void onEnter();
		virtual void onExit();

		virtual bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event);
		virtual void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event);
		virtual void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event);

		void SetScene(CheckerGameScene* scene) { mScene = scene; }

		void SetIndex(int index) { mIndex = index; }
		int GetIndex() const { return mIndex; }

		void SetSymbol(Symbol symbol) { mSymbol = symbol; }
		Symbol GetSymbol() const { return mSymbol; }

		void SetEnabled(bool enabled) { mEnabled = enabled; }
		void SetSelected(bool selected) { mSelected = selected; }

		// implement the "static node()" method manually
	    CREATE_FUNC(SymbolNode);

	private:
		bool containsTouchLocation(cocos2d::CCTouch* touch);

	private:
		Symbol mSymbol;
		int mIndex;
		bool mEnabled;
		bool mSelected;

		CheckerGameScene* mScene;
	};

	class BoardLayer : public cocos2d::CCLayer
	{
	public:
		BoardLayer();
		virtual ~BoardLayer();

		virtual bool init();  
		virtual void draw();

		void SetEnabled(bool enabled);
		void SetScene(CheckerGameScene* scene);
		void SetSymbol(int index, Symbol symbol);

	    // implement the "static node()" method manually
	    CREATE_FUNC(BoardLayer);

	private:
		SymbolNode* mBoard[kCellRows*kCellColumns];
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

class CheckerGameScene : public cocos2d::CCScene
{
public:
	CheckerGameScene();
	virtual ~CheckerGameScene();

    virtual bool init();  

    void menuEchoCallback(CCObject* pSender);

	void OnRecv(rapidjson::Document& data);

	void OnSymbolTouched(Checker::SymbolNode* symbol);

    // implement the "static node()" method manually
    CREATE_FUNC(CheckerGameScene);

private:
	void InitFSM();
	void ShutdownFSM();

	void OnEnterSetPlayers(int nPrevState);
	void OnUpdateSetPlayers(rapidjson::Document& data);
	void OnLeaveSetPlayers(int nNextState);

	void OnEnterSetTurn(int nPrevState);
	void OnUpdateSetTurn(rapidjson::Document& data);
	void OnLeaveSetTurn(int nNextState);

	void OnEnterMyTurn(int nPrevState);
	void OnUpdateMyTurn(rapidjson::Document& data);
	void OnLeaveMyTurn(int nNextState);

	void OnEnterWait(int nPrevState);
	void OnUpdateWait(rapidjson::Document& data);
	void OnLeaveWait(int nNextState);

	void OnEnterGameEnd(int nPrevState);
	void OnUpdateGameEnd(rapidjson::Document& data);
	void OnLeaveGameEnd(int nNextState);

	void OnEnterGameCanceled(int nPrevState);
	void OnUpdateGameCanceled(rapidjson::Document& data);
	void OnLeaveGameCanceled(int nNextState);

	void DummyUpdate(){}

	void CheckPlayerMove(rapidjson::Document& data);

private:
	enum State
	{
		kStateSetPlayers,
		kStateSetTurn,
		kStateMyTurn,
		kStateWait,
		kStateGameEnd,
		kStateGameCanceled,
	};

	enum PlayerIndex
	{
		kPlayer1 = 0,
		kPlayer2 = 1,

		kPlayerCount,
	};

	struct Player
	{
		std::string name;
	};

	FSM mFSM;

	Player mPlayers[kPlayerCount];
	PlayerIndex mMyIndex;
	PlayerIndex mWinnerIndex;

	Checker::BoardLayer* mBoardLayer;
	Checker::HUDLayer* mHUDLayer;
	Checker::SymbolNode* mSymbolSelected;
};

