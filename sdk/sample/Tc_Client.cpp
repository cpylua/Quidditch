#include <Windows.h>
#include <tchar.h>
#include <math.h>

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include "../inc/TuringCup9ClientAPI.h"
#pragma comment(lib, "../lib/TuringCup9.lib")

#define CLIENT_EXPORT
#include "Tc_Client.h"

/*
	How to debug?

	Goto
	Project -. Properties --> Configuration Properties --> Debugging

	Copy "$(ProjectDir)TuringCup9.exe" 
	to the Command field.(Without quotation marks)
*/

CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(_T("Fancy"));
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_HERMIONE,L"Hermione");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_GINNY,L"Ginny");
}

#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
#define GOLDBALL_REACHABLE 3
TC_Handle hHero_1, hHero_2;
TC_Handle hEnemy_1, hEnemy_2;
TC_Handle hHeroWithBall, hHeroWithoutBall;
TC_Handle hEnemyWithBall, hEnemyWithoutBall;
TC_Handle hFreeBall, hGhostBall, hGoldBall;
TC_Ball iFreeBall, iGhostBall, iGoldBall;
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2, iHeroWithBall, iHeroWithoutBall, iEnemyWithBall, iEnemyWithoutBall;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_Direction iAttackDirection;

int GetState()
{

	if (iHero_1.b_ghostball == false && iHero_2.b_ghostball == false && iEnemy_1.b_ghostball == false && iEnemy_2.b_ghostball == false)
	{
		return GHOSTBALL_IN_NO_TEAM;
	}
	if (iHero_1.b_ghostball == true || iHero_2.b_ghostball == true)
	{
		if (iHero_1.b_ghostball == true)
		{
			iHeroWithBall = iHero_1;
			hHeroWithBall = hHero_1;
			iHeroWithoutBall = iHero_2;
			hHeroWithoutBall = hHero_2;
		}
		else 
		{
			iHeroWithBall = iHero_2;
			hHeroWithBall = hHero_2;
			iHeroWithoutBall = iHero_1;
			hHeroWithoutBall = hHero_2;
		}
		return GHOSTBALL_IN_SELF_TEAM;
	}
	if (iEnemy_1.b_ghostball == true || iEnemy_2.b_ghostball == true)
	{
		if (iEnemy_1.b_ghostball == true)
		{
			iEnemyWithBall = iEnemy_1;
			hEnemyWithBall = hEnemy_1;
			iEnemyWithoutBall = iEnemy_2;
			hEnemyWithoutBall = hEnemy_2;
		}
		else 
		{
			iEnemyWithBall = iEnemy_2;
			hEnemyWithBall = hEnemy_2;
			iEnemyWithoutBall = iEnemy_1;
			hEnemyWithoutBall = hEnemy_1;
		}
		return GHOSTBALL_IN_ENEMY_TEAM;
	}
}

TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pEnemy)
{
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x == pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y == pEnemy->pos.y)return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x == pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y == pEnemy->pos.y)return TC_DIRECTION_LEFT;
}
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)
{
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x == pBall->pos.x && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y == pBall->pos.y)return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x == pBall->pos.x && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y == pBall->pos.y)return TC_DIRECTION_LEFT;
}

TC_Direction GetAttackDirection(TC_Hero *pHero)
{
	if (pHero->pos.y >= iEnemyGate.y_lower-128)
	{
		if (iAttackDirection == TC_DIRECTION_RIGHT)
		{
			return TC_DIRECTION_RIGHTTOP;
		}
		else
		{
			return TC_DIRECTION_LEFTTOP;
		}
	}
	else
	{
		if (pHero->pos.y <= iEnemyGate.y_upper+2)
		{
			if (iAttackDirection == TC_DIRECTION_RIGHT)
			{
				return TC_DIRECTION_RIGHTBOTTOM;
			}
			else
			{
				return TC_DIRECTION_LEFTBOTTOM;
			}
		}
		else
		{
			return iAttackDirection;
		}
	}
}

CLIENT_EXPORT_API void __stdcall AI()
{
	hHero_1 = Tc_GetHeroHandle(TC_MEMBER1);
    hHero_2 = Tc_GetHeroHandle(TC_MEMBER2);
	hEnemy_1 = Tc_GetEnemyHandle(TC_MEMBER1);
	hEnemy_2 = Tc_GetEnemyHandle(TC_MEMBER2);
	hFreeBall = Tc_GetBallHandle(TC_FREE_BALL);
	hGhostBall = Tc_GetBallHandle(TC_GHOST_BALL);
	hGoldBall = Tc_GetBallHandle(TC_GOLD_BALL);

	Tc_GetBallInfo(hGoldBall,&iGoldBall);
	Tc_GetBallInfo(hFreeBall,&iFreeBall);
	Tc_GetBallInfo(hGhostBall,&iGhostBall);
	Tc_GetHeroInfo(hHero_1,&iHero_1);
	Tc_GetHeroInfo(hHero_2,&iHero_2);
	Tc_GetEnemyInfo(hEnemy_1,&iEnemy_1);
	Tc_GetEnemyInfo(hEnemy_2,&iEnemy_2);

	Tc_GetOwnGate(hHero_1,&iOwnGate);
	Tc_GetEnemyGate(hEnemy_1,&iEnemyGate);
	Tc_GetForbiddenArea(&iForbiddenArea);

	if (iOwnGate.x < iEnemyGate.x)
	{
		iAttackDirection = TC_DIRECTION_RIGHT;
	}
	else
	{
		iAttackDirection = TC_DIRECTION_LEFT;
	}
	int state = GetState();
	switch (state)
	{
	case GHOSTBALL_IN_SELF_TEAM:
		{
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
			break;
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
		{
			TC_Handle hHeroToBall, hHeroNotToBall;
			TC_Hero iHeroToBall, iHeroNotToBall;
			if (iAttackDirection == TC_DIRECTION_RIGHT)
			{
				if (iEnemyWithBall.pos.x - iOwnGate.x <= 8)
				{
					Tc_Spell(hHero_2,hEnemyWithBall);
				}
				if (iEnemyWithBall.pos.x < iForbiddenArea.left && iEnemyWithBall.pos.x-iOwnGate.x > 8)
				{
					Tc_Spell(hHero_1,hEnemyWithBall);
				}
			}
			else
			{
				if (iOwnGate.x - iEnemyWithBall.pos.x <= 8)
				{
					Tc_Spell(hHero_2,hEnemyWithBall);
				}
				if (iEnemyWithBall.pos.x > iForbiddenArea.right && iOwnGate.x-iEnemyWithBall.pos.x > 8)
				{
					Tc_Spell(hHero_1,hEnemyWithBall);
				}
			}
			if (iHero_1.b_snatch_ghostball == true)
			{
				if (Tc_SnatchBall(hHero_1,TC_GHOST_BALL) == true)
				{
					break;
				}
			}
			if (iHero_2.b_snatch_ghostball == true)
			{
				if (Tc_SnatchBall(hHero_2,TC_GHOST_BALL) == true)
				{
					break;
				}
			}
			Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithoutBall));
			break;
		}
	case GHOSTBALL_IN_NO_TEAM:
		{
			if (iHero_1.b_snatch_ghostball == false) 
			{
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			}
			else
			{
				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
			}
			if (iHero_2.b_ghostball == false)
			{
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			}
			else 
			{
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
			}
			break;
		}
	case GOLDBALL_REACHABLE:
		{
			break;
		}
	default:
		{
			break;
		}
	}
}