#define TC_CLIENT_EXPORTS
#include "TuringCup9ClientAPI.h"

#include "HeroRon.h"
#include "HeroGinny.h"
#include "HeroHermione.h"
#include "HeroMalfoy.h"
#include "HeroState.h"
#include "cBall.h"
#include "Game.h"

#include <fstream>
#include <StrSafe.h>

using namespace std;

TCHAR g_TeamNameTmp[TEAM_NAME_LEN];
TCHAR g_szTeamNames[AI_NUM][TEAM_NAME_LEN];

TCHAR g_HeroNameTmp[2][HERO_NAME_LEN];
TCHAR g_szHeroNames[AI_NUM*2][HERO_NAME_LEN];

cHero*	g_HeroTeam1[AI_NUM];
cHero*	g_HeroTeam2[AI_NUM];
cHero*  g_Heros[AI_NUM * 2];
cBall*	g_Balls[BALL_NUM];

int	g_ActionTeam[2] = { -1, -1 };

// half by half
int g_HeroTypeTmp[AI_NUM];
int g_AI_HeroType[AI_NUM * 2];	


int	g_GameMode1[2] = { GAME_MODE_KEYBOARD, GAME_MODE_KEYBOARD };//GAME_MODE_KEYBOARD;
int	g_GameMode2[2] = { GAME_MODE_KEYBOARD, GAME_MODE_KEYBOARD };//GAME_MODE_KEYBOARD;

/* masks */
unsigned g_CurrentMask = 0;
unsigned g_CurrentEnemyMask = 0;
#define TC_MASK (0x81367040)


int g_IndexOffset = 0;
#define TC_HANDLE_TO_PHERO(h) ((cHero*)((unsigned)(h) ^ (g_CurrentMask)))
#define TC_PHERO_TO_HANDLE(h) ((TC_Handle)((unsigned)(h) ^ (g_CurrentMask)))

#define TC_HANDLE_TO_ENEMY_PHERO(h) ((cHero*)((unsigned)(h) ^ (g_CurrentEnemyMask)))
#define TC_ENEMY_PHERO_TO_HANDLE(h) ((TC_Handle)((unsigned)(h) ^ (g_CurrentEnemyMask)))

#define TC_HANDLE_TO_PBALL(h) ((cBall*)((unsigned)(h) ^ TC_MASK))
#define TC_PBALL_TO_HANDLE(h) ((TC_Handle)((unsigned)(h) ^ TC_MASK))

TC_Hero g_ClientHeros[AI_NUM * 2];
TC_Ball g_ClientBalls[BALL_NUM];

TC_Rect g_ForbiddenAreas[AI_NUM] = {
	{ 128, 200, 640, 456 },
	{ 128, 1592, 640, 1848 }
};		// forbidden areas

extern Gate g_Gate[AI_NUM];
extern DWORD g_GameTime;
extern DWORD g_dwGameTime;
extern DWORD g_TurnStartTime;
extern unsigned g_TeamScore[AI_NUM];		// store team scores


#define MAP_WIDTH	2048
#define MAP_HEIGHT	768

unsigned SNATCH_INTERVAL = 25;

bool SpellInRange(cHero *pHero1, cHero *pHero2);


void ClientInit1()
{
	(*g_pfnInit[0])();

	StringCchCopy(g_szTeamNames[0], _countof(g_szTeamNames[0]), g_TeamNameTmp);
	StringCchCopy(g_szHeroNames[0], HERO_NAME_LEN, g_HeroNameTmp[0]);
	StringCchCopy(g_szHeroNames[1], HERO_NAME_LEN, g_HeroNameTmp[1]);
	g_AI_HeroType[0] = g_HeroTypeTmp[0];
	g_AI_HeroType[1] = g_HeroTypeTmp[1];
}

void ClientInit2()
{
	(*g_pfnInit[1])();

	StringCchCopy(g_szTeamNames[1], _countof(g_szTeamNames[1]), g_TeamNameTmp);
	StringCchCopy(g_szHeroNames[2], HERO_NAME_LEN, g_HeroNameTmp[0]);
	StringCchCopy(g_szHeroNames[3], HERO_NAME_LEN, g_HeroNameTmp[1]);
	g_AI_HeroType[2] = g_HeroTypeTmp[0];
	g_AI_HeroType[3] = g_HeroTypeTmp[1];
}

static void SetClientHeroInfo()
{
	/* copy hero type */
	for(int i = 0; i < AI_NUM * 2; i++)
	{
		g_ClientHeros[i].type = (TC_Hero_Type)g_AI_HeroType[i];

		/* copy hero speed */
		sSpeed speed = g_Heros[i]->GetSpeed();
		g_ClientHeros[i].speed.vx = speed.vx;
		g_ClientHeros[i].speed.vy = speed.vy;

		/* copy hero position */
		sPosition pos = g_Heros[i]->getPosition();
		g_ClientHeros[i].pos.x = pos.x;
		g_ClientHeros[i].pos.y = pos.y;

		/* copy blue info */
		g_ClientHeros[i].max_blue = g_Heros[i]->GetMaxBlue();
		g_ClientHeros[i].curr_blue = g_Heros[i]->GetCurrBlue();
		g_ClientHeros[i].spell_cost = g_Heros[i]->GetSpellCost();
		g_ClientHeros[i].spell_interval = g_Heros[i]->GetStaticSpellInterval();

		/* spell state */
		g_ClientHeros[i].b_is_spelling = g_Heros[i]->IsSpelling();
		g_ClientHeros[i].b_can_spell = g_Heros[i]->CanSpell();

		/* copy ball info */
		g_ClientHeros[i].b_ghostball = g_Heros[i]->HaveGhostBall();
		g_ClientHeros[i].b_goldball = g_Heros[i]->HaveGoldBall();
		g_ClientHeros[i].b_snatch_ghostball = g_Heros[i]->CanSnatchGhostBall();
		g_ClientHeros[i].b_snatch_goldball = g_Heros[i]->CanSnatchGoldBall();

		/* copy abnormal type */
		g_ClientHeros[i].abnormal_type = (TC_Abnormal_Type)g_Heros[i]->GetAbnormalType();

		/* copy snatch steps */
		g_ClientHeros[i].steps_before_next_snatch = g_Heros[i]->GetSnatchSteps();
	}
}

static void SetClientBallInfo()
{
	for(int i = 0; i < BALL_NUM; i++)
	{
		g_ClientBalls[i].type = (TC_Ball_Type)g_Balls[i]->GetBallType();
		
		g_ClientBalls[i].b_visible = g_Balls[i]->IsVisible();

		if( g_Balls[i]->IsVisible() )
		{
			sSpeed speed = g_Balls[i]->GetSpeed();
			g_ClientBalls[i].speed.vx = speed.vx;
			g_ClientBalls[i].speed.vy = speed.vy;

			sPosition pos = g_Balls[i]->getPosition();
			g_ClientBalls[i].pos.x = pos.x;
			g_ClientBalls[i].pos.y = pos.y;
		}
		else
		{
			g_ClientBalls[i].speed.vx = 0;
			g_ClientBalls[i].speed.vy = 0;

			g_ClientBalls[i].pos.x = -1;
			g_ClientBalls[i].pos.y = -1;
		}
	}

	/* free ball only */
	cFreeBall *pFreeBall = (cFreeBall*)g_Balls[0];
	if( pFreeBall->IsVisible() )
	{
		sPosition pos1 = pFreeBall->GetStartPos();
		sPosition pos2 = pFreeBall->GetEndPos();
		g_ClientBalls[0].u.path.pos_start.x = pos1.x;
		g_ClientBalls[0].u.path.pos_start.y = pos1.y;
		g_ClientBalls[0].u.path.pos_end.x = pos2.x;
		g_ClientBalls[0].u.path.pos_end.y = pos2.y;
	}
	else
	{
		g_ClientBalls[0].u.path.pos_start.x = -1;
		g_ClientBalls[0].u.path.pos_start.y = -1;
		g_ClientBalls[0].u.path.pos_end.x = -1;
		g_ClientBalls[0].u.path.pos_end.y = -1;
	}
	g_ClientBalls[0].u.moving_time_left = pFreeBall->GetShowTime();

	/* ghost ball only */
	cGhostBall *pGhostBall = (cGhostBall*)g_Balls[1];
	if( pGhostBall->IsMoving() )
	{
		g_ClientBalls[1].u.target.x = pGhostBall->m_posTarget.x;
		g_ClientBalls[1].u.target.y = pGhostBall->m_posTarget.y;
	}
	else
	{
		g_ClientBalls[1].u.target.x = -1;
		g_ClientBalls[1].u.target.y = -1;
	}
	
	/* gold ball only */
	cGoldBall *pGoldBall = (cGoldBall*)g_Balls[2];

	g_ClientBalls[2].u.slow_down_steps_left = pGoldBall->GetSlowDownSteps();
}

void ClientAI1()
{
	//
	// setup masks
	//
	g_CurrentMask = (unsigned)g_pfnAI[0];
	g_CurrentEnemyMask = ((unsigned)g_pfnAI[1] == 0) ? TC_MASK : (unsigned)g_pfnAI[1];
	g_IndexOffset = 0;

	//
	// Set ball and hero info
	//
	SetClientBallInfo();
	SetClientHeroInfo();

	(*g_pfnAI[0])();
}

void ClientAI2()
{
	//
	// setup masks
	//
	g_CurrentMask = (unsigned)g_pfnAI[1];
	g_CurrentEnemyMask = ((unsigned)g_pfnAI[0] == 0) ? TC_MASK : (unsigned)g_pfnAI[0];

	g_IndexOffset = 2;

	//
	// Set ball and hero info
	//
	SetClientBallInfo();
	SetClientHeroInfo();

	(*g_pfnAI[1])();
}

void  KeyBoard1()
{
	int action = -1;

	bool bW, bA, bS, bD;
	bool bC, bV, bB;

	bW = KEYDOWN('W');
	bA = KEYDOWN('A');
	bS = KEYDOWN('S');
	bD = KEYDOWN('D');

	bC = KEYDOWN('C');
	bV = KEYDOWN('V');
	bB = KEYDOWN('B');


	if( g_HeroTeam1[0]->CanMove() )
	{
		if( bW && bA && !bS && !bD )
		{
			action = ACTION_HERO_LEFTTOP;
		}
		else if( bW && !bA && !bS && !bD )
		{
			action = ACTION_HERO_TOP;
		}
		else if( bW && bD && !bA && !bS )
		{
			action = ACTION_HERO_RIGHTTOP;
		}
		else if( bA && !bW && !bS && !bD )
		{
			action = ACTION_HERO_LEFT;
		}
		else if( bD && !bW && !bA && !bS )
		{
			action = ACTION_HERO_RIGHT;
		}
		else if( bA && bS && !bW && !bD )
		{
			action = ACTION_HERO_LEFTBOTTOM;
		}
		else if( bS && !bW && !bA && !bD )
		{
			action = ACTION_HERO_BOTTOM;
		}
		else if( bS && bD && !bW && !bA )
		{
			action = ACTION_HERO_RIGHTBOTTOM;
		}
	}

	if( bC && !bV && !bB ) {
		Tc_Spell(TC_PHERO_TO_HANDLE(g_HeroTeam1[0]), TC_ENEMY_PHERO_TO_HANDLE(g_HeroTeam2[0]) );
	}
	
	if( g_HeroTeam1[0]->CanSnatchGhostBall() && bV && !bB ) {
		Tc_SnatchBall(TC_PHERO_TO_HANDLE(g_HeroTeam1[0]), TC_GHOST_BALL);
	}

	if( g_HeroTeam1[0]->CanSnatchGoldBall() && !bV && bB ) {
		Tc_SnatchBall(TC_PHERO_TO_HANDLE(g_HeroTeam1[0]), TC_GOLD_BALL);
	}

	g_HeroTeam1[0]->setAction(action);
}

void  KeyBoard2()
{
	int action = -1;

	bool bI, bJ, bK, bL;
	bool bP, bLBra, bRBra;

	bI = KEYDOWN('I');
	bJ = KEYDOWN('J');
	bK = KEYDOWN('K');
	bL = KEYDOWN('L');

	bP = KEYDOWN('P');
	bLBra = KEYDOWN(VK_OEM_4);
	bRBra = KEYDOWN(VK_OEM_6);


	if( g_HeroTeam2[0]->CanMove() )
	{
		if( bI && bJ && !bK && !bL )
		{
			action = ACTION_HERO_LEFTTOP;
		}
		else if( bI && !bJ && !bK && !bL )
		{
			action = ACTION_HERO_TOP;
		}
		else if( bI && bL && !bJ && !bK )
		{
			action = ACTION_HERO_RIGHTTOP;
		}
		else if( bJ && !bI && !bK && !bL )
		{
			action = ACTION_HERO_LEFT;
		}
		else if( bL && !bI && !bJ && !bK )
		{
			action = ACTION_HERO_RIGHT;
		}
		else if( bJ && bK && !bI && !bL )
		{
			action = ACTION_HERO_LEFTBOTTOM;
		}
		else if( bK && !bI && !bJ && !bL )
		{
			action = ACTION_HERO_BOTTOM;
		}
		else if( bK && bL && !bI && !bJ )
		{
			action = ACTION_HERO_RIGHTBOTTOM;
		}
	}

	if( bP && !bLBra && !bRBra ) {
		Tc_Spell(TC_PHERO_TO_HANDLE(g_HeroTeam2[0]), TC_ENEMY_PHERO_TO_HANDLE(g_HeroTeam1[0]) );
	}

	if( g_HeroTeam2[0]->CanSnatchGhostBall() && bLBra && !bRBra ) {
		Tc_SnatchBall(TC_PHERO_TO_HANDLE(g_HeroTeam2[0]), TC_GHOST_BALL);
	}

	if( g_HeroTeam2[0]->CanSnatchGoldBall() && !bLBra && bRBra ) {
		Tc_SnatchBall(TC_PHERO_TO_HANDLE(g_HeroTeam2[0]), TC_GOLD_BALL);
	}

	g_HeroTeam2[0]->setAction(action);
}


void (*g_Pattern1[2])() = { ClientAI1, KeyBoard1 };
void (*g_Pattern2[2])() = { ClientAI2, KeyBoard2 };


/*TC_CLIENT_API*/ void __stdcall Tc_SetTeamName( IN PCWSTR pszTeamName )
{
	StringCchCopy( g_TeamNameTmp, _countof(g_TeamNameTmp), pszTeamName );
}

/*TC_CLIENT_API*/ void __stdcall Tc_ChooseHero( IN TC_Team_Member index, IN TC_Hero_Type Type, IN PCWSTR pszHeroName )
{
	if( index < TC_MEMBER1 || index > TC_MEMBER2 )
		return;
	if( Type < TC_HERO_RON || Type > TC_HERO_GINNY )
		return;
	if( pszHeroName == NULL )
		return;

	g_HeroTypeTmp[index] = Type;
	StringCchCopy(g_HeroNameTmp[index], HERO_NAME_LEN, pszHeroName);	
}


/*TC_CLIENT_API*/ bool __stdcall Tc_Move( IN TC_Handle handle_self, IN TC_Direction direction )
{
	cHero *pHero = TC_HANDLE_TO_PHERO(handle_self);
	if( cGame::FindHeroTeam(pHero) == -1 )
		return false;

	if( direction < TC_DIRECTION_BOTTOM || 
		direction > TC_DIRECTION_RIGHTBOTTOM )
	{
		return false;
	}

	if( pHero->CanMove() )
	{
		pHero->setAction(direction);
		return true;
	}

	return false;	
}

bool SpellInRange(cHero *pHero1, cHero *pHero2)
{
	sPosition pos1, pos2;
	pHero1->GetPos(pos1);
	pHero2->GetPos(pos2);

	pos1.x += HERO_WIDTH / 2;
	pos1.y += HERO_WIDTH / 2;
	pos2.x += HERO_WIDTH / 2;
	pos2.y += HERO_WIDTH / 2;

	double len = pow((double)pos1.x - pos2.x, 2) + pow((double)pos1.y - pos2.y, 2);

	return len <= pow((double)TC_HERO_SPELL_DISTANCE, 2.0);
}

/*TC_CLIENT_API*/ bool __stdcall Tc_Spell( IN TC_Handle handle_self, IN TC_Handle handle_target )
{
	cHero *pHeroTarget = NULL;
	cHero *pHeroSelf = TC_HANDLE_TO_PHERO(handle_self);
	
	// Harry Potter is special
	if( pHeroSelf->m_objectType == HERO_HARRY )
	{
		pHeroTarget = TC_HANDLE_TO_PHERO(handle_target);
		if( pHeroTarget == pHeroSelf )
			return false;

		if( cGame::FindHeroTeam(pHeroTarget) == -1 ||
			pHeroTarget == pHeroSelf )
		{
			return false;
		}

		if( pHeroTarget->GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) ||
			pHeroTarget->GetStateMachine()->IsInState(cSpelledByVoldemortState::Instance()) ||
			pHeroTarget->GetStateMachine()->IsInState(cSpelledByMalfoyState::Instance()) )
		{
			if( !SpellInRange(pHeroSelf, pHeroTarget) )
				return false;

			if( pHeroSelf->CanSpell() )
			{
				pHeroSelf->SetSpellObject(pHeroTarget);
				pHeroSelf->setAction(ACTION_HERO_SPELLING);
				pHeroSelf->Spell();

				return true;
			}
		}
	}
	else
	{
		pHeroTarget = TC_HANDLE_TO_ENEMY_PHERO(handle_target);
		if( (cGame::FindHeroTeam(pHeroSelf) == -1) ||
			(cGame::FindHeroTeam(pHeroTarget) == -1) )
		{
			return false;
		}

		// in range??
		if( !SpellInRange(pHeroTarget, pHeroSelf) )
			return false;

		if( pHeroSelf->CanSpell() &&
			pHeroTarget->CanBeSpelled() )
		{
			pHeroSelf->SetSpellObject(pHeroTarget);
			pHeroSelf->setAction(ACTION_HERO_SPELLING);
			pHeroSelf->Spell();

			return true;
		}
	}

	return true;
}

static bool PassBallOutofRange(TC_Position &pos)
{
	sPosition hp;
	g_Balls[1]->GetPos(hp);
	hp.x += BALL_WIDTH / 2;
	hp.y += BALL_WIDTH / 2;
	TC_Position bp;
	bp.x = pos.x + BALL_WIDTH / 2;
	bp.y = pos.y + BALL_WIDTH / 2;
	double len = pow(hp.x - bp.x, 2.0) + pow(hp.y - bp.y, 2.0);
	if( len > pow(TC_PASSBALL_DISTANCE, 2.0) )
		return true;

	if( pos.y > MAP_HEIGHT - BALL_WIDTH || pos.y < BALL_WIDTH )
		return true;

	if( pos.x < g_ForbiddenAreas[0].left || pos.x > g_ForbiddenAreas[1].right - BALL_WIDTH )
		return true;

	for(int i = 0; i < AI_NUM; i++)
	{
		if( pos.x > g_ForbiddenAreas[i].left - BALL_WIDTH && pos.x < g_ForbiddenAreas[i].right &&
			pos.y > g_ForbiddenAreas[i].top - BALL_WIDTH && pos.y < g_ForbiddenAreas[i].bottom )
		{
			return true;
		}
	}	

	return false;
}

/*TC_CLIENT_API*/ bool __stdcall Tc_PassBall( IN TC_Handle handle_self, IN TC_Position &pos_target )
{
	cHero *pHero = TC_HANDLE_TO_PHERO(handle_self);
	if( cGame::FindHeroTeam(pHero) == -1 )
		return false;

	if( !(pHero->HaveGhostBall() && pHero->CanPassBall()) ||
		PassBallOutofRange(pos_target) )
	{
		return false;
	}

	sPosition pos;
	pos.x = pos_target.x;
	pos.y = pos_target.y;

	pHero->PassBall(pos);
	return true;
}

/*TC_CLIENT_API*/ bool __stdcall Tc_SnatchBall( IN TC_Handle handle_self, IN TC_Ball_Type ball_type )
{
	bool bRet = false;

	cHero *pHero = TC_HANDLE_TO_PHERO(handle_self);
	if( cGame::FindHeroTeam(pHero) == -1 )
		return bRet;

	if( ball_type == TC_GHOST_BALL && pHero->CanSnatchGhostBall() ||
		ball_type == TC_GOLD_BALL && pHero->CanSnatchGoldBall() )
	{
		bRet = pHero->SnatchBall(ball_type);
		pHero->m_SnatchInterval = SNATCH_INTERVAL;
	}

	return bRet;
}

int FindHeroIndex(cHero *p_pHero)
{
	if( p_pHero == g_HeroTeam1[0] )
		return 0;
	else if( p_pHero == g_HeroTeam1[1] )
		return 1;
	else if( p_pHero == g_HeroTeam2[0] )
		return 2;
	else if( p_pHero == g_HeroTeam2[1] )
		return 3;
	else
		return -1;
}

/*TC_CLIENT_API*/ bool __stdcall Tc_GetHeroInfo( IN TC_Handle handle_self, IN OUT PTC_Hero p_hero_info )
{
	if(!p_hero_info)
		return false;
	else
		memset(p_hero_info, 0, sizeof(TC_Hero));

	cHero *pHero = TC_HANDLE_TO_PHERO(handle_self);
	if( cGame::FindHeroTeam(pHero) == -1 )
		return false;

	int index = FindHeroIndex(pHero);
	if( p_hero_info != NULL )
	{
		memcpy(p_hero_info, &g_ClientHeros[index], sizeof(TC_Hero));
	}
	return true;
}

//
// internal use only
//
static int FindBallIndex(cBall *p_pBall)
{
	for(int i = 0; i < BALL_NUM; i++)
	{
		if( p_pBall == g_Balls[i] )
			return i;
	}

	return -1;
}
/*TC_CLIENT_API*/ bool __stdcall Tc_GetBallInfo( IN TC_Handle handle_ball, IN OUT PTC_Ball p_ball_info )
{
	if(p_ball_info)
		memset(p_ball_info, 0, sizeof(TC_Ball));
	else
		return false;

	cBall *pBall = TC_HANDLE_TO_PBALL(handle_ball);
	if( cGame::FindBall(pBall) == -1 )
		return false;

	int index = FindBallIndex(pBall);
	if( p_ball_info != NULL )
	{
		memcpy(p_ball_info, &g_ClientBalls[index], sizeof(TC_Ball));
	}
	return true;
}


/*TC_CLIENT_API*/ bool  __stdcall Tc_GetEnemyInfo( IN TC_Handle handle_enemy, IN OUT PTC_Hero p_enemy_info )
{
	if( p_enemy_info )
		memset(p_enemy_info, 0, sizeof(TC_Hero));
	else
		return false;

	cHero *pHero = TC_HANDLE_TO_ENEMY_PHERO(handle_enemy);
	if( cGame::FindHeroTeam(pHero) == -1 )
		return false;

	if( p_enemy_info )
	{
		int index = FindHeroIndex(pHero);
		memcpy(p_enemy_info, &g_ClientHeros[index], sizeof(TC_Hero));
	}


	return true;
}

/*TC_CLIENT_API*/ TC_Handle __stdcall Tc_GetHeroHandle( IN TC_Team_Member member )
{
	return TC_PHERO_TO_HANDLE(g_Heros[g_IndexOffset + member]);
}

/*TC_CLIENT_API*/ TC_Handle __stdcall Tc_GetEnemyHandle( IN TC_Team_Member member )
{
	int i = (g_IndexOffset == 2) ? 0 : 2;

	return TC_ENEMY_PHERO_TO_HANDLE(g_Heros[i+member]);
}

/*TC_CLIENT_API*/ TC_Handle __stdcall Tc_GetBallHandle( IN TC_Ball_Type ball_type )
{
	return TC_PBALL_TO_HANDLE(g_Balls[ball_type]);
}

/*TC_CLIENT_API*/ bool __stdcall Tc_Stop( IN TC_Handle handle_self )
{
	cHero *pHero = TC_HANDLE_TO_PHERO(handle_self);
	if( cGame::FindHeroTeam(pHero) == -1 )
		return false;

	pHero->setAction(ACTION_HERO_STOP);
	return true;
}

/*TC_CLIENT_API*/ bool __stdcall Tc_GetOwnGate( IN TC_Handle handle_self, IN OUT PTC_Gate p_own_gate )
{
	if( p_own_gate )
		memset(p_own_gate, 0, sizeof(TC_Gate));
	else
		return false;

	cHero *pHero = TC_HANDLE_TO_PHERO(handle_self);
	int index = cGame::FindHeroTeam(pHero);
	if(index == -1)
		return false;

	p_own_gate->x = g_Gate[index].x;
	p_own_gate->y_lower = g_Gate[index].y_lower;
	p_own_gate->y_upper = g_Gate[index].y_upper;
	return true;
}

/*TC_CLIENT_API*/ bool __stdcall Tc_GetEnemyGate( IN TC_Handle handle_enemy, IN OUT PTC_Gate p_enemy_gate )
{
	if( p_enemy_gate )
		memset(p_enemy_gate, 0, sizeof(TC_Gate));
	else
		return false;

	cHero *pHero = TC_HANDLE_TO_ENEMY_PHERO(handle_enemy);
	int index = cGame::FindHeroTeam(pHero);
	if(index == -1)
		return false;

	p_enemy_gate->x = g_Gate[index].x;
	p_enemy_gate->y_lower = g_Gate[index].y_lower;
	p_enemy_gate->y_upper = g_Gate[index].y_upper;
	return true;
}

/*TC_CLIENT_API*/ bool __stdcall Tc_CanBeSpelled( IN TC_Handle handle_self, IN TC_Handle handle_enemy )
{
	cHero *pHeroEnemy;
	cHero *pHeroSelf = TC_HANDLE_TO_PHERO(handle_self);

	if( pHeroSelf->m_objectType != HERO_HARRY )
		pHeroEnemy = TC_HANDLE_TO_ENEMY_PHERO(handle_enemy);
	else
	{
		pHeroEnemy = TC_HANDLE_TO_PHERO(handle_enemy);
		if( pHeroEnemy == pHeroSelf )
			return false;
	}

	if( cGame::FindHeroTeam(pHeroEnemy) == -1 ||
		cGame::FindHeroTeam(pHeroSelf) == -1 )
	{
		return false;
	}

	if( !SpellInRange(pHeroEnemy, pHeroSelf) )
		return false;

	if( pHeroSelf->m_objectType != HERO_HARRY )
		return pHeroEnemy->CanBeSpelled();
	else
	{
		return (
			pHeroEnemy->GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) ||
			pHeroEnemy->GetStateMachine()->IsInState(cSpelledByVoldemortState::Instance()) ||
			pHeroEnemy->GetStateMachine()->IsInState(cSpelledByMalfoyState::Instance())
			);
	}
}

/*TC_CLIENT_API*/ bool __stdcall Tc_GetGameInfo( TC_Handle handle_self, PTC_GameInfo p_game_info )
{
	if( !p_game_info )
		return false;
	else
		memset(p_game_info, 0, sizeof(TC_GameInfo));

	cHero *pHeroSelf = TC_HANDLE_TO_PHERO(handle_self);
	int index = cGame::FindHeroTeam(pHeroSelf);
	if( index == -1 )
		return false;

	p_game_info->score_self = g_TeamScore[index];
	p_game_info->score_enemy = g_TeamScore[index ? 0 : 1];
	p_game_info->game_time_left = g_dwGameTime - g_GameTime - (GetTickCount() - g_TurnStartTime);

	return true;
}

/*TC_CLIENT_API*/ bool __stdcall Tc_GetForbiddenArea( PTC_ForbiddenArea p_forbidden_area )
{
	if( p_forbidden_area )
	{
		memcpy(p_forbidden_area, &g_ForbiddenAreas[0], sizeof(TC_ForbiddenArea));

		return true;
	}

	return false;
}
