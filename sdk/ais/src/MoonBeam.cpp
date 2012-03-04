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
	Tc_SetTeamName(L"Moonbeam");
	
//	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"Ï²ÑòÑò");
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_RON,L"Ï²ÑòÑò");
//	Tc_ChooseHero(TC_MEMBER2,TC_HERO_HERMIONE,L"»ÒÌ«ÀÇ");
//	Tc_ChooseHero(TC_MEMBER2,TC_HERO_MALFOY,L"»ÒÌ«ÀÇ");
	Tc_ChooseHero(TC_MEMBER2, TC_HERO_GINNY, L"»ÒÌ«ÀÇ");
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

TC_Handle hEnemy_closer, hEnemy_farther;
TC_Handle hHero_closer, hHero_farther;
TC_Hero iEnemy_closer, iEnemy_farther;
TC_Hero iHero_closer, iHero_farther;

TC_GameInfo gameInfo;
const double doubleINF=1.0e20;

int GetState()
{
//	if(iGoldBall.b_visible==true)
//		return GOLDBALL_REACHABLE;

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
	return 0;
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

TC_Direction GetHeroDirection(TC_Hero *pHeroFrom, TC_Hero *pHeroTo){
	if(pHeroFrom->pos.x<pHeroTo->pos.x && pHeroFrom->pos.y<pHeroTo->pos.y)return TC_DIRECTION_RIGHTBOTTOM;
	if(pHeroFrom->pos.x==pHeroTo->pos.x && pHeroFrom->pos.y<pHeroTo->pos.y)return TC_DIRECTION_BOTTOM;
	if(pHeroFrom->pos.x>pHeroTo->pos.x && pHeroFrom->pos.y<pHeroTo->pos.y)return TC_DIRECTION_LEFTBOTTOM;
	if(pHeroFrom->pos.x>pHeroTo->pos.x && pHeroFrom->pos.y==pHeroTo->pos.y)return TC_DIRECTION_LEFT;
	if(pHeroFrom->pos.x>pHeroTo->pos.x && pHeroFrom->pos.y>pHeroTo->pos.y)return TC_DIRECTION_LEFTTOP;
	if(pHeroFrom->pos.x==pHeroTo->pos.x && pHeroFrom->pos.y>pHeroTo->pos.y)return TC_DIRECTION_TOP;
	if(pHeroFrom->pos.x<pHeroTo->pos.x && pHeroFrom->pos.y>pHeroTo->pos.y)return TC_DIRECTION_RIGHTTOP;
	if(pHeroFrom->pos.x<pHeroTo->pos.x && pHeroFrom->pos.y==pHeroTo->pos.y)return TC_DIRECTION_RIGHT;
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

double GetEnemytoBallDist(TC_Hero *pHero)
{
	return (pHero->pos.x-iGhostBall.pos.x)*(pHero->pos.x-iGhostBall.pos.x)+(pHero->pos.y-iGhostBall.pos.y)*(pHero->pos.y-iGhostBall.pos.y);
}

bool IsInRightForbiddenArea(TC_Position *pPose)
{
	if(pPose->x>=iForbiddenArea.right.left && pPose->x<=iForbiddenArea.right.right && pPose->y>=iForbiddenArea.right.top && pPose->y<=iForbiddenArea.right.bottom)
		return 1;
	if(pPose->x>=iEnemyGate.x)
		return 1;
	return 0;
}

bool IsInLeftForbiddenArea(TC_Position *pPose)
{
	if(pPose->x<=iForbiddenArea.left.left && pPose->x>=iForbiddenArea.left.right && pPose->y>=iForbiddenArea.left.top && pPose->y<=iForbiddenArea.left.bottom)
		return 1;
	if(pPose->x<=iEnemyGate.x)
		return 1;
	return 0;
}

double timeToGold(TC_Hero *pHero){
	if(pHero->speed.vx || pHero->speed.vy){
		int disx=iGoldBall.pos.x-pHero->pos.x;
		int disy=iGoldBall.pos.y-pHero->pos.y;
		double dis=disx*1.0*disx+disy*1.0*disy;
		double v=pHero->speed.vx*1.0*pHero->speed.vx+pHero->speed.vy*1.0*pHero->speed.vy;
		double t=sqrt(dis/v);
		if( (int) t < gameInfo.game_time_left/1000*60)
			return t;
	}
	return doubleINF;
}

CLIENT_EXPORT_API void __stdcall AI()
{
	//
	// TODO: add your AI here
	//
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
	Tc_GetGameInfo(hHero_1, &gameInfo);

	if (iOwnGate.x < iEnemyGate.x)
	{
		iAttackDirection = TC_DIRECTION_RIGHT;
	}
	else
	{
		iAttackDirection = TC_DIRECTION_LEFT;
	}

	double dish1,dish2,dise1,dise2;

	int state = GetState();
	if(iGoldBall.b_visible){
		dish1=timeToGold(&iHero_1);
		dish2=timeToGold(&iHero_2);
		dise1=timeToGold(&iEnemy_1);
		dise2=timeToGold(&iEnemy_2);

		if(dish1<dish2){
			hHero_closer=hHero_1;
			hHero_farther=hHero_2;
			iHero_closer=iHero_1;
			iHero_farther=iHero_2;
		}
		else{
			hHero_closer=hHero_2;
			hHero_farther=hHero_1;
			iHero_closer=iHero_2;
			iHero_farther=iHero_1;
		}
		if(dise1<dise2){
			hEnemy_closer=hEnemy_1;
			hEnemy_farther=hEnemy_2;
			iEnemy_closer=iEnemy_1;
			iEnemy_farther=iEnemy_2;
		}
		else{
			hEnemy_closer=hEnemy_2;
			hEnemy_farther=hEnemy_1;
			iEnemy_closer=iEnemy_2;
			iEnemy_farther=iEnemy_1;
		}
		dise1=min(dise1, dise2);
		dish1=min(dish1, dish2);

		if(iHero_1.b_snatch_goldball==true)
		{
			Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
		}
		if(iHero_2.b_snatch_goldball==true)
		{
			Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
		}

		if(gameInfo.score_self-gameInfo.score_enemy>2||gameInfo.score_enemy-gameInfo.score_self>2)
		{
			
			if(dise1<dish1)
			{
				if(iHero_2.b_can_spell && Tc_CanBeSpelled(hHero_2, hEnemy_closer)){
					Tc_Spell(hHero_2,hEnemy_closer);
					if(iHero_2.b_ghostball)Tc_Move(hHero_2, GetAttackDirection(&iHero_2));
					else Tc_Move(hHero_2, GetBallDirection(&iHero_2, &iGoldBall));
				}
				else {
					if(iHero_1.b_ghostball)Tc_Move(hHero_1, GetAttackDirection(&iHero_1));
					else Tc_Move(hHero_1, GetBallDirection(&iHero_1, &iGoldBall));
					if(iHero_2.b_can_spell && Tc_CanBeSpelled(hHero_2, hEnemy_closer))Tc_Spell(hHero_2, hEnemy_closer);
					else Tc_Move(hHero_2, GetBallDirection(&iHero_2, &iGoldBall));
				}
			}
			if(dish1<doubleINF){
				if(Tc_SnatchBall(hHero_closer, TC_GOLD_BALL)==false){
					Tc_Move(hHero_closer, GetBallDirection(&iHero_closer, &iGoldBall));
					return;
				}
			}
		}
	
	}

	
	switch (state)
	{
	case GHOSTBALL_IN_SELF_TEAM:
		{

			TC_Position point1;
			if(iAttackDirection == TC_DIRECTION_RIGHT)
				point1.x=iHeroWithBall.pos.x+TC_PASSBALL_DISTANCE-100;
			else 
				point1.x=iHeroWithBall.pos.x-TC_PASSBALL_DISTANCE+100;
			point1.y=iHeroWithBall.pos.y;

			TC_Position point2;
			if(iAttackDirection == TC_DIRECTION_RIGHT)
				point2.x=iForbiddenArea.right.left;
			else 
				point2.x=iForbiddenArea.left.right;
			point2.y=iEnemyGate.y_lower-128;
			

			if(iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE)
			{
				if(iHeroWithoutBall.b_snatch_ghostball == true )
				{
					Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
					break;
					
				}
			}
	/*		if(iAttackDirection == TC_DIRECTION_RIGHT&&iHeroWithBall.pos.x>iForbiddenArea.right.left||iAttackDirection == TC_DIRECTION_LEFT&&iHeroWithBall.pos.x<iForbiddenArea.left.right)
			{
				if(GetEnemytoBallDist(&iEnemy_1)<GetEnemytoBallDist(&iEnemy_2))
					Tc_Spell(hHero_1,hEnemy_1);
				else
					Tc_Spell(hHero_1,hEnemy_2);
			}*/
//			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
//			Tc_Move(hHeroWithoutBall,GetHeroDirection(&iHeroWithoutBall,&iHeroWithBall));
			
			
			
			if((iAttackDirection == TC_DIRECTION_RIGHT&&iGhostBall.pos.x<iForbiddenArea.right.left-100)||(iAttackDirection==TC_DIRECTION_LEFT&&iGhostBall.pos.x>iForbiddenArea.left.right+100))
			{
				if((iAttackDirection == TC_DIRECTION_RIGHT&&point1.x<iForbiddenArea.right.left-100)||(iAttackDirection==TC_DIRECTION_LEFT&&point1.x>iForbiddenArea.left.right+100))
					Tc_PassBall(hHeroWithBall,point1);
				else
					Tc_PassBall(hHeroWithBall,point2);

			
			}
			else  
			{
				if(iEnemy_1.type==TC_HERO_GINNY)
				{
					if(iEnemy_1.b_is_spelling==true&&iHeroWithoutBall.b_snatch_ghostball == true)
					{
						Tc_Stop(hHeroWithBall);
						Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						break;
					}
					else
					{
						Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
						Tc_Move(hHeroWithoutBall,GetHeroDirection(&iHeroWithoutBall,&iHeroWithBall));
					}
				}
				if(iEnemy_2.type==TC_HERO_GINNY)
				{
					if(iEnemy_2.b_is_spelling==true&&iHeroWithoutBall.b_snatch_ghostball == true)
					{
						Tc_Stop(hHeroWithBall);
						Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						break;
					}
					else
					{
						Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
						Tc_Move(hHeroWithoutBall,GetHeroDirection(&iHeroWithoutBall,&iHeroWithBall));
					}
				}
				if(iEnemy_1.type==TC_HERO_RON)
				{
					if(iEnemy_1.b_is_spelling==true&&iHeroWithoutBall.b_snatch_ghostball == true)
					{
						Tc_Stop(hHeroWithBall);
						Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						break;
					}
					else
					{
						Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
						Tc_Move(hHeroWithoutBall,GetHeroDirection(&iHeroWithoutBall,&iHeroWithBall));
						break;
					}
				}
				if(iEnemy_2.type==TC_HERO_RON)
				{
					if(iEnemy_2.b_is_spelling==true&&iHeroWithoutBall.b_snatch_ghostball == true)
					{
						Tc_Stop(hHeroWithBall);
						Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						break;
					}
					else
					{
						Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
						Tc_Move(hHeroWithoutBall,GetHeroDirection(&iHeroWithoutBall,&iHeroWithBall));
						break;
					}
				}
				
			
				Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
				Tc_Move(hHeroWithoutBall,GetHeroDirection(&iHeroWithoutBall,&iHeroWithBall));
			}  
		
			break;
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
		{
			if (iAttackDirection == TC_DIRECTION_RIGHT)
			{
				if (iEnemyWithBall.pos.x<= iForbiddenArea.left.right && iHero_2.b_can_spell==true)
				{
					if(Tc_Spell(hHero_2,hEnemyWithBall))
					{
						Tc_Move(hHeroWithBall,iAttackDirection);
						Tc_Move(hHeroWithoutBall,iAttackDirection);
						break;
					}
				}
				if (iEnemyWithBall.pos.x - iOwnGate.x <= 100  && iHero_1.b_can_spell==true)
				{
					if(Tc_Spell(hHero_1,hEnemyWithBall))
					{
						Tc_Move(hHeroWithBall,iAttackDirection);
						Tc_Move(hHeroWithoutBall,iAttackDirection);
						break;
					}
				}
			}
			else
			{
				if (iEnemyWithBall.pos.x>= iForbiddenArea.right.left && iHero_2.b_can_spell==true)
				{
					if(Tc_Spell(hHero_2,hEnemyWithBall))
					{
						Tc_Move(hHeroWithBall,iAttackDirection);
						Tc_Move(hHeroWithoutBall,iAttackDirection);
						break;
					}
				}
				else if (iOwnGate.x - iEnemyWithBall.pos.x <= 100 && iHero_1.b_can_spell==true)
				{
					if(Tc_Spell(hHero_1,hEnemyWithBall))
					{
						Tc_Move(hHeroWithBall,iAttackDirection);
						Tc_Move(hHeroWithoutBall,iAttackDirection);
						break;
					}
				}
			}
			Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
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
			Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
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
			if (iHero_2.b_snatch_ghostball == false)
			{
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