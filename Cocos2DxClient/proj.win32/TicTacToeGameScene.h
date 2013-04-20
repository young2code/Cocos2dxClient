#pragma once
#include "cocos2d.h"
#include <rapidjson\document.h>
#include <string>
#include "FSM.h"

namespace cocos2d
{
	namespace extension
	{
		class CCEditBox;
	}
	class CCLabelTTF;
}

namespace TicTacToe
{
	enum Symbol
	{
		kSymbolNone = 0, 
		kSymbolOOO,
		kSymbolXXX, 
	};

	enum CellCount
	{
		kCellRows = 3,
		kCellColumns = 3,
	};

	class SymbolNode : public cocos2d::CCNode
	{
	public:
		SymbolNode();
		virtual ~SymbolNode();

		virtual bool init();  
		virtual void draw();

		void SetSymbol(Symbol symbol) { mSymbol = symbol; }

		// implement the "static node()" method manually
	    CREATE_FUNC(SymbolNode);

	private:
		Symbol mSymbol;
	};

	class BoardLayer : public cocos2d::CCLayer
	{
	public:
		BoardLayer();
		virtual ~BoardLayer();

		virtual bool init();  
		virtual void draw();

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

		void SetTitle(const char* title);

	    // implement the "static node()" method manually
	    CREATE_FUNC(HUDLayer);

	private:
			cocos2d::CCLabelTTF* mLabelTitle;
	};
};

class TicTacToeGameScene : public cocos2d::CCScene
{
public:
	TicTacToeGameScene();
	virtual ~TicTacToeGameScene();

    virtual bool init();  

    void menuEchoCallback(CCObject* pSender);

	// network
	bool OnRecv(rapidjson::Document& data);

    // implement the "static node()" method manually
    CREATE_FUNC(TicTacToeGameScene);

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

	TicTacToe::BoardLayer* mBoardLayer;
	TicTacToe::HUDLayer* mHUDLayer;
};

