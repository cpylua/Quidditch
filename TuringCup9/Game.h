#ifndef		__GAME_H__
#define		__GAME_H__

#include <vector>
#include "Engine/Graphics.h"
#include "ResourceManager.h"
#include "GlobalDef.h"
#include "Hero.h"

using namespace std;

const int GAME_STATE_NUM = 6;

struct Gate
{
	int x;
	int y_upper;
	int y_lower;
};

extern Gate g_Gate[AI_NUM];

enum Game_State
{
	WAITFORSTART,
	READY,
	GAMING,
	PAUSE,
	END,
	NEWTURN,
};

struct fwpos
{
	float x;
	float y;
};

class cGame
{
	typedef void (*FSM)();
	typedef void (*RSM)();
	typedef void (*Func)();
public:
	cGame();
	~cGame();

	static		void		Render();
	static		void		RenderWaitForStart();
	static		void		RenderReady();
	static		void		RenderGaming();
	static		void		RenderPause();
	static		void		RenderNewTurn();
	static		void		RenderEnd();

	static		void		RenderBackGround();
	static		void		RenderTeamName();
	static		void		RenderGameTime();

	static		void		Initial(HWND p_hWnd, HWND hwndDlg);
	static		void		CreateGameHeros();
	static		void		LoadInformationFromFile();

	static		void		CleanUp();
	static		void		GameLoop();

	static		void		WaitForStart();
	static		void		Ready();
	static		void		Gaming();
	static		void		Pause();
	static		void		NewTurn();
	static		void		End();
	static		void		LoadAllRes();
	static		void		ReleaseAllRes();

	static		cResourceManager*	GetResourceManager() { return &s_ResourceManager; }
	static		int FindHeroTeam(cHero *p_pHero);
	static		int FindBall(cBall *p_pBall);


private:
	static		cResourceManager	s_ResourceManager;
	static		cGraphics		s_Graphics;

	static		Game_State		s_currState;
	static		FSM				s_FSM[GAME_STATE_NUM];
	static		RSM				s_RSM[GAME_STATE_NUM];
	static		cMap*			s_pMap;

	static		void CreateBalls();
};

void __End();

#endif