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
	class BoardLayer : public cocos2d::CCLayer
	{
	public:
		BoardLayer();
		virtual ~BoardLayer();

		virtual bool init();  

	    // implement the "static node()" method manually
	    CREATE_FUNC(BoardLayer);
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

	void OnEnterMyTurn(int nPrevState);
	void OnUpdateMyTurn(rapidjson::Document& data);
	void OnLeaveMyTurn(int nNextState);

	void OnEnterYourTurn(int nPrevState);
	void OnUpdateYourTurn(rapidjson::Document& data);
	void OnLeaveYourTurn(int nNextState);

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
		kStateMyTurn,
		kStateYourTurn,
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
	PlayerIndex mCurTurn;

	TicTacToe::BoardLayer* mBoardLayer;
	TicTacToe::HUDLayer* mHUDLayer;
};

