#ifndef		__GLOBALDEF_H__
#define		__GLOBALDEF_H__

#define _CRT_RAND_S		// for rand_s

#include <windows.h>
#include <cassert>
#include <vector>

using namespace std;

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)


class cHero;
class cBall;


const int HERO_WIDTH = 128;

enum SoundEffect_Type
{
	SND_SPELLED_BY_HERMIONE,
	SND_SPELLED_BY_MALFOY,
	SND_RON_GINNY_SPELLING,
	SND_GAME_OVER,
	SND_NEW_TURN,
	SND_SPELLED_BY_HARRY,
	SND_SPELLED_BY_VOLDEMORT,
};

extern vector<SoundEffect_Type>	g_SoundEffectIndex;


enum Hero_Type
{
	HERO_RON,
	HERO_HERMIONE,
	HERO_MALFOY,
	HERO_HARRY,
	HERO_VOLDEMORT,
	HERO_GINNY,
};

enum Ball_Type
{
	BALL_FREE,
	BALL_GHOST,
	BALL_GOLD,
};

enum Element_Type
{
	MAGICS,
	BANNER,
	FIREWORK1,
	FIREWORK2,
	FIREWORK3,
	FIREWORK4,
};


enum Hero_Action
{
	ACTION_HERO_BOTTOM,
	ACTION_HERO_LEFTBOTTOM,
	ACTION_HERO_LEFT,
	ACTION_HERO_LEFTTOP,
	ACTION_HERO_TOP,
	ACTION_HERO_RIGHTTOP,
	ACTION_HERO_RIGHT,
	ACTION_HERO_RIGHTBOTTOM,
	ACTION_HERO_SPELLING,
	ACTION_HERO_STOP,
};

enum Abnormal_Type
{
	/* normal state */
	SPELLED_BY_NONE,
	SPELLING,

	/* abmormal states */
	SPELLED_BY_RON,
	SPELLED_BY_HERMIONE,
	SPELLED_BY_MALFOY,
	SPELLED_BY_HARRY,
	SPELLED_BY_VOLDEMORT,
	SPELLED_BY_GINNY,
	SPELLED_BY_FREEBALL,
};


enum Game_Mode
{
	GAME_MODE_AI,
	GAME_MODE_KEYBOARD,
};

//Speed structure
struct sSpeed
{
	int vx;
	int	vy;
	sSpeed(){}
	sSpeed(int p_vx, int p_vy)
		:vx(p_vx),vy(p_vy){}
};

//Position sturcture
struct sPosition
{
	int x;
	int y;
	sPosition(){}
	sPosition(int p_x, int p_y)
		:x(p_x),y(p_y){}
};



/////////////////////////////////////////////////////////////////////////////

/*----------------------------Global Declation----------------------------------*/

#define AI_NUM		2
#define BALL_NUM	3
#define TEAM_NAME_LEN	16
#define HERO_NAME_LEN	16

#define HERO_SPEED 7
#define HERO_SPEED_SLOW 5

#define GAME_MAP_WIDTH	2048
#define GAME_MAP_HEIGHT	768


typedef void (__stdcall *PFN_AI)();
typedef void (__stdcall *PFN_INIT)();

extern	cHero*			g_HeroTeam1[AI_NUM];
extern	cHero*			g_HeroTeam2[AI_NUM];
extern	cHero*			g_Heros[AI_NUM * 2];
extern  cBall*			g_Balls[BALL_NUM];
extern  TCHAR g_szTeamNames[AI_NUM][TEAM_NAME_LEN];
extern  TCHAR g_szHeroNames[AI_NUM*2][HERO_NAME_LEN];



extern PFN_AI g_pfnAI[AI_NUM];
extern PFN_INIT g_pfnInit[AI_NUM];


extern	int g_AI_HeroType[AI_NUM * 2];

extern	const int GAME_SCREEN_WIDTH;
extern	const int GAME_SCREEN_HEIGHT;
extern	const int GAME_WINDOW_WIDTH;
extern	const int GAME_WINDOW_HEIGHT;

typedef unsigned (__stdcall *PTHREAD_START) (void *);

#define BEGINTHREADEX(psa, cbStack,pfnStartAddr, \
	pvParam, fdwCreate, pdwThreadId)			 \
	( (HANDLE) _beginthreadex(					 \
	(void *)(psa),							 \
	(unsigned) (cbStack),					 \
	(PTHREAD_START) (pfnStartAddr),			 \
	(void *) (pvParam),						 \
	(unsigned) (fdwCreate),					 \
	(unsigned *) (pdwThreadId)) )


#define WM_LOAD_IN_PROGRESS	WM_USER + 2009
#define WM_LOAD_DONE		WM_USER + 2012
/*---------------------------------------------------------------------------------*/

#endif