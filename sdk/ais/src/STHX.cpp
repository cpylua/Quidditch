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
	//
	// TODO: Set team name(no Chinese) and choose heros here
	//
	Tc_SetTeamName(_T("STHX"));
	Tc_ChooseHero(TC_MEMBER1, TC_HERO_MALFOY, TEXT("marfu"));
	Tc_ChooseHero(TC_MEMBER2, TC_HERO_GINNY, TEXT("ginny"));
}

//
// TODO: Rename your DLL's filename with your team name
//

#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
TC_Handle hHero_1, hHero_2;
TC_Handle hEnemy_1, hEnemy_2;
TC_Handle hHeroWithBall, hHeroWithoutBall;
TC_Handle hEnemyWithBall, hEnemyWithoutBall;
TC_Handle hGhostBall;
TC_Ball iGhostBall;
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2, iHeroWithBall, iHeroWithoutBall, iEnemyWithBall, iEnemyWithoutBall;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_Direction iAttackDirection;
TC_GameInfo iGameInfo;

int GetBallDistance(TC_Hero hero,TC_Ball ball)
{
	return int(sqrt(double( (hero.pos.x-ball.pos.x)*(hero.pos.x-ball.pos.x) + (hero.pos.y-ball.pos.y)*(hero.pos.y-ball.pos.y) )));
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
			hHeroWithoutBall = hHero_2;
		}
		return GHOSTBALL_IN_SELF_TEAM;
	}
	else
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
	return TC_DIRECTION_RIGHT;
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
	return TC_DIRECTION_RIGHT;
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
	hGhostBall = Tc_GetBallHandle(TC_GHOST_BALL);

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
	switch(state)
	{
	case GHOSTBALL_IN_SELF_TEAM:
		{
			TC_Position iPos;

			if (iAttackDirection==TC_DIRECTION_LEFT)
			{
				if (iHeroWithBall.pos.x<=1024)   //禁区-100后有人接近放魔
				{
					///////////////////
					if( GetBallDistance(iEnemy_1,iGhostBall)<=400)
					{
						if (Tc_CanBeSpelled(hHero_1,hEnemy_1))
						{
							Tc_Spell(hHero_1,hEnemy_1);
						}

					}
					if( GetBallDistance(iEnemy_2,iGhostBall)<=400)
					{
						if (Tc_CanBeSpelled(hHero_1,hEnemy_2))
						{
							Tc_Spell(hHero_1,hEnemy_1);
						}

					}
				}
				if (iHeroWithBall.pos.x<(iForbiddenArea.right.left-100)&&(iHeroWithBall.pos.x>(iForbiddenArea.left.right+100)))
				{
					iPos.x = iForbiddenArea.left.right+1;
					iPos.y = (iEnemyGate.y_upper+iEnemyGate.y_lower)/2;
					if ((iEnemy_1.pos.x<(iForbiddenArea.right.left+100))&(iEnemy_1.pos.x>(iForbiddenArea.right.left-100)))
					{
						if (iEnemy_1.pos.y<iPos.y)
						{
							iPos.y=iPos.y+100;
							Tc_PassBall(hHeroWithBall,iPos);

						}
						else
						{
							iPos.y=iPos.y-100;
							Tc_PassBall(hHeroWithBall,iPos);

						}
						break;
					}
					if ((iEnemy_2.pos.x<(iForbiddenArea.right.left+100))&(iEnemy_2.pos.x>(iForbiddenArea.right.left-100)))
					{
						if (iEnemy_2.pos.y<iPos.y)
						{
							iPos.y=iPos.y+100;
							Tc_PassBall(hHeroWithBall,iPos);
						}
						else
						{
							iPos.y=iPos.y-100;
							Tc_PassBall(hHeroWithBall,iPos);
						}
						break;
					}
					Tc_PassBall(hHeroWithBall,iPos);
				}
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			}
			else                         //向右进攻
			{
				if (iHeroWithBall.pos.x>=1024)    //禁区+100后有人接近放魔
				{
					///////////////////
					if( GetBallDistance(iEnemy_1,iGhostBall)<=400)
					{
						if (Tc_CanBeSpelled(hHero_1,hEnemy_1))
						{
							Tc_Spell(hHero_1,hEnemy_1);
						}

					}
					if( GetBallDistance(iEnemy_2,iGhostBall)<=400)
					{
						if (Tc_CanBeSpelled(hHero_1,hEnemy_2))
						{
							Tc_Spell(hHero_1,hEnemy_2);
						}

					}
				}
	
				if (iHeroWithBall.pos.x>(iForbiddenArea.left.right+100)&&iHeroWithBall.pos.x<(iForbiddenArea.right.left-100))
				{
					iPos.x = iForbiddenArea.right.left-1;
					iPos.y = (iEnemyGate.y_upper+iEnemyGate.y_lower)/2;
					if ((iEnemy_1.pos.x<(iForbiddenArea.right.left+100))&(iEnemy_1.pos.x>(iForbiddenArea.right.left-100)))
					{
						if (iEnemy_1.pos.y<iPos.y)
						{
							iPos.y=iPos.y+100;
							Tc_PassBall(hHeroWithBall,iPos);

						}
						else
						{
							iPos.y=iPos.y-100;
							Tc_PassBall(hHeroWithBall,iPos);

						}
						break;
					}
					if ((iEnemy_2.pos.x<(iForbiddenArea.right.left+100))&(iEnemy_2.pos.x>(iForbiddenArea.right.left-100)))
					{
						if (iEnemy_2.pos.y<iPos.y)
						{
							iPos.y=iPos.y+100;
							Tc_PassBall(hHeroWithBall,iPos);
						}
						else
						{
							iPos.y=iPos.y-100;
							Tc_PassBall(hHeroWithBall,iPos);
						}
						break;
					}
					Tc_PassBall(hHeroWithBall,iPos);
				}
				Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			}

			break;
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
		{
			if (iAttackDirection == TC_DIRECTION_RIGHT)
			{
				if (iEnemyWithBall.pos.x - iOwnGate.x <= 200)
				{
					if (Tc_Spell(hHero_2,hEnemyWithBall))
					{
						break;
					}
					else
					{
						Tc_Spell(hHero_1,hEnemyWithBall);
					}
					if(iHero_1.pos.x!=iGhostBall.pos.x || iHero_1.pos.y!=iGhostBall.pos.y)
						Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					if(iHero_2.pos.x!=iGhostBall.pos.x || iHero_2.pos.y!=iGhostBall.pos.y)
						Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
					break;
				}
				if (iEnemyWithBall.pos.x < iForbiddenArea.left.right && iEnemyWithBall.pos.x-iOwnGate.x > 200)
				{
					if (iHero_1.b_snatch_ghostball == true||iHero_2.b_snatch_ghostball == true)
					{
						if (Tc_SnatchBall(hHero_1,TC_GHOST_BALL) == true)
						{
							break;
						}
						else
						{
							if (Tc_SnatchBall(hHero_2,TC_GHOST_BALL) == true)//1眩晕，2吹飞
							{
								break;
							}
							Tc_Spell(hHero_1,hEnemyWithBall);
						}

					}
					else
					{
						Tc_Spell(hHero_1,hEnemyWithBall);
					}
				}
			}
			else                                                            //向左进攻
			{
				if (iOwnGate.x - iEnemyWithBall.pos.x <= 200)
				{
					if (Tc_Spell(hHero_2,hEnemyWithBall))//1眩晕，2吹飞
					{
						break;
					}
					else
					{
						Tc_Spell(hHero_1,hEnemyWithBall);
					}
					if(iHero_1.pos.x!=iGhostBall.pos.x || iHero_1.pos.y!=iGhostBall.pos.y)
						Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					if(iHero_2.pos.x!=iGhostBall.pos.x || iHero_2.pos.y!=iGhostBall.pos.y)
						Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
					break;
				}
				if (iEnemyWithBall.pos.x > iForbiddenArea.right.left && iOwnGate.x-iEnemyWithBall.pos.x > 200)
				{
					if (iHero_1.b_snatch_ghostball == true||iHero_2.b_snatch_ghostball==true)
					{
						if (Tc_SnatchBall(hHero_1,TC_GHOST_BALL) == true)
						{
							break;
						}
						else
						{
							if (Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true)
							{
								break;
							}
							Tc_Spell(hHero_1,hEnemyWithBall);
						}

					}
					else
					{
						Tc_Spell(hHero_1,hEnemyWithBall);
					}
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
			if(iHero_1.pos.x!=iGhostBall.pos.x || iHero_1.pos.y!=iGhostBall.pos.y)
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			Tc_Move(hHero_2,GetBallDirection(&iHero_1,&iGhostBall));
			break;
		}
	case GHOSTBALL_IN_NO_TEAM:
		{
			if (iGameInfo.game_time_left>=89000)
			{
				if ((sqrt(double(iEnemy_1.pos.x-iGhostBall.pos.x))+sqrt(double(iEnemy_1.pos.y-iGhostBall.pos.y)))<=(sqrt(double(iEnemy_2.pos.x-iGhostBall.pos.x))+sqrt(double(iEnemy_2.pos.y-iGhostBall.pos.y))))
				{
					Tc_Spell(hHero_2,hEnemy_1);
				} 
				else
				{
					Tc_Spell(hHero_2,hEnemy_2);
				}
			}
			if (iHero_1.b_snatch_ghostball == false) 
			{
				if(iHero_1.pos.x!=iGhostBall.pos.x || iHero_1.pos.y!=iGhostBall.pos.y)
					Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			}
			else
			{
				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
			}
			if (iHero_2.b_ghostball == false)
			{
				if(iHero_2.pos.x!=iGhostBall.pos.x || iHero_2.pos.y!=iGhostBall.pos.y)
					Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			}
			else 
			{
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
			}
			break;
		}
	default:
		{
			break;
		}
	}
}