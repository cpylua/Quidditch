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

	Copy "$(ProjectDir)bin\TuringCup9.exe" 
	to the Command field.(Without quotation marks)
*/

CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(_T("KVC"));
	
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_RON,L"KVC_Ron");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_MALFOY,L"KVC_Malfoy");
}

#define SNATCHBALL_DISTANCE 150
#define DEFANCE_DISTANCE 200

#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
#define GOLDBALL_VISIABLE 3

TC_Handle hHero_1,hHero_2,hHeroWithBall,hHeroWithoutBall;
TC_Handle hEnemy_1,hEnemy_2,hEnemyWithBall,hEnemyWithoutBall;
TC_Handle hGoldBall,hGhostBall,hFreeBall;
TC_Hero iHero_1,iHero_2,iHeroWithBall,iHeroWithoutBall;
TC_Hero iEnemy_1,iEnemy_2,iEnemyWithBall,iEnemyWithoutBall;
TC_Ball iGoldBall,iGhostBall,iFreeBall;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_Direction iAttackDirection,iDefenceDirection;
TC_GameInfo iGameInfo;
int iState;

static bool bPassedByOwnHero = false;

int RealDistance(TC_Hero Hero,TC_Hero Enemy)
{
	return (int)sqrt(double((Hero.pos.x - Enemy.pos.x) * (Hero.pos.x - Enemy.pos.x) + (Hero.pos.y - Enemy.pos.y) * (Hero.pos.y - Enemy.pos.y)));
}

int RealDistance(TC_Hero Hero,TC_Ball Ball)
{
	if (Ball.type == TC_GHOST_BALL || Ball.type == TC_FREE_BALL)
	{
		return (int)sqrt(double((Hero.pos.x + TC_HERO_WIDTH - Ball.pos.x - TC_BALL_WIDTH) * (Hero.pos.x + TC_HERO_HEIGHT - Ball.pos.x - TC_BALL_HEIGHT) + 
			(Hero.pos.y + TC_HERO_WIDTH - Ball.pos.y - TC_BALL_WIDTH) * (Hero.pos.y + TC_HERO_HEIGHT - Ball.pos.y - TC_BALL_HEIGHT)));
	}
	else 
	{
		return (int)sqrt(double((Hero.pos.x - Ball.pos.x) * (Hero.pos.x - Ball.pos.x) + (Hero.pos.y - Ball.pos.y) * (Hero.pos.y - Ball.pos.y)));
	}
}

int VisualDistance(TC_Position Pos_1,TC_Position Pos_2)
{
	int Distance_x = (int)fabs((double)(Pos_1.x - Pos_2.x));
	int Distance_y = (int)fabs((double)(Pos_1.y - Pos_2.y));
	return  int(0.414*min(Distance_x,Distance_y) + max(Distance_x,Distance_y));
}

bool BeInTheRectangle(TC_Position posLeftTop,TC_Position posRightBottom,TC_Position posForJudgement)
{
	if ( posLeftTop.x > posRightBottom.x)
	{
		TC_Position posTemp;
		posTemp = posLeftTop;
		posLeftTop = posRightBottom;
		posRightBottom = posTemp;
	}
	if (posForJudgement.x >= posLeftTop.x && posForJudgement.x <= posRightBottom.x 
		&& posForJudgement.y >= posLeftTop.y && posForJudgement.y <= posRightBottom.y )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool BeCanPassBall()
{
	if ( iEnemy_1.b_snatch_ghostball == true || ( iEnemy_1.steps_before_next_snatch <= 5 && BeInTheRectangle(iHeroWithBall.pos,iHeroWithoutBall.pos,iEnemy_1.pos)) 
		|| iEnemy_2.b_snatch_ghostball == true || ( iEnemy_2.steps_before_next_snatch <= 5 && BeInTheRectangle(iHeroWithBall.pos,iHeroWithoutBall.pos,iEnemy_2.pos)) )
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Kvc_Move(TC_Handle hHero,TC_Hero iHero,TC_Direction direction)
{
	TC_Position posTarget;
	int l;
	if (iHero.steps_before_next_snatch == 0)
	{
		l = (25-1)*7 + 50;
	}
	else
	{
		l = (iHero.steps_before_next_snatch-1)*7 +100;
	}
	
	if ( direction == TC_DIRECTION_RIGHT )
	{
		posTarget.x = iHero.pos.x + l + 96;
		posTarget.y = iHero.pos.y + 32;
		if (posTarget.x + TC_BALL_WIDTH >= iForbiddenArea.right.left)
		{
			posTarget.x = iForbiddenArea.right.left - TC_BALL_WIDTH - 1;
		}
		return Tc_PassBall(hHero,posTarget);
	}
	if ( direction == TC_DIRECTION_RIGHTTOP )
	{
		int dy = iGhostBall.pos.y - (iEnemyGate.y_lower - TC_HERO_HEIGHT - 1);
		if ( l >= 1.414 * dy )
		{
			posTarget.x = iHero.pos.x + l - 0.414 * dy ;
			posTarget.y = iHero.pos.y - dy ;
		}
		else
		{
			posTarget.x = iHero.pos.x + l/1.414 ;
			posTarget.y = iHero.pos.y - l/1.414 ;
		}
		if (posTarget.x + TC_BALL_WIDTH >= iForbiddenArea.right.left)
		{
			posTarget.x = iForbiddenArea.right.left - TC_BALL_WIDTH - 1;
		}
		return Tc_PassBall(hHero,posTarget);
	}
	if ( direction == TC_DIRECTION_RIGHTBOTTOM )
	{
		int dy = iEnemyGate.y_upper + 1 - iGhostBall.pos.y ;
		if ( l >= 1.414 * dy )
		{
			posTarget.x = iHero.pos.x + l - 0.414 * dy ;
			posTarget.y = iHero.pos.y + dy ;
		}
		else
		{
			posTarget.x = iHero.pos.x + l/1.414 ;
			posTarget.y = iHero.pos.y + l/1.414 ;
		}
		if (posTarget.x + TC_BALL_WIDTH >= iForbiddenArea.right.left)
		{
			posTarget.x = iForbiddenArea.right.left - TC_BALL_WIDTH - 1;
		}
		return Tc_PassBall(hHero,posTarget);
	}
	if ( direction == TC_DIRECTION_LEFT )
	{
		posTarget.x = iHero.pos.x - l - 32;
		posTarget.y = iHero.pos.y + 32;
		if (posTarget.x <= iForbiddenArea.left.right)
		{
			posTarget.x = iForbiddenArea.left.right + 1;
		}
		return Tc_PassBall(hHero,posTarget);
	}
	if ( direction == TC_DIRECTION_LEFTTOP )
	{
		int dy = iGhostBall.pos.y - (iEnemyGate.y_lower - TC_HERO_HEIGHT - 1);
		if ( l >= 1.414 * dy )
		{
			posTarget.x = iHero.pos.x - l + 0.414 * dy ;
			posTarget.y = iHero.pos.y - dy ;
		}
		else
		{
			posTarget.x = iHero.pos.x - l/1.414 ;
			posTarget.y = iHero.pos.y - l/1.414 ;
		}
		if (posTarget.x <= iForbiddenArea.left.right)
		{
			posTarget.x = iForbiddenArea.left.right + 1;
		}
		return Tc_PassBall(hHero,posTarget);
	}
	if ( direction == TC_DIRECTION_LEFTBOTTOM )
	{
		int dy = iEnemyGate.y_upper + 1 - iGhostBall.pos.y ;
		if ( l >= 1.414 * dy )
		{
			posTarget.x = iHero.pos.x - l + 0.414 * dy ;
			posTarget.y = iHero.pos.y + dy ;
		}
		else
		{
			posTarget.x = iHero.pos.x - l/1.414 ;
			posTarget.y = iHero.pos.y + l/1.414 ;
		}
		if (posTarget.x <= iForbiddenArea.left.right)
		{
			posTarget.x = iForbiddenArea.left.right + 1;
		}
		return Tc_PassBall(hHero,posTarget);
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
	if (pBall->type == TC_GHOST_BALL || pBall->type == TC_FREE_BALL)
	{
		if ((pHero->pos.x + TC_HERO_WIDTH/2) > (pBall->pos.x + TC_BALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) > (pBall->pos.y + TC_BALL_HEIGHT/2))return TC_DIRECTION_LEFTTOP;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) == (pBall->pos.x + TC_BALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) > (pBall->pos.y + TC_BALL_HEIGHT/2))return TC_DIRECTION_TOP;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) < (pBall->pos.x + TC_BALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) > (pBall->pos.y + TC_BALL_HEIGHT/2))return TC_DIRECTION_RIGHTTOP;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) < (pBall->pos.x + TC_BALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) == (pBall->pos.y + TC_BALL_HEIGHT/2))return TC_DIRECTION_RIGHT;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) < (pBall->pos.x + TC_BALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) < (pBall->pos.y + TC_BALL_HEIGHT/2))return TC_DIRECTION_RIGHTBOTTOM;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) == (pBall->pos.x + TC_BALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) < (pBall->pos.y + TC_BALL_HEIGHT/2))return TC_DIRECTION_BOTTOM;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) > (pBall->pos.x + TC_BALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) < (pBall->pos.y + TC_BALL_HEIGHT/2))return TC_DIRECTION_LEFTBOTTOM;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) > (pBall->pos.x + TC_BALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) == (pBall->pos.y + TC_BALL_HEIGHT/2))return TC_DIRECTION_LEFT;
	}
	else
	{
		if ((pHero->pos.x + TC_HERO_WIDTH/2) > (pBall->pos.x + TC_GOLDBALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) > (pBall->pos.y + TC_GOLDBALL_HEIGHT/2))return TC_DIRECTION_LEFTTOP;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) == (pBall->pos.x + TC_GOLDBALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) > (pBall->pos.y + TC_GOLDBALL_HEIGHT/2))return TC_DIRECTION_TOP;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) < (pBall->pos.x + TC_GOLDBALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) > (pBall->pos.y + TC_GOLDBALL_HEIGHT/2))return TC_DIRECTION_RIGHTTOP;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) < (pBall->pos.x + TC_GOLDBALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) == (pBall->pos.y + TC_GOLDBALL_HEIGHT/2))return TC_DIRECTION_RIGHT;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) < (pBall->pos.x + TC_GOLDBALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) < (pBall->pos.y + TC_GOLDBALL_HEIGHT/2))return TC_DIRECTION_RIGHTBOTTOM;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) == (pBall->pos.x + TC_GOLDBALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) < (pBall->pos.y + TC_GOLDBALL_HEIGHT/2))return TC_DIRECTION_BOTTOM;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) > (pBall->pos.x + TC_GOLDBALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) < (pBall->pos.y + TC_GOLDBALL_HEIGHT/2))return TC_DIRECTION_LEFTBOTTOM;
		if ((pHero->pos.x + TC_HERO_WIDTH/2) > (pBall->pos.x + TC_GOLDBALL_WIDTH/2) && (pHero->pos.y + TC_HERO_HEIGHT/2) == (pBall->pos.y + TC_GOLDBALL_HEIGHT/2))return TC_DIRECTION_LEFT;
	}
}
TC_Direction GetAttackDirection(TC_Hero *pHero)
{
	if (pHero->pos.y > iEnemyGate.y_upper && pHero->pos.y < iEnemyGate.y_lower - TC_HERO_HEIGHT)
	{
		return iAttackDirection;
	}
	else if (pHero->pos.y >= iEnemyGate.y_lower - TC_HERO_HEIGHT)
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
	else if (pHero->pos.y <= iEnemyGate.y_upper)
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
}
void GetAllInfo()
{
	hHero_1 = Tc_GetHeroHandle(TC_MEMBER1);
	hHero_2 = Tc_GetHeroHandle(TC_MEMBER2);
	hEnemy_1 = Tc_GetEnemyHandle(TC_MEMBER1);
	hEnemy_2 = Tc_GetEnemyHandle(TC_MEMBER2);
	hFreeBall = Tc_GetBallHandle(TC_FREE_BALL);
	hGhostBall = Tc_GetBallHandle(TC_GHOST_BALL);
	hGoldBall = Tc_GetBallHandle(TC_GOLD_BALL);

	Tc_GetHeroInfo(hHero_1,&iHero_1);
	Tc_GetHeroInfo(hHero_2,&iHero_2);
	Tc_GetEnemyInfo(hEnemy_1,&iEnemy_1);
	Tc_GetEnemyInfo(hEnemy_2,&iEnemy_2);
	Tc_GetBallInfo(hFreeBall,&iFreeBall);
	Tc_GetBallInfo(hGhostBall,&iGhostBall);
	Tc_GetBallInfo(hGoldBall,&iGoldBall);

	Tc_GetOwnGate(hHero_1,&iOwnGate);
	Tc_GetEnemyGate(hEnemy_1,&iEnemyGate);
	Tc_GetForbiddenArea(&iForbiddenArea);
	Tc_GetGameInfo(hHero_1,&iGameInfo);

	if (iOwnGate.x < iEnemyGate.x)
	{
		iAttackDirection = TC_DIRECTION_RIGHT;
		iDefenceDirection = TC_DIRECTION_LEFT;
	}
	else 
	{
		iAttackDirection = TC_DIRECTION_LEFT;
		iDefenceDirection = TC_DIRECTION_RIGHT;
	}
}

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
			hHeroWithoutBall = hHero_1;
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

CLIENT_EXPORT_API void __stdcall AI()
{
	GetAllInfo();
	iState = GetState();
	switch(iState)
	{
	case GHOSTBALL_IN_NO_TEAM:
		{
			if (iHero_1.b_ghostball==false && iHero_2.b_ghostball == false)
			{
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			}
			if (iHero_1.b_snatch_ghostball == true && iHero_2.b_snatch_ghostball == true)
			{
				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
			}
			if (iHero_1.b_snatch_ghostball == true && iHero_2.b_snatch_ghostball == false)
			{
				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			}
			if (iHero_1.b_snatch_ghostball == false && iHero_2.b_snatch_ghostball == true)
			{
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			}
			break;
		}
	case GHOSTBALL_IN_SELF_TEAM:
		{
			if (iHeroWithBall.abnormal_type == TC_SPELLED_BY_FREEBALL || iHeroWithBall.abnormal_type == TC_SPELLED_BY_HERMIONE 
				|| iHeroWithBall.abnormal_type == TC_SPELLED_BY_MALFOY)
			{
				if ( iHeroWithBall.abnormal_type == TC_SPELLED_BY_HERMIONE )
				{
					Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
				}
				if (iHeroWithoutBall.b_snatch_ghostball == true)
				{
					Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
				}
				else
				{
					TC_Hero iEnemyToBeSpelled;
					TC_Handle hEnemyToBeSpelled;
					if ( RealDistance(iEnemy_1,iGhostBall) <= RealDistance(iEnemy_2,iGhostBall) )
					{
						iEnemyToBeSpelled = iEnemy_1;
						hEnemyToBeSpelled = hEnemy_1;
					}
					else
					{
						iEnemyToBeSpelled = iEnemy_2;
						hEnemyToBeSpelled = hEnemy_2;
					}
					if (((iEnemyToBeSpelled.steps_before_next_snatch < iHeroWithoutBall.steps_before_next_snatch
						&& RealDistance(iEnemy_1,iGhostBall) > TC_SNATCH_DISTANCE_GHOST + 30*7
						&&iHeroWithBall.abnormal_type == TC_SPELLED_BY_FREEBALL || iHeroWithBall.abnormal_type == TC_SPELLED_BY_MALFOY)
						||(iEnemyToBeSpelled.steps_before_next_snatch < iHeroWithoutBall.steps_before_next_snatch
						&& RealDistance(iEnemy_1,iGhostBall) > TC_SNATCH_DISTANCE_GHOST + 30*4 && iHeroWithBall.abnormal_type == TC_SPELLED_BY_HERMIONE))
						&&(iHeroWithoutBall.b_can_spell && Tc_CanBeSpelled(hHeroWithoutBall,hEnemyToBeSpelled)
						&&(iHeroWithoutBall.curr_blue - iHeroWithoutBall.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHeroWithoutBall.spell_cost))
						)
					{
						Tc_Spell(hHeroWithoutBall,hEnemyToBeSpelled);
					}
					else
					{
						Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHeroWithBall));
					}
				}
			}
			else
			{
				if (iAttackDirection == TC_DIRECTION_RIGHT)
				{
//向右进攻，己方禁区
					if ( iHeroWithBall.pos.x <= iForbiddenArea.left.right )
					{
						if ( iEnemy_1.b_is_spelling == true || iEnemy_2.b_is_spelling == true )
						{
							if ( RealDistance(iHeroWithBall,iHeroWithoutBall) < TC_PASSBALL_DISTANCE )
							{
								Tc_PassBall(hHeroWithBall,iHero_2.pos);
								Tc_Stop(hHeroWithoutBall);
							}
							else
							{
								TC_Position iTarget;
								iTarget.x = iHeroWithBall.pos.x + 510;
								iTarget.y = iHeroWithBall.pos.y;
								Tc_PassBall(hHeroWithBall,iTarget);
								Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
							}
						}
						else
						{
							if ( BeCanPassBall() == false || iHeroWithBall.pos.x >= iHeroWithoutBall.pos.x 
								|| RealDistance(iHeroWithBall,iHeroWithoutBall) >= TC_PASSBALL_DISTANCE - 10 )
							{
								if ( BeCanPassBall() == false 
									||(iEnemy_1.pos.x > iHeroWithBall.pos.x && iEnemy_1.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
									||(iEnemy_2.pos.x > iHeroWithBall.pos.x && iEnemy_2.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
									)
								{
									Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
								}
								else
								{
									Kvc_Move(hHeroWithBall,iHeroWithBall,GetAttackDirection(&iHeroWithBall));
								}								
								if ( BeCanPassBall() == false || iHeroWithBall.pos.x >= iHeroWithoutBall.pos.x )
								{
									Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
								}
								else
								{
									Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHeroWithBall));
								}
							}
							else
							{
								TC_Position posTarget;
								posTarget.x = iHeroWithoutBall.pos.x + 90;
								posTarget.y = iHeroWithoutBall.pos.y;
								if( Tc_PassBall(hHeroWithBall,posTarget) )
								{
									bPassedByOwnHero = true;
								}
								else
								{
									bPassedByOwnHero = false;
								}
								Tc_Move(hHeroWithoutBall,iAttackDirection);
							}
						}
					}
					else
					{
//向右进攻，自由区
						if (iHeroWithBall.pos.x > iForbiddenArea.left.right && iHeroWithBall.pos.x + TC_HERO_WIDTH < iForbiddenArea.right.left)
						{
							if (iEnemy_1.b_is_spelling == true || iEnemy_2.b_is_spelling == true)
							{
								if ( RealDistance(iHeroWithBall,iHeroWithoutBall) < TC_PASSBALL_DISTANCE )
								{
									Tc_PassBall(hHeroWithBall,iHero_2.pos);
									Tc_Stop(hHeroWithoutBall);
								}
								else
								{
									TC_Position iTarget;
									iTarget.x = iHeroWithBall.pos.x + 510;
									iTarget.y = iHeroWithBall.pos.y;
									Tc_PassBall(hHeroWithBall,iTarget);
									Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
								}
							}
							else
							{
								if ( BeCanPassBall() == false || iHeroWithBall.pos.x >= iHeroWithoutBall.pos.x 
									|| RealDistance(iHeroWithBall,iHeroWithoutBall) >= TC_PASSBALL_DISTANCE - 100
									|| iHeroWithoutBall.pos.x + TC_HERO_WIDTH >= iForbiddenArea.right.left )
								{
									if ( BeCanPassBall() == false 
										||(iEnemy_1.pos.x > iHeroWithBall.pos.x && iEnemy_1.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
										||(iEnemy_2.pos.x > iHeroWithBall.pos.x && iEnemy_2.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
										)
									{
										Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
									}
									else
									{
										Kvc_Move(hHeroWithBall,iHeroWithBall,GetAttackDirection(&iHeroWithBall));
										if ( iHeroWithBall.pos.x > iEnemy_1.pos.x && iHeroWithBall.pos.x < iEnemy_2.pos.x 
											&& (iHeroWithoutBall.b_can_spell && Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2)
											&&(iHeroWithoutBall.curr_blue - iHeroWithoutBall.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHeroWithoutBall.spell_cost))
											)
										{
											Tc_Spell(hHeroWithoutBall,hEnemy_2);
										}
										if ( iHeroWithBall.pos.x < iEnemy_1.pos.x && iHeroWithBall.pos.x > iEnemy_2.pos.x 
											&& (iHeroWithoutBall.b_can_spell && Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_1)
											&&(iHeroWithoutBall.curr_blue - iHeroWithoutBall.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHeroWithoutBall.spell_cost))
											)
										{
											Tc_Spell(hHeroWithoutBall,hEnemy_1);
										}
									}
									if ( iHeroWithoutBall.pos.x + TC_HERO_WIDTH >= iForbiddenArea.right.left)
									{								
										Tc_Move(hHeroWithoutBall,iDefenceDirection);
									}
									else
									{
										if ( BeCanPassBall() == false || iHeroWithBall.pos.x >= iHeroWithoutBall.pos.x )
										{
											Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
										}
										else
										{
											Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHeroWithBall));
										}
									}
								}
								else
								{
									if ( RealDistance( iHeroWithBall,iHeroWithoutBall ) >= 150 
										|| (iHeroWithoutBall.pos.x + TC_HERO_WIDTH >= iForbiddenArea.right.left - 15 
										&& iHeroWithoutBall.pos.x + TC_HERO_WIDTH < iForbiddenArea.right.left && iHeroWithBall.pos.x < iHeroWithoutBall.pos.x) )
									{
										TC_Position posTarget;
										if ( iHeroWithoutBall.pos.x + 90 + TC_BALL_WIDTH >= iForbiddenArea.right.left)
										{
											posTarget.x = iForbiddenArea.right.left - TC_BALL_WIDTH - 1;
										}
										else
										{
											posTarget.x = iHeroWithoutBall.pos.x + 90;
										}
										posTarget.y = iHeroWithoutBall.pos.y + 32;

										if( Tc_PassBall(hHeroWithBall,posTarget) )
										{
											bPassedByOwnHero = true;
										}
										else
										{
											bPassedByOwnHero = false;
										}
										if ( iHeroWithoutBall.pos.x == posTarget.x )
										{
											Tc_Stop(hHeroWithoutBall);
										}
										else
										{
											Tc_Move(hHeroWithoutBall,iAttackDirection);
										}
									}
									else
									{
										if ( iEnemy_1.b_snatch_ghostball == true || RealDistance(iEnemy_1,iGhostBall) < 64
											|| iEnemy_2.b_snatch_ghostball == true || RealDistance(iEnemy_2,iGhostBall) < 64
											||(iEnemy_1.pos.x > iHeroWithBall.pos.x && iEnemy_1.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
											||(iEnemy_2.pos.x > iHeroWithBall.pos.x && iEnemy_2.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
											)
										{
											Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
										}
										else
										{
											Kvc_Move(hHeroWithBall,iHeroWithBall,GetAttackDirection(&iHeroWithBall));
										}
										if ( iHeroWithoutBall.pos.x + TC_HERO_WIDTH >= iForbiddenArea.right.left - 15)
										{
											if ( iHeroWithoutBall.pos.x + TC_HERO_WIDTH == iForbiddenArea.right.left - 15 )
											{
												Tc_Stop(hHeroWithoutBall);
											}
											else
											{
												Tc_Move(hHeroWithoutBall,iDefenceDirection);
											}
										}
										else
										{
											Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));	
										}

									}
								}
							}
						}

//向右进攻，对方禁区
						else
						{
							if ( iHeroWithoutBall.pos.x + TC_HERO_WIDTH == iForbiddenArea.right.left - 10 )
							{
								Tc_Stop(hHeroWithoutBall);
							}
							else
							{
								if (iHeroWithoutBall.pos.x + TC_HERO_WIDTH > iForbiddenArea.right.left - 10)
								{
									Tc_Move(hHeroWithoutBall,iDefenceDirection);
								}
								else
								{
									Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
								}
							}
							if ( (iEnemy_1.b_is_spelling && (iEnemy_1.type == TC_HERO_GINNY || iEnemy_1.type == TC_HERO_RON || iEnemy_1.type == TC_HERO_MALFOY)
								&& iHeroWithBall.pos.x < iEnemyGate.x - 22 )
								|| (iEnemy_2.b_is_spelling && (iEnemy_2.type == TC_HERO_GINNY || iEnemy_2.type == TC_HERO_RON || iEnemy_2.type == TC_HERO_MALFOY)
								&& iHeroWithBall.pos.x < iEnemyGate.x - 22 )
								)
							{
								if ( BeCanPassBall() == true )
								{
									if ( Tc_PassBall(hHeroWithBall,iHeroWithoutBall.pos) )
									{
										Tc_Stop(hHeroWithoutBall);
										bPassedByOwnHero = true;
									}
									else
									{
										Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
										Tc_Stop(hHeroWithoutBall);
									}
								}
								else
								{
									TC_Position posTarget;
									posTarget.x = iHeroWithoutBall.pos.x;
									if ( iEnemyGate.y_upper - iHeroWithoutBall.pos.y >= iHeroWithoutBall.pos.y + TC_HERO_HEIGHT - iEnemyGate.y_lower)
									{
										posTarget.y = iHeroWithoutBall.pos.y + TC_HERO_HEIGHT + 50;
									}
									else
									{
										posTarget.y = iHeroWithoutBall.pos.y - 50;
									}
									if (Tc_PassBall(hHeroWithBall,posTarget))
									{

										Tc_Stop(hHeroWithoutBall);
									}
									else
									{
										Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
										Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
									}
								}
							}
							else
							{
								Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
								Tc_Stop(hHeroWithoutBall);
							}
						}
					}
				}

//向左进攻
				else
				{
					if ( iHeroWithBall.pos.x + TC_HERO_WIDTH >= iForbiddenArea.right.left )
					{
						if ( iEnemy_1.b_is_spelling == true || iEnemy_2.b_is_spelling == true )
						{
							if ( RealDistance(iHeroWithBall,iHeroWithoutBall) < TC_PASSBALL_DISTANCE )
							{
								Tc_PassBall(hHeroWithBall,iHero_2.pos);
								Tc_Stop(hHeroWithoutBall);
							}
							else
							{
								TC_Position iTarget;
								iTarget.x = iHeroWithBall.pos.x - 510;
								iTarget.y = iHeroWithBall.pos.y;
								Tc_PassBall(hHeroWithBall,iTarget);
								Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
							}
						}
						else
						{
							if ( BeCanPassBall() == false || iHeroWithBall.pos.x <= iHeroWithoutBall.pos.x 
								|| RealDistance(iHeroWithBall,iHeroWithoutBall) >= TC_PASSBALL_DISTANCE - 10 )
							{
								if (BeCanPassBall() == false
									||(iEnemy_1.pos.x < iHeroWithBall.pos.x && iEnemy_1.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
									||(iEnemy_2.pos.x < iHeroWithBall.pos.x && iEnemy_2.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
									)
								{
									Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
								}
								else
								{
									Kvc_Move(hHeroWithBall,iHeroWithBall,GetAttackDirection(&iHeroWithBall));
								}
								if ( BeCanPassBall() == false || iHeroWithBall.pos.x <= iHeroWithoutBall.pos.x )
								{
									Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
								}
								else
								{
									Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHeroWithBall));
								}
							}
							else
							{
								TC_Position posTarget;
								posTarget.x = iHeroWithoutBall.pos.x - 90;
								posTarget.y = iHeroWithoutBall.pos.y;
								if( Tc_PassBall(hHeroWithBall,posTarget) )
								{
									bPassedByOwnHero = true;
								}
								else
								{
									bPassedByOwnHero = false;
								}
								Tc_Move(hHeroWithoutBall,iAttackDirection);
							}
						}
					}
					else
					{
//自由区域
						if (iHeroWithBall.pos.x > iForbiddenArea.left.right && iHeroWithBall.pos.x + TC_HERO_WIDTH < iForbiddenArea.right.left)
						{
							if ( (iEnemy_1.b_is_spelling == true && iEnemy_1.type == TC_HERO_MALFOY) 
								|| (iEnemy_1.b_is_spelling == true && iEnemy_1.type == TC_HERO_MALFOY) )
							{
								if ( RealDistance(iHeroWithBall,iHeroWithoutBall) < TC_PASSBALL_DISTANCE )
								{
									Tc_PassBall(hHeroWithBall,iHero_2.pos);
									Tc_Stop(hHeroWithoutBall);
								}
								else
								{
									TC_Position iTarget;
									iTarget.x = iHeroWithBall.pos.x - 510;
									iTarget.y = iHeroWithBall.pos.y;
									Tc_PassBall(hHeroWithBall,iTarget);
									Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
								}
							}
							else
							{
								if ( BeCanPassBall() == false || iHeroWithBall.pos.x <= iHeroWithoutBall.pos.x 
									|| RealDistance(iHeroWithBall,iHeroWithoutBall) >= TC_PASSBALL_DISTANCE - 100
									|| iHeroWithoutBall.pos.x <= iForbiddenArea.left.right )
								{
									if ( BeCanPassBall() == false 
										||(iEnemy_1.pos.x < iHeroWithBall.pos.x && iEnemy_1.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
										||(iEnemy_2.pos.x < iHeroWithBall.pos.x && iEnemy_2.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
										)
									{
										Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
									}
									else
									{
										Kvc_Move(hHeroWithBall,iHeroWithBall,GetAttackDirection(&iHeroWithBall));
										if ( iHeroWithBall.pos.x > iEnemy_1.pos.x && iHeroWithBall.pos.x < iEnemy_2.pos.x 
											&& (iHeroWithoutBall.b_can_spell && Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2)
											&&(iHeroWithoutBall.curr_blue - iHeroWithoutBall.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHeroWithoutBall.spell_cost))
											)
										{
											Tc_Spell(hHeroWithoutBall,hEnemy_2);
										}
										if ( iHeroWithBall.pos.x < iEnemy_1.pos.x && iHeroWithBall.pos.x > iEnemy_2.pos.x 
											&& (iHeroWithoutBall.b_can_spell && Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_1)
											&&(iHeroWithoutBall.curr_blue - iHeroWithoutBall.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHeroWithoutBall.spell_cost))
											)
										{
											Tc_Spell(hHeroWithoutBall,hEnemy_1);
										}
									}
									if ( iHeroWithoutBall.pos.x <= iForbiddenArea.left.right )
									{								
										Tc_Move(hHeroWithoutBall,iDefenceDirection);
									}
									else
									{
										if ( BeCanPassBall() == false || iHeroWithBall.pos.x <= iHeroWithoutBall.pos.x )
										{
											Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
										}
										else
										{
											Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHeroWithBall));
										}
									}
								}
								else
								{
									if ( RealDistance( iHeroWithBall,iHeroWithoutBall ) >= 150 
										|| (iHeroWithoutBall.pos.x <= iForbiddenArea.left.right - 15 
										&& iHeroWithoutBall.pos.x > iForbiddenArea.left.right && iHeroWithBall.pos.x > iHeroWithoutBall.pos.x) )
									{
										TC_Position posTarget;
										if ( iHeroWithoutBall.pos.x - 90 <= iForbiddenArea.left.right)
										{
											posTarget.x = iForbiddenArea.left.right + 1;
										}
										else
										{
											posTarget.x = iHeroWithoutBall.pos.x - 90;
										}
										posTarget.y = iHeroWithoutBall.pos.y;

										if( Tc_PassBall(hHeroWithBall,posTarget) )
										{
											bPassedByOwnHero = true;
										}
										else
										{
											bPassedByOwnHero = false;
										}
										if ( iHeroWithoutBall.pos.x == posTarget.x )
										{
											Tc_Stop(hHeroWithoutBall);
										}
										else
										{
											Tc_Move(hHeroWithoutBall,iAttackDirection);
										}
									}
									else
									{
										if ( iEnemy_1.b_snatch_ghostball == true || RealDistance(iEnemy_1,iGhostBall) < 64
											|| iEnemy_2.b_snatch_ghostball == true || RealDistance(iEnemy_2,iGhostBall) < 64 
											||(iEnemy_1.pos.x > iHeroWithBall.pos.x && iEnemy_1.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
											||(iEnemy_2.pos.x > iHeroWithBall.pos.x && iEnemy_2.steps_before_next_snatch<8 && RealDistance(iHeroWithBall,iEnemy_1)<=150)
											)
										{
											Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
										}
										else
										{
											Kvc_Move(hHeroWithBall,iHeroWithBall,GetAttackDirection(&iHeroWithBall));
										}
										if ( iHeroWithoutBall.pos.x <= iForbiddenArea.left.right + 15)
										{
											if ( iHeroWithoutBall.pos.x == iForbiddenArea.left.right + 15 )
											{
												Tc_Stop(hHeroWithoutBall);
											}
											else
											{
												Tc_Move(hHeroWithoutBall,iDefenceDirection);
											}
										}
										else
										{
											Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));	
										}

									}
								}
							}
						}

// 对方禁区
						else
						{
							if ( iHeroWithoutBall.pos.x  == iForbiddenArea.left.right + 10 )
							{
								Tc_Stop(hHeroWithoutBall);
							}
							else
							{
								if (iHeroWithoutBall.pos.x < iForbiddenArea.left.right + 10)
								{
									Tc_Move(hHeroWithoutBall,iDefenceDirection);
								}
								else
								{
									Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
								}
							}
							if ( (iEnemy_1.b_is_spelling && (iEnemy_1.type == TC_HERO_GINNY || iEnemy_1.type == TC_HERO_RON || iEnemy_1.type == TC_HERO_MALFOY)
								&& iHeroWithBall.pos.x + TC_HERO_WIDTH > iEnemyGate.x + 22 )
								|| (iEnemy_2.b_is_spelling && (iEnemy_2.type == TC_HERO_GINNY || iEnemy_2.type == TC_HERO_RON || iEnemy_2.type == TC_HERO_MALFOY)
								&& iHeroWithBall.pos.x + TC_HERO_WIDTH > iEnemyGate.x + 22 )
								)
							{
								if ( BeCanPassBall() == true )
								{
									if ( Tc_PassBall(hHeroWithBall,iHeroWithoutBall.pos) )
									{
										Tc_Stop(hHeroWithoutBall);
										bPassedByOwnHero = true;
									}
									else
									{
										Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
										Tc_Stop(hHeroWithoutBall);
									}
								}
								else
								{
									TC_Position posTarget;
									posTarget.x = iHeroWithoutBall.pos.x;
									if ( iEnemyGate.y_upper - iHeroWithoutBall.pos.y >= iHeroWithoutBall.pos.y + TC_HERO_HEIGHT - iEnemyGate.y_lower)
									{
										posTarget.y = iHeroWithoutBall.pos.y + TC_HERO_HEIGHT + 50;
									}
									else
									{
										posTarget.y = iHeroWithoutBall.pos.y - 50;
									}
									if (Tc_PassBall(hHeroWithBall,posTarget))
									{

										Tc_Stop(hHeroWithoutBall);
									}
									else
									{
										Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
										Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
									}
								}
							}
							else
							{
								Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
								Tc_Stop(hHeroWithoutBall);
							}
						}
					}
				}
			}
			
			
			break;
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
		{
				if (iEnemyWithBall.pos.x <= iForbiddenArea.left.right || iEnemyWithBall.pos.x >= iForbiddenArea.right.left)
				{
					if (iEnemyWithBall.abnormal_type == TC_SPELLED_BY_FREEBALL || iEnemyWithBall.abnormal_type == TC_SPELLED_BY_MALFOY)
					{
						if (iHero_1.b_snatch_ghostball == true)
						{
							Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
							Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithoutBall));
						}
						else
						{
							if (iHero_2.b_snatch_ghostball == true)
							{
								Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
								Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithoutBall));
							}
							else
							{
								Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
								Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
							}
						}
					}
					else
					{
						if (iHero_1.b_is_spelling == true || iHero_2.b_is_spelling == true)
						{
							TC_Hero VisualEnemy;
							VisualEnemy.pos.x = (iEnemy_1.pos.x + iEnemy_2.pos.x)/2;
							VisualEnemy.pos.y = (iEnemy_1.pos.y + iEnemy_2.pos.y)/2;
							if (iHero_1.b_is_spelling == true)
							{
								Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&VisualEnemy));
							}
							else
							{
								Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&VisualEnemy));
							}
						}
						else
						{
							TC_Hero VisualEnemy;
							if (RealDistance(iEnemy_1,iEnemy_2) <= 300)
							{
								VisualEnemy.pos.x = (iEnemy_1.pos.x + iEnemy_2.pos.x)/2;
								VisualEnemy.pos.y = (iEnemy_1.pos.y + iEnemy_2.pos.y)/2;
							}
							else
							{
								VisualEnemy = iEnemyWithBall;
							}
							if (iHero_1.b_snatch_ghostball == true || iHero_2.b_snatch_ghostball == true)
							{
								
								if (iHero_2.b_can_spell && Tc_CanBeSpelled(hHero_2,hEnemyWithBall)
										&& ( iHero_2.curr_blue - iHero_2.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHero_2.spell_cost))
								{
									Tc_Spell(hHero_2,hEnemyWithBall);
									Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&VisualEnemy));
								}
								else
								{
									if (iHero_1.b_snatch_ghostball == true && iHero_2.b_snatch_ghostball == true)
									{
										Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
										Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&VisualEnemy));
									}
									else
									{
										if (iHero_1.b_snatch_ghostball)
										{
											Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
											Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&VisualEnemy));
										}
										else
										{
											Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
											Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&VisualEnemy));
										}
									}
								}
							}
							else 
							{
								if (iHero_1.b_can_spell && Tc_CanBeSpelled(hHero_1,hEnemyWithBall)
									&& ( iHero_1.curr_blue - iHero_1.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHero_1.spell_cost))
								{
									Tc_Spell(hHero_1,hEnemyWithBall);
								}
								else
								{
									Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
								}
								Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&VisualEnemy));
							}
						}
					}
				}
				else
				{
					if (iHero_1.b_snatch_ghostball == true || iHero_2.b_snatch_ghostball == true)
					{
						if (iHero_1.b_snatch_ghostball == true)
						{
							Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
							if ( RealDistance(iHero_2,iGhostBall) <= TC_SNATCH_DISTANCE_GHOST)
							{
								Tc_Stop( hHero_2 );
							}
							else
							{
								Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
							}
							
						}
						else
						{
							Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
							if ( RealDistance(iHero_2,iGhostBall) <= TC_SNATCH_DISTANCE_GHOST)
							{
								Tc_Stop( hHero_1 );
							}
							else
							{
								Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
							}
							
						}
					}
					else
					{
						if ((iHero_1.b_can_spell && Tc_CanBeSpelled(hHero_1,hEnemyWithBall)
							&&(iHero_1.curr_blue - iHero_1.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHero_1.spell_cost))
							)
						{
							if ( Tc_Spell(hHero_1,hEnemyWithBall) == false)
							{
								Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
							}
							Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
						}
						else
						{
							if ((iHero_2.b_can_spell && Tc_CanBeSpelled(hHero_2,hEnemyWithBall)
								&&(iHero_2.curr_blue - iHero_2.spell_cost + (iGameInfo.game_time_left-10000)*6/100 >= iHero_2.spell_cost))
								)
							{
								if ( Tc_Spell(hHero_2,hEnemyWithBall) == false)
								{
									Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
								}
								Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
							}
							else
							{
								Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
								Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
							}

						}
					}
				}
			
			break;
		}
	}

	if ( iGoldBall.b_visible == true )
	{
		if ( iHero_1.b_goldball || iHero_2.b_goldball )
		{
			Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
			Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
		}
		else
		{
			TC_Hero iEnemyForGoldBall,iHeroDefence;
			TC_Handle hEnemyForGoldBall,hHeroDefence;

			if ( RealDistance(iEnemy_1,iGoldBall) <= RealDistance(iEnemy_2,iGoldBall) )
			{
				iEnemyForGoldBall = iEnemy_1;
				hEnemyForGoldBall = hEnemy_1;
			}
			else
			{
				iEnemyForGoldBall = iEnemy_2;
				hEnemyForGoldBall = hEnemy_2;
			}

			if ( RealDistance(iEnemyForGoldBall,iGoldBall) <= 250 )
			{
				switch (iState)
				{
				case GHOSTBALL_IN_NO_TEAM:
					{
						if ( iHero_1.curr_blue >= iHero_1.spell_cost - 150 )
						{
							iHeroDefence = iHero_1;
							hHeroDefence = hHero_1;
						}
						else
						{
							if ( iHero_2.curr_blue >= iHero_2.spell_cost - 150 )
							{
								iHeroDefence = iHero_2;
								hHeroDefence = hHero_2;
							}
							else
							{
								if ( iHero_1.spell_cost - iHero_1.curr_blue <= iHero_2.spell_cost - iHero_2.curr_blue  )
								{
									iHeroDefence = iHero_1;
									hHeroDefence = hHero_1;
								}
								else
								{
									iHeroDefence = iHero_2;
									hHeroDefence = hHero_2;
								}
							}
						}

						if ( RealDistance(iEnemyForGoldBall,iGoldBall) <= 90 && iGoldBall.u.slow_down_steps_left > 3)
						{
							Tc_Spell(hHeroDefence,hEnemyForGoldBall);
						}
						else
						{
							Tc_Move(hHeroDefence,GetEnemyDirection(&iHeroDefence,&iEnemyForGoldBall));
						}
						break;

					}
				case GHOSTBALL_IN_ENEMY_TEAM:
					{
						if ( iHero_1.curr_blue >= iHero_1.spell_cost - 150 )
						{
							iHeroDefence = iHero_1;
							hHeroDefence = hHero_1;
						}
						else
						{
							if ( iHero_2.curr_blue >= iHero_2.spell_cost - 150 )
							{
								iHeroDefence = iHero_2;
								hHeroDefence = hHero_2;
							}
							else
							{
								if ( iHero_1.spell_cost - iHero_1.curr_blue <= iHero_2.spell_cost - iHero_2.curr_blue  )
								{
									iHeroDefence = iHero_1;
									hHeroDefence = hHero_1;
								}
								else
								{
									iHeroDefence = iHero_2;
									hHeroDefence = hHero_2;
								}
							}
						}

						if ( RealDistance(iEnemyForGoldBall,iGoldBall) <= 90 && iGoldBall.u.slow_down_steps_left > 3)
						{
							Tc_Spell(hHeroDefence,hEnemyForGoldBall);
						}
						else
						{
							Tc_Move(hHeroDefence,GetEnemyDirection(&iHeroDefence,&iEnemyForGoldBall));
						}
						break;
					}
				case GHOSTBALL_IN_SELF_TEAM:
					{
						iHeroDefence = iHeroWithoutBall;
						hHeroDefence = hHeroWithoutBall;
						if ( RealDistance(iEnemyForGoldBall,iGoldBall) <= 90 && iGoldBall.u.slow_down_steps_left > 3)
						{
							Tc_Spell(hHeroDefence,hEnemyForGoldBall);
						}
						else
						{
							Tc_Move(hHeroDefence,GetEnemyDirection(&iHeroDefence,&iEnemyForGoldBall));
						}
						break;
					}
				}
			}
		}
	}
}