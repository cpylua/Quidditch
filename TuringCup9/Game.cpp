#ifndef _CRT_RAND_S
#define _CRT_RAND_S		// for rand_s
#endif

#include "Game.h"
#include "TuringCup9ClientAPI.h"
#include "TuringCup9Server.h"
#include "GlobalDef.h"
#include "HeroRon.h"
#include "HeroGinny.h"
#include "HeroHermione.h"
#include "HeroMalfoy.h"
#include "HeroHarry.h"
#include "HeroVoldemort.h"
#include "cBall.h"
#include "Config.h"
#include <process.h>
#include <StrSafe.h>

#include <fstream>
using namespace std;

cResourceManager		cGame::s_ResourceManager;
cGraphics				cGame::s_Graphics;
Game_State				cGame::s_currState;
cMap*					cGame::s_pMap;
cGame::FSM				cGame::s_FSM[GAME_STATE_NUM];
cGame::RSM				cGame::s_RSM[GAME_STATE_NUM];

extern void (*g_Pattern1[2])();
extern void (*g_Pattern2[2])();
extern int g_GameMode1[2];
extern int g_GameMode2[2];
extern void ClientInit1();
extern void ClientInit2();
extern bool g_bUseOvertimeConfig;

/*
	config data
*/
// freeball
extern int FREE_BALL_SHOW_TIME_MIN;
extern int FREE_BALL_SHOW_TIME_MAX;

extern int FREE_BALL_HIDE_TIME_MIN;
extern int FREE_BALL_HIDE_TIME_MAX;

extern int FREE_BALL_SPEED;

// ghostball
extern int GHOST_BALL_SPEED;

// goldball
extern int GOLD_BALL_SPEED;
extern int GOLD_BALL_SHOW_TIME;
extern int GOLD_BALL_SLOWDOWN_STEPS;

// spell
extern int SPELLED_BY_GINNY_PERIOD;
extern int SPELLED_BY_RON_PERIOD;
extern int SPELLED_BY_HARRY_PERIOD;
extern int SPELLED_BY_FREEBALL_PERIOD;
extern int SPELLED_BY_MALFOY_PERIOD;
extern int SPELLED_BY_HERMIONE_PERIOD;
extern int SPELLED_BY_VOLDEMORT_PERIOD;
extern int SPELLED_BY_VOLDEMORT_DELTA;

// heros
extern int GINNY_SPELL_COST;
extern int GINNY_SPELL_INTERVAL;
extern int GINNY_MAX_BLUE;

extern int HERMIONE_SPELL_COST;
extern int HERMIONE_SPELL_INTERVAL;
extern int HERMIONE_MAX_BLUE;

extern int MALFOY_SPELL_COST;
extern int MALFOY_SPELL_INTERVAL;
extern int MALFOY_MAX_BLUE;

extern int RON_SPELL_COST;
extern int RON_SPELL_INTERVAL;
extern int RON_MAX_BLUE;

extern int HARRY_SPELL_COST;
extern int HARRY_SPELL_INTERVAL;
extern int HARRY_MAX_BLUE;

extern int VOLDEMORT_SPELL_COST;
extern int VOLDEMORT_SPELL_INTERVAL;
extern int VOLDEMORT_MAX_BLUE;


cFont				g_BigFont;
cFont				g_NewTurnFont;
cFont				g_TimeFont;
cFont				g_TeamNameFont;
cFont				g_HeroFont;
cFont				g_GoldBallFont;
bool				g_GameStop	= false;
bool				g_QuitCurrentGame = false;
bool				g_AfterReady;

vector<SoundEffect_Type>	g_SoundEffectIndex;
cSoundEffect*	g_pSoundEffect;


DWORD g_GameStartTime = 0;
DWORD g_TurnStartTime = 0;
DWORD g_GameTime = 0;
float g_GameEndTime = 0;

DWORD g_dwGameTime = (DWORD)90000;

int g_totalNum;
int g_LastAction;

HANDLE hEventAIs[AI_NUM];
HANDLE hEventWorkDone;
HANDLE hWorkerThreads[AI_NUM];
BOOL g_bExitThread[AI_NUM];
DWORD g_ThreadIds[AI_NUM];
//int	g_AIndex;

static int g_ScoredTeam;			// which team has won a turn
unsigned g_TeamScore[AI_NUM];		// store team scores

int g_GoldBallOwner;
cHero *g_pGoldBallOwner;
sPosition g_GoldPos;

fwpos g_FireworksPos[4];
fwpos g_SecondFireworksPos[4];
float vy[4];
float Secondvy[4];
float ay = 0.1f;
float Seconday = 0.1f;
bool g_bSecond = false;
#define FIREWORK_SIZE	256

static DWORD g_dwFlashColorTime = 0;
static int g_clrIndexTime = 0;

#define HERO_FONT_WIDTH		10
#define HERO_FONT_HEIGHT	20

#define TEAM_NAME_FONT_WIDTH	14
#define TEAM_NAME_FONT_HEIGHT	24

#define BIG_FONT_WIDTH	48
#define BIG_FONT_HEIGHT	96

#define GOLDBALL_FONT_WIDTH		24
#define GOLDBALL_FONT_HEIGHT	48

#define TIME_FONT_WIDTH		16
#define TIME_FONT_HEIGHT	24

const int GAME_SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
const int GAME_SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
const int GAME_WINDOW_WIDTH = 1024;
const int GAME_WINDOW_HEIGHT = 768;

//
// start positions
//
static int g_HeroInitPos[] = {
	0, 0 + 16,
	0, GAME_WINDOW_HEIGHT - HERO_WIDTH,
	GAME_WINDOW_WIDTH - HERO_WIDTH, 0 + 16,
	GAME_WINDOW_WIDTH - HERO_WIDTH, GAME_WINDOW_HEIGHT - HERO_WIDTH
};

//
// gate positions
//

Gate g_Gate[AI_NUM] = {
	{200, 256, 512},
	{1848, 256, 512}
};

int FindHeroIndex(cHero *p_pHero);

cGame::cGame()
{
	//
	// TODO: Tasklist test
	//
}

cGame::~cGame()
{

}


static void __ThreadProc(int i)
{
	while(!g_bExitThread[i])
	{
		WaitForSingleObject(hEventAIs[i], INFINITE);

		if( !g_bExitThread[i] )
		{
			if( i == 0 )
				g_Pattern1[g_GameMode1[0]]();
			else if( i == 1 )
				g_Pattern2[g_GameMode2[0]]();

			SetEvent(hEventWorkDone);
		}
	}
}

DWORD WINAPI ThreadProc0(PVOID pvParam)
{
	UNREFERENCED_PARAMETER(pvParam);

	__try
	{
		__ThreadProc(0);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		SetEvent(hEventWorkDone);
		return 1;
	}

	return 0;
}

DWORD WINAPI ThreadProc1(PVOID pvParam)
{
	UNREFERENCED_PARAMETER(pvParam);

	__try
	{
		__ThreadProc(1);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		SetEvent(hEventWorkDone);
		return 2;
	}

	return 0;
}

void cGame::Initial(HWND p_hWnd, HWND hwndDlg)
{
	s_FSM[0] = WaitForStart;
	s_FSM[1] = Ready;
	s_FSM[2] = Gaming;
	s_FSM[3] = Pause;
	s_FSM[4] = End;
	s_FSM[5] = NewTurn;

	s_RSM[0] = RenderWaitForStart;
	s_RSM[1] = RenderReady;
	s_RSM[2] = RenderGaming;
	s_RSM[3] = RenderPause;
	s_RSM[4] = RenderEnd;
	s_RSM[5] = RenderNewTurn;

	//
	// load resources
	//
	s_Graphics.init();
	s_Graphics.setMode( p_hWnd, true );
	s_ResourceManager.init( &s_Graphics, p_hWnd );
	s_ResourceManager.loadAllResources(hwndDlg);

	// create fonts
	g_BigFont.create( &s_Graphics, _T("Courier New"), BIG_FONT_WIDTH, BIG_FONT_HEIGHT, 2000 );
	g_GoldBallFont.create( &s_Graphics, TEXT("Courier New"), GOLDBALL_FONT_WIDTH, GOLDBALL_FONT_HEIGHT, 1000);
	g_NewTurnFont.create(&s_Graphics, _T("Courier New"), 25, 35, 2000, true);
	g_TimeFont.create(&s_Graphics, TEXT("Courier New"), TIME_FONT_WIDTH, TIME_FONT_HEIGHT, 800);
	g_TeamNameFont.create(&s_Graphics, TEXT("Courier New"), TEAM_NAME_FONT_WIDTH, TEAM_NAME_FONT_HEIGHT, 800);
	g_HeroFont.create(&s_Graphics, _T("Courier New"), HERO_FONT_WIDTH, HERO_FONT_HEIGHT, 1600);

	s_pMap = cGame::s_ResourceManager.getMap();
	s_pMap->Reset();

	// default team name
	for(int i = 0; i < AI_NUM; i++)
	{
		StringCchPrintf(g_szTeamNames[i], TEAM_NAME_LEN,
			TEXT("%s%d"), TEXT("Team"), i);
	}

	// default hero name
	for(int i = 0; i < AI_NUM * 2; i++)
	{
		StringCchPrintf(g_szHeroNames[i], HERO_NAME_LEN,
			TEXT("%s%d"), TEXT("Hero"), i);
	}

	//
	// load config file
	//
	LoadInformationFromFile();

	// 
	// create balls
	//
	CreateBalls();

	//
	// choose and create heros
	//
	if( g_pfnInit[0] != NULL )
		ClientInit1();
	if( g_pfnInit[1] != NULL )
		ClientInit2();

	// 
	// must choose two different heros
	//
	if( g_AI_HeroType[0] == g_AI_HeroType[1] )
	{
		g_QuitCurrentGame = true;
		MessageBox(p_hWnd, TEXT("You MUST choose two different heros."),
			TEXT("Sorry!"), MB_OK | MB_ICONINFORMATION);
	}

	CreateGameHeros();


	g_TeamScore[0] = g_TeamScore[1] = 0;

	g_GameStop		= false;

	s_currState	= READY;
	s_ResourceManager.playReadyMusic();

	g_GoldBallOwner = -1;
	g_pGoldBallOwner = NULL;
	g_GoldPos.x = g_GoldPos.y = -1000;

	g_clrIndexTime = 0;
	g_dwFlashColorTime = 0;

	//
	// Create events and threads
	//
	g_bExitThread[0] = g_bExitThread[1] = FALSE;

	for(int i = 0; i < AI_NUM; i++)
	{	
		hEventAIs[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if( hEventAIs[i] == NULL )
			g_QuitCurrentGame = true;
	}
	hEventWorkDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( hEventWorkDone == NULL )
		g_QuitCurrentGame = true;

	// Create threads
	hWorkerThreads[0] = BEGINTHREADEX(NULL, 0, ThreadProc0, NULL, 0, &g_ThreadIds[0]);
	hWorkerThreads[1] = BEGINTHREADEX(NULL, 0, ThreadProc1, NULL, 0, &g_ThreadIds[1]);

}

void cGame::GameLoop()
{
	if( s_Graphics.TestDevice() )
	{
		(*s_FSM[s_currState])();
		Render();
	}
}

void cGame::WaitForStart()
{
	if( KEYDOWN(VK_RETURN) )
		s_currState = READY;
}

void cGame::Ready()
{
	if( KEYDOWN(VK_RETURN) )
	{
		g_AfterReady = true;

		g_GameStartTime	 = GetTickCount();
		g_TurnStartTime = GetTickCount();
		g_GameTime = 0;

		s_currState	 = GAMING;
	
		s_ResourceManager.stopReadyMusic();
		s_ResourceManager.playBackGroundMusic();
	}
}

//
// return true if the game is over
//
bool GameOver()
{
	//
	// someone got the gold ball
	//
	cGoldBall *pGoldball = ((cGoldBall*)g_Balls[2]);
	if( pGoldball->m_pOwner != NULL )
	{
		g_GoldBallOwner = cGame::FindHeroTeam( pGoldball->m_pOwner );
		g_pGoldBallOwner = pGoldball->m_pOwner;
		pGoldball->GetPos(g_GoldPos);
		return true;
	}

	//
	// no more time
	//
	if( IsDebuggerPresent() )
		return false;

	if( g_GameTime + GetTickCount() - g_TurnStartTime >= g_dwGameTime )
		return true;

	return false;
}


#define WAIT_TIME_MS_DEBUG	 INFINITE

#define WAIT_TIME_MS_RELEASE 300


static void __CallAI(int i)
{
	static const TCHAR szTimeOut[] = 
		TEXT("[%s]'s AI is taking up too much time.\n")
		TEXT("The code may be poorly written.\n\n")
		TEXT("Current game will be terminated.\n");
	static const TCHAR szThreadDied[] = 
		TEXT("[%s]'s AI has crashed.\n")
		TEXT("TID[%u] died unexpectedly!\n\n")
		TEXT("Current game will be terminated.\n");

	UINT uType = MB_OK;
	TCHAR szMsg[256];
	szMsg[0] = TEXT('\0');

	// wake up thread
	SetEvent(hEventAIs[i]);

	DWORD dwWait, dwCode;
	if( IsDebuggerPresent() )
		dwWait = WaitForSingleObject(hEventWorkDone, WAIT_TIME_MS_DEBUG);
	else
		dwWait = WaitForSingleObject(hEventWorkDone, WAIT_TIME_MS_RELEASE);

	GetExitCodeThread(hWorkerThreads[i], &dwCode);

	if( dwWait == WAIT_OBJECT_0)
	{
		// worker thread died unexpectedly
		if( dwCode != STILL_ACTIVE && dwCode != 0 )
		{
			StringCchPrintf( szMsg, _countof(szMsg), 
				szThreadDied, g_szTeamNames[i], g_ThreadIds[i] );
			uType |= MB_ICONSTOP;
		}
	}

	if( dwWait == WAIT_TIMEOUT )
	{
		if( dwCode == STILL_ACTIVE )
		{
			StringCchPrintf(szMsg, _countof(szMsg), szTimeOut, g_szTeamNames[i]);
			uType |= MB_ICONEXCLAMATION;
		}
		else if( dwCode != 0 )
		{
			StringCchPrintf(szMsg, _countof(szMsg), 
				szThreadDied,  g_szTeamNames[i], g_ThreadIds[i] );
			uType |= MB_ICONSTOP;
		}
	}	/* dwWait == WAIT_TIMEOUT */

	// display error
	if( szMsg[0] != TEXT('\0') )
	{
		MessageBox(GetForegroundWindow(), szMsg, TEXT("Oops!"), uType);
		g_QuitCurrentGame = true;
	}

}

static void __CallAIs(int i)
{
	int iNext = i ? 0 : 1;

	__CallAI(i);
	
	__CallAI(iNext);
}


static void CallAIs()
{
	unsigned int rnd;
	rand_s(&rnd);
	rnd = (unsigned int)((double)rnd / UINT_MAX * 200 + 1);
	
	// select an order to execute AIs
	if( rnd > 100 )
	{
		__CallAIs(1);
	}
	else
	{
		__CallAIs(0);
	}

	//__CallAIs(g_AIndex);
}

void cGame::Gaming()
{
	CallAIs();

	//
	// update heros and balls
	//
	g_HeroTeam1[0]->Update();
	if ( g_GameMode1[0] == GAME_MODE_AI )
		g_HeroTeam1[1]->Update();
	g_HeroTeam2[0]->Update();
	if( g_GameMode2[0] == GAME_MODE_AI )
		g_HeroTeam2[1]->Update();

	g_Balls[0]->Update();
	g_Balls[1]->Update();
	g_Balls[2]->Update();

	//
	// scroll map
	//
	sPosition pos = g_Balls[1]->getPosition();
	pos.x += BALL_WIDTH / 2;
	pos.y += BALL_WIDTH / 2;
	pos.x = (pos.x > 512 * 3) ? (512 * 3) : pos.x;
	pos.x = (pos.x < 512) ? 512 : pos.x;
	s_pMap->SetRoad(pos.x - GAME_WINDOW_WIDTH / 2);

	if( GameOver() )
	{
		//
		// game over
		//
		g_SoundEffectIndex.push_back(SND_GAME_OVER);
		s_ResourceManager.stopBackGroundMusic();
		g_GameStop	= true;
		s_currState = END;
		g_GameEndTime = (float)GetTickCount();
	}
	else
	{
		//
		// see if someone has crossed the gate
		//
		cHero *pHero = ( (cGhostBall*)g_Balls[1] )->m_pOwner;
		if( pHero )
		{
			sPosition pos_tmp = pHero->getPosition();
			int side = FindHeroTeam(pHero);
			
			if( side == 0 && pos_tmp.x > g_Gate[1].x ||
				side == 1 && pos_tmp.x < g_Gate[0].x - HERO_WIDTH )
			{
				s_currState = NEWTURN;
				g_GameTime += GetTickCount() - g_TurnStartTime;
				//g_AIndex = g_AIndex ? 0 : 1;
				g_SoundEffectIndex.push_back(SND_NEW_TURN);
				g_TeamScore[side]++;
				g_ScoredTeam = side;

				g_bSecond = false;
				for(int i = 0; i < _countof(g_FireworksPos); i++)
					g_FireworksPos[i].x = g_FireworksPos[i].y = -1.0f;
			}
		}
	}

	//play all the sound effect
	vector<SoundEffect_Type>::iterator i;
	for( i = g_SoundEffectIndex.begin(); i != g_SoundEffectIndex.end(); ++i )
	{
		g_pSoundEffect->play(*i);		
	}
	g_SoundEffectIndex.clear();
}

void cGame::Pause()
{

}

void __End()
{
	g_QuitCurrentGame = true;

	g_bExitThread[0] = g_bExitThread[1] = TRUE;
	SetEvent(hEventAIs[0]);
	SetEvent(hEventAIs[1]);
	WaitForMultipleObjects(_countof(hWorkerThreads), hWorkerThreads, TRUE, INFINITE);
	for(int i = 0; i < AI_NUM; i++)
	{
		CloseHandle(hWorkerThreads[i]);
		hWorkerThreads[i] = NULL;	
		g_ThreadIds[i] = 0;
	}

	for(int i = 0; i < AI_NUM; i++)
	{
		if( hEventAIs[i] )
		{
			CloseHandle(hEventAIs[i]);
			hEventAIs[i] = NULL;
		}
	}
	CloseHandle(hEventWorkDone);
	hEventWorkDone = NULL;

	UnloadAllClients();
}

void cGame::End()
{
	if( KEYDOWN(VK_RETURN) )
	{
		__End();
	}
}

void cGame::Render()
{
	s_Graphics.clearDisplay(D3DCOLOR_XRGB(0,0,0));

	if( s_Graphics.beginScene() )
	{
		(*s_RSM[s_currState])();
		s_Graphics.endScene();
	}

	s_Graphics.display();
}

void cGame::RenderReady()
{
	static TCHAR szReady[] = TEXT("READY?");

	RenderGaming();

	g_BigFont.print(szReady, 
		(GAME_WINDOW_WIDTH - (_countof(szReady) - 1) * BIG_FONT_WIDTH) / 2, 
		(GAME_WINDOW_HEIGHT - BIG_FONT_HEIGHT) / 2,
		0, 0, D3DCOLOR_XRGB(0, 128, 64));
}

void cGame::RenderGaming()
{
	RenderBackGround();

	g_HeroTeam1[0]->Render();
	g_HeroTeam2[0]->Render();

	if (g_GameMode2[0] == GAME_MODE_AI)
		g_HeroTeam2[1]->Render();

	if (g_GameMode1[0] == GAME_MODE_AI)
 		g_HeroTeam1[1]->Render();


	g_Balls[0]->Render();
	g_Balls[1]->Render();
	g_Balls[2]->Render();

	//draw team name
	RenderTeamName();

	//draw time
	if( s_currState == GAMING )
		RenderGameTime();
}

void cGame::RenderPause()
{

}

void cGame::RenderEnd()
{
	static TCHAR teamWin[TEAM_NAME_LEN * 2];
	static DWORD dwGbFlash;
	static int clrGbIndex = 1;
	static int GoldballIndex = 0;

	static D3DCOLOR clrGbFlash[2] = {
		D3DCOLOR_XRGB(0x23, 0xE4, 0xE4),
		D3DCOLOR_XRGB(0xFF, 0xFF, 0xB3)
	};

	D3DCOLOR back = D3DCOLOR_XRGB(128, 128, 128);
	D3DCOLOR fore = D3DCOLOR_XRGB(0xFF, 242, 0);
	size_t len;
	
	int index = g_TeamScore[0] > g_TeamScore[1] ? 0 : 1;

	// if someone has caught the gold ball,
	// scroll the window
	if( g_pGoldBallOwner != NULL )
	{
		sPosition pos = g_GoldPos;
		pos.x += BALL_WIDTH;
		pos.y += BALL_WIDTH;
		pos.x = (pos.x > 512 * 3) ? (512 * 3) : pos.x;
		pos.x = (pos.x < 512) ? 512 : pos.x;
		s_pMap->SetRoad(pos.x - GAME_WINDOW_WIDTH / 2);
	}

	RenderGaming();

	// draw the goldball
	if( g_pGoldBallOwner != NULL )
	{
		cTexture *pTexGoldball = s_ResourceManager.GetElementTexture(MAGICS);
		int road = s_ResourceManager.getMap()->GetRoad();
		pTexGoldball->draw(g_GoldPos.x - road, g_GoldPos.y,
			GoldballIndex * 128, 5 * 128, 128, 128);

		GoldballIndex++;
		GoldballIndex %= 8;
	}
	
	if( g_GoldBallOwner == 0 || g_GoldBallOwner == 1 )
	{
		StringCchPrintf(teamWin, _countof(teamWin), 
			TEXT("%s caught goldball"), g_szTeamNames[g_GoldBallOwner]);
		StringCchLength(teamWin, _countof(teamWin), &len);

		if( GetTickCount() - dwGbFlash > 300 )
		{
			clrGbIndex = clrGbIndex ? 0 : 1;
			dwGbFlash = GetTickCount();
		}

		g_GoldBallFont.print( teamWin, 
			(GAME_WINDOW_WIDTH - len * GOLDBALL_FONT_WIDTH) / 2, 64,
			0, 0,  clrGbFlash[clrGbIndex] );
	}


	if( g_TeamScore[0] != g_TeamScore[1] )
	{
 		StringCchPrintf( teamWin, _countof(teamWin),TEXT( "%s WINS!"),
 				g_szTeamNames[index]);
	}
	else
	{
		StringCchPrintf( teamWin, _countof(teamWin),TEXT( "TIE GAME!") );
	}

	StringCchLength(teamWin, _countof(teamWin), &len);


#define DELTA 2

	g_BigFont.print( teamWin, 
		(GAME_WINDOW_WIDTH - len * BIG_FONT_WIDTH) / 2 + DELTA, (GAME_WINDOW_HEIGHT - BIG_FONT_HEIGHT) / 2 + DELTA,
		0, 0,  back);
	g_BigFont.print( teamWin, 
		(GAME_WINDOW_WIDTH - len * BIG_FONT_WIDTH) / 2, (GAME_WINDOW_HEIGHT - BIG_FONT_HEIGHT) / 2,
		0, 0,  fore);
}

void cGame::CleanUp()
{
	g_SoundEffectIndex.clear();

	s_ResourceManager.Release();

	g_BigFont.destroy();
	g_NewTurnFont.destroy();
	g_TimeFont.destroy();
	g_TeamNameFont.destroy();
	g_HeroFont.destroy();
	g_GoldBallFont.destroy();

	s_Graphics.destroy();

	for(int i = 0; i < _countof(g_Heros); i++)
	{
		if( g_Heros[i])
			delete g_Heros[i];
		g_Heros[i] = NULL;
	}
	g_HeroTeam1[0] = g_HeroTeam1[1] = NULL;
	g_HeroTeam2[0] = g_HeroTeam2[1] = NULL;

	for(int i = 0; i <_countof(g_AI_HeroType); i++)
	{
		g_AI_HeroType[i] = -1;
	}

	for(int i = 0; i < _countof(g_Balls); i++)
	{
		if(g_Balls[i])
			delete g_Balls[i];
		g_Balls[i] = NULL;
	}

}

void cGame::RenderWaitForStart()
{
	//
	// TO DO
	//
}

void cGame::LoadInformationFromFile()
{
	DWORD dwStatus;
	GameConfig config;
	static PCTSTR aszConifgFile[2] = {
		TEXT("config.cfg"),
		TEXT("overtime.cfg")
	};
	int i = g_bUseOvertimeConfig ? 1 : 0;

	dwStatus = ConfigLoad(aszConifgFile[i], &config, sizeof(GameConfig));
	if( dwStatus != ERROR_SUCCESS )
	{
		HLOCAL pszErrMsg = NULL;
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwStatus, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			(PTSTR)&pszErrMsg,
			0,
			NULL);

		if( pszErrMsg != NULL )
		{
			MessageBox(GetForegroundWindow(),
				(PCTSTR)pszErrMsg, 
				TEXT("Oops!"),
				MB_OK | MB_ICONEXCLAMATION);

			LocalFree(pszErrMsg);
		}

		g_QuitCurrentGame = true;
	}

	FREE_BALL_SHOW_TIME_MIN = config.FreeballShowTime.Min;
	FREE_BALL_SHOW_TIME_MAX = config.FreeballShowTime.Max;
	FREE_BALL_HIDE_TIME_MAX = config.FreeballHideTime.Max;
	FREE_BALL_HIDE_TIME_MIN = config.FreeballHideTime.Min;
	FREE_BALL_SPEED = config.FreeballSpeed;

	GHOST_BALL_SPEED = config.GhostballSpeed;

	GOLD_BALL_SHOW_TIME = config.GoldBallShowTime;
	GOLD_BALL_SLOWDOWN_STEPS = config.GoldBallSlowDownSteps;
	GOLD_BALL_SPEED = config.GoldBallSpeed;

	g_dwGameTime = config.GameTime;

	SPELLED_BY_RON_PERIOD = config.SpelledByRonTime;
	SPELLED_BY_HARRY_PERIOD = config.SpelledByHarryTime;
	SPELLED_BY_GINNY_PERIOD = config.SpelledByGinnyTime;
	SPELLED_BY_MALFOY_PERIOD = config.SpelledByMalfoyTime;
	SPELLED_BY_HERMIONE_PERIOD = config.SpelledByHermioneTime;
	SPELLED_BY_FREEBALL_PERIOD = config.SpelledByFreeBallTime;
	SPELLED_BY_VOLDEMORT_PERIOD = config.SpelledByVoldemortTime;
	SPELLED_BY_VOLDEMORT_DELTA = config.SpelledByVoldemortDelta;

	SPELLED_BY_VOLDEMORT_DELTA = config.SpelledByVoldemortDelta;

	GINNY_MAX_BLUE = config.Ginny.MaxMana;
	GINNY_SPELL_COST = config.Ginny.SpellCost;
	GINNY_SPELL_INTERVAL = config.Ginny.SpellInterval;

	RON_MAX_BLUE = config.Ron.MaxMana;
	RON_SPELL_COST = config.Ron.SpellCost;
	RON_SPELL_INTERVAL = config.Ron.SpellInterval;

	MALFOY_MAX_BLUE = config.Malfoy.MaxMana;
	MALFOY_SPELL_COST = config.Malfoy.SpellCost;
	MALFOY_SPELL_INTERVAL = config.Malfoy.SpellInterval;

	HERMIONE_MAX_BLUE = config.Hermione.MaxMana;
	HERMIONE_SPELL_COST = config.Hermione.SpellCost;
	HERMIONE_SPELL_INTERVAL = config.Hermione.SpellInterval;

	HARRY_MAX_BLUE = config.Harry.MaxMana;
	HARRY_SPELL_INTERVAL = config.Harry.SpellInterval;
	HARRY_SPELL_COST = config.Harry.SpellCost;

	VOLDEMORT_MAX_BLUE = config.Voldemort.MaxMana;
	VOLDEMORT_SPELL_COST = config.Voldemort.SpellCost;
	VOLDEMORT_SPELL_INTERVAL = config.Voldemort.SpellInterval;	
}

void cGame::CreateGameHeros()
{
	int OldMana[4];
	int OldSpellItvl[4];
	bool bRestore = false;

	for (int i = 0; i < _countof(g_AI_HeroType); i++) {
		if (g_GameMode1[0] == GAME_MODE_KEYBOARD && (g_AI_HeroType[0] == HERO_HARRY) ||
			g_GameMode2[0] == GAME_MODE_KEYBOARD && (g_AI_HeroType[2] == HERO_HARRY) ) {
				g_QuitCurrentGame = true;
				MessageBox(GetDesktopWindow(), 
					TEXT("Harry Potter HATES keyboard!"), TEXT("Oops!"), MB_ICONINFORMATION | MB_OK);
				break;
		}
	}

	int road = s_pMap->GetRoad();

	for(int i = 0; i < _countof(g_Heros); i++)
	{
		//
		// delete objects
		//
		if( g_Heros[i] )
		{
			// Save old values
			OldMana[i] = g_Heros[i]->GetCurrBlue();
			OldSpellItvl[i] = g_Heros[i]->GetSpellInterval();
			bRestore = true;

			delete g_Heros[i];
		}

		//
		// create new heros
		//
		sPosition pos(0, 0);
		switch(g_AI_HeroType[i])
		{
		case HERO_GINNY:
			g_Heros[i] = new cHeroGinny(pos);
			break;
			
		case HERO_HERMIONE:
			g_Heros[i] = new cHeroHermione(pos);
			break;

		case HERO_MALFOY:
			g_Heros[i] = new cHeroMalfoy(pos);
			break;

		case HERO_RON:
			g_Heros[i] = new cHeroRon(pos);
			break;

		case HERO_HARRY:
			g_Heros[i] = new cHeroHarry(pos);
			break;

		case HERO_VOLDEMORT:
			g_Heros[i] = new cHeroVoldemort(pos);
			break;
		}
	}

	// Set team name and restore old mana status
	for(int i = 0; i < AI_NUM * 2; i++)
	{
		if( g_Heros[i] )
		{
			g_Heros[i]->SetTeamName(g_szTeamNames[i / 2]);
			g_Heros[i]->SetHeroName(g_szHeroNames[i]);

			if( bRestore )
			{
				g_Heros[i]->SetCurrBlue(OldMana[i]);
				g_Heros[i]->SetSpellInterval(OldSpellItvl[i]);
			}

		}
		else
		{
			MessageBox(NULL, TEXT("Create heros failed!"), TEXT("Oops"), MB_OK | MB_ICONEXCLAMATION);
			return;
		}
	}

	g_HeroTeam1[0] = g_Heros[0];
	if (g_GameMode1[0] == GAME_MODE_AI ) {
		g_HeroTeam1[0]->SetPos(g_HeroInitPos[0] + road, g_HeroInitPos[1]);
	}
	else {
		g_HeroTeam1[0]->SetPos( g_HeroInitPos[0] + road, (GAME_WINDOW_HEIGHT - HERO_WIDTH) / 2 );
	}
	g_HeroTeam1[0]->setAction(ACTION_HERO_RIGHT);
	g_HeroTeam1[0]->setAction(ACTION_HERO_STOP);


	g_HeroTeam1[1] = g_Heros[1];
	g_HeroTeam1[1]->SetPos(g_HeroInitPos[2] + road, g_HeroInitPos[3]);
	g_HeroTeam1[1]->setAction(ACTION_HERO_RIGHT);
	g_HeroTeam1[1]->setAction(ACTION_HERO_STOP);


	g_HeroTeam2[0] = g_Heros[2];
	if (g_GameMode2[0] == GAME_MODE_AI)
		g_HeroTeam2[0]->SetPos(road + g_HeroInitPos[4], g_HeroInitPos[5]);
	else
		g_HeroTeam2[0]->SetPos(road + g_HeroInitPos[4], (GAME_WINDOW_HEIGHT - HERO_WIDTH) / 2);
	g_HeroTeam2[0]->setAction(ACTION_HERO_LEFT);
	g_HeroTeam2[0]->setAction(ACTION_HERO_STOP);


	g_HeroTeam2[1] = g_Heros[3];
	g_HeroTeam2[1]->SetPos(road + g_HeroInitPos[6], g_HeroInitPos[7]);
	g_HeroTeam2[1]->setAction(ACTION_HERO_LEFT);
	g_HeroTeam2[1]->setAction(ACTION_HERO_STOP);
}

void cGame::RenderGameTime()
{
	static D3DCOLOR clrTime[2] = {
		D3DCOLOR_XRGB(0xE8, 0xE8, 0x53),
		D3DCOLOR_XRGB(0xFF, 0x00, 0x00)
	};


	if( !g_GameStop )
	{
		TCHAR szTime[32];
		if( g_AfterReady )
		{
			StringCchPrintf( szTime, _countof(szTime), TEXT("Elapsed Time: %.1fs[%.1fs]"),
				(GetTickCount() - g_TurnStartTime + g_GameTime) / 1000.0f, g_dwGameTime / 1000.0f );	
		}
		else
		{
			StringCchPrintf( szTime, _countof(szTime),_T("Elapsed Time: %.1fs[%.1fs]"),
				(float)0.0, g_dwGameTime / 1000.0f);
		}

		// flash time
		if( GetTickCount() - g_TurnStartTime + g_GameTime > 5 * g_dwGameTime / 6)
		{
			if( GetTickCount() -  g_dwFlashColorTime > 300 )
			{
				g_clrIndexTime = g_clrIndexTime ? 0 : 1;
				g_dwFlashColorTime = GetTickCount();
			}
		}

		size_t len = 0;
		StringCchLength(szTime, _countof(szTime), &len);
		g_TimeFont.print(szTime, 
			(GAME_WINDOW_WIDTH - len * TIME_FONT_WIDTH) / 2, 0,
			0, 0, clrTime[g_clrIndexTime] );
	}	
}

int GetBannerIndex(cHero *pHero)
{
	switch(pHero->m_objectType)
	{
	case HERO_RON:
		return 1;
		
	case HERO_GINNY:
		return 3;

	case HERO_HARRY:
		return 0;

	case HERO_HERMIONE:
		return 2;

	case HERO_MALFOY:
		return 4;

	case HERO_VOLDEMORT:
		return 5;

	default:
		return -1;
	}
}

void cGame::RenderTeamName()
{
	TCHAR szMsg[TEAM_NAME_LEN * 2];
	size_t len;
	D3DCOLOR colors[AI_NUM] = { D3DCOLOR_XRGB(0xFF, 142, 30),
		D3DCOLOR_XRGB(0, 0xFF, 33) };
	int ax[AI_NUM*2] = {0, 0, GAME_WINDOW_WIDTH - 128, GAME_WINDOW_WIDTH - 128};
	int ay[AI_NUM*2] = {64, 160, 64, 160};

	// render current score
	StringCchPrintf(szMsg, _countof(szMsg), TEXT("%s score: %u"),
		g_szTeamNames[0], g_TeamScore[0]);
	g_TeamNameFont.print(szMsg, 20, 30, 0, 0, colors[0]);

	StringCchPrintf(szMsg, _countof(szMsg), TEXT("%s score: %u"),
		g_szTeamNames[1], g_TeamScore[1]);
	StringCchLength(szMsg, _countof(szMsg), &len);
	g_TeamNameFont.print(szMsg, 
		GAME_WINDOW_WIDTH - 20 - len * TEAM_NAME_FONT_WIDTH, 30, 0, 0, colors[1]);

	// render mana banner
	cTexture *pTexBanner = s_ResourceManager.GetElementTexture(BANNER);
	for(int i = 0; i <AI_NUM * 2; i++)
	{
		if ( (i == 1 && g_GameMode1[0] == GAME_MODE_KEYBOARD) ||
			(i == 3 && g_GameMode2[0] == GAME_MODE_KEYBOARD)) {
				continue;
		}

		if( pTexBanner != NULL )
		{
			pTexBanner->draw(ax[i], ay[i],
				GetBannerIndex(g_Heros[i]) * 128, 128, 128, 128,
				0xFFFFFFFF, NULL, 0.0f, 0.5f, 0.5f);

			double r = (double)g_Heros[i]->GetCurrBlue() / g_Heros[i]->GetMaxBlue();
			int manalen = (int)(128 * r);
			manalen = manalen < 1 ? 1 : manalen;
			pTexBanner->draw(ax[i], ay[i] + 42,
				0, 32, manalen, 32);
		}
	}

	sPosition pos;
	int road = s_pMap->GetRoad();

	for(int i = 0; i < AI_NUM * 2; i++)
	{
		if ( (i == 1 && g_GameMode1[0] == GAME_MODE_KEYBOARD) ||
			(i == 3 && g_GameMode2[0] == GAME_MODE_KEYBOARD)) {
				continue;
		}

		g_Heros[i]->GetPos(pos);
		StringCchLength(g_Heros[i]->GetHeroName(), HERO_NAME_LEN, &len);
		pos.x = pos.x + (HERO_WIDTH / HERO_FONT_WIDTH - len) * HERO_FONT_WIDTH / 2;
		g_HeroFont.print(g_Heros[i]->GetHeroName(), pos.x - road , pos.y - 8, 0, 0, colors[i / 2]);
	}

}

void cGame::RenderBackGround()
{
	s_pMap->display();
}

void cGame::RenderNewTurn()
{
	cTexture* pFireworksTex[4];

	pFireworksTex[0] = s_ResourceManager.GetElementTexture(FIREWORK1);
	pFireworksTex[1] = s_ResourceManager.GetElementTexture(FIREWORK2);
	pFireworksTex[2] = s_ResourceManager.GetElementTexture(FIREWORK3);
	pFireworksTex[3] = s_ResourceManager.GetElementTexture(FIREWORK4);

	RenderGaming();

	TCHAR szMsg[TEAM_NAME_LEN * 2];
	StringCchPrintf(szMsg, _countof(szMsg), TEXT("%s scored!"), g_szTeamNames[g_ScoredTeam]);
	g_NewTurnFont.print( szMsg, 350, 50, 0, 0, D3DCOLOR_XRGB(0x77, 0xFF, 0) );

	for(int i = 0; i < 4; i++)
	{
		int TexX = 0, SecondTexX = 0;

		if( vy[i] < -7 )
			TexX = 0;
		else if( vy[i] < -6.6 )
			TexX = 1;
		else if( vy[i] < -6.2 )
			TexX = 2;
		else if( vy[i] < -4 )
		{
			ay = 0.2f;
			TexX = 3;

			if( !g_bSecond )
			{
				//
				// Replay
				//
				g_bSecond = true;
				for(int i = 0; i < _countof(g_SecondFireworksPos); i++)
					g_SecondFireworksPos[i].x = g_SecondFireworksPos[i].y = -1.0f;
			}
		}
		else if( vy[i] < -2 )
			TexX = 4;
		else if ( vy[i] < -1 )
			TexX = 5;
		else if ( vy[i] < 0 )
			TexX = 6;
		else
			vy[i] = 1;

		if( vy[i] <= 0 )
		{
			if( TexX != 6 )
				pFireworksTex[i]->draw((long)g_FireworksPos[i].x, (long)g_FireworksPos[i].y,
					TexX * FIREWORK_SIZE, 0, FIREWORK_SIZE, FIREWORK_SIZE);
			else
				pFireworksTex[i]->draw((long)g_FireworksPos[i].x, (long)g_FireworksPos[i].y,
					5 * FIREWORK_SIZE, 0, FIREWORK_SIZE, FIREWORK_SIZE, D3DCOLOR_XRGB(0xEE, 0xEE, 0xEE));
		}

		// Second fire
		if( g_bSecond )
		{
			if( Secondvy[i] < -7 )
				SecondTexX = 0;
			else if( Secondvy[i] < -6.6 )
				SecondTexX = 1;
			else if( Secondvy[i] < -6.2 )
				SecondTexX = 2;
			else if( Secondvy[i] < -4 )
			{
				Seconday = 0.2f;
				SecondTexX = 3;
			}
			else if( Secondvy[i] < -2 )
				SecondTexX = 4;
			else if ( Secondvy[i] < -1 )
				SecondTexX = 5;
			else if ( Secondvy[i] < 0 )
				SecondTexX = 6;
			else
				Secondvy[i] = 1;

			if( Secondvy[i] <= 0 )
			{
				if( SecondTexX != 6 )
					pFireworksTex[i]->draw((long)g_SecondFireworksPos[i].x, (long)g_SecondFireworksPos[i].y,
						SecondTexX * FIREWORK_SIZE, 0, FIREWORK_SIZE, FIREWORK_SIZE);
				else
					pFireworksTex[i]->draw((long)g_SecondFireworksPos[i].x, (long)g_SecondFireworksPos[i].y,
						5 * FIREWORK_SIZE, 0, FIREWORK_SIZE, FIREWORK_SIZE, D3DCOLOR_XRGB(0xEE, 0xEE, 0xEE));
			}
		}
	}
}

void cGame::NewTurn()
{
	float FireworksXPos[4] = {
		0, 256, 512, 768
	};

	float v[4] = {
		-12.0f,
		-11.2f,
		-12.7f,
		-13.4f
	};

	for(int i = 0; i < 4; i++)
	{
		if( g_FireworksPos[i].x == -1.0f && g_FireworksPos[i].y == -1.0f )
		{
			// Fire now!
			g_FireworksPos[i].x = FireworksXPos[i];
			g_FireworksPos[i].y = 700;

			vy[i] = v[i];			
			ay = 0.1f;

			continue;
		}
		
		// Update firework position
		if( vy[i] < 0 )
		{
			g_FireworksPos[i].y += vy[i];
			vy[i] += ay;
		}

		// Second fire
		if( g_bSecond )
		{
			if( g_SecondFireworksPos[i].x == -1.0f && g_SecondFireworksPos[i].y == -1.0f )
			{
				// Fire now!
				g_SecondFireworksPos[i].x = FireworksXPos[3-i];
				g_SecondFireworksPos[i].y = 700;

				Secondvy[i] = v[i];			
				Seconday = 0.1f;

				continue;
			}

			// Update firework position
			if( Secondvy[i] < 0 )
			{
				g_SecondFireworksPos[i].y += Secondvy[i];
				Secondvy[i] += Seconday;
			}
		}
	}

	for(int i = 0; i < 4; i++)
	{
		if( vy[i] < 0 ) return;
	}

	for(int i = 0; i < 4; i++)
	{
		if( Secondvy[i] < 0 ) return;
	}

	//
	// reset heros and balls
	//
	s_pMap->SetRoad(512);
	CreateGameHeros();
	CreateBalls();

	//
	// return to gaming state
	//
	s_currState = GAMING;
	g_TurnStartTime = GetTickCount();
}

void cGame::CreateBalls()
{
	for(int i = 0; i < BALL_NUM; i++)
		if(g_Balls[i])
			delete g_Balls[i];

	g_Balls[0] = new cFreeBall;
	g_Balls[1] = new cGhostBall;
	g_Balls[2] = new cGoldBall;
}

int cGame::FindHeroTeam(cHero *p_pHero)
{
	int iRet = -1;

	for(int i = 0; i < _countof(g_HeroTeam1); i++)
	{
		if( g_HeroTeam1[i] == p_pHero )
		{
			iRet = 0;
			goto EXIT;
		}
	}
	for(int i = 0; i < _countof(g_HeroTeam2); i++)
	{
		if(g_HeroTeam2[i] == p_pHero)
		{
			iRet =  1;
			goto EXIT;
		}
	}

EXIT:
	return iRet;
	
}

int cGame::FindBall( cBall *p_pBall )
{
	int iRet = -1;

	for(int i = 0; i < _countof(g_Balls); i++)
	{
		if( p_pBall == g_Balls[i] )
		{
			iRet = i;
			break;
		}
	}

	return iRet;
}