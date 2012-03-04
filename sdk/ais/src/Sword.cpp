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
	Tc_SetTeamName(_T("Sword"));
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_GINNY,L"XXXX");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_MALFOY,L"LLLLL");
}

#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
#define GOLDBALL_REACHABLE_BY_HERO1 3
#define GOLDBALL_REACHABLE_BY_HERO2 4
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
	if (iGoldBall.b_visible)
	{
		if (iHero_1.b_snatch_goldball)
		{
			return GOLDBALL_REACHABLE_BY_HERO1;
		}
		if (iHero_2.b_snatch_goldball)
		{
			return GOLDBALL_REACHABLE_BY_HERO2;
		}
		/*此处待改进,人物与金色飞贼的距离判断
		if (sqrt(double((iHero_1.pos.x+128/2-iGoldBall.pos.x+64/2)*(iHero_1.pos.x+128/2-iGoldBall.pos.x+64/2)+(iHero_1.pos.y+128/2-iGoldBall.pos.y+64/2)*(iHero_1.pos.y+128/2-iGoldBall.pos.y+64/2)))<=50)
				{
					return GOLDBALL_REACHABLE_BY_HERO1;
				}
				if (sqrt(double((iHero_2.pos.x+128/2-iGoldBall.pos.x+64/2)*(iHero_2.pos.x+128/2-iGoldBall.pos.x+64/2)+(iHero_2.pos.y+128/2-iGoldBall.pos.y+64/2)*(iHero_2.pos.y+128/2-iGoldBall.pos.y+64/2)))<=50)
				{
					return GOLDBALL_REACHABLE_BY_HERO2;
				}
				double tx,ty,tmax;
				if(abs(iHero_1.pos.x+128/2-iGoldBall.pos.x+64/2)>50)
					tx=abs(iHero_1.pos.x+128/2-iGoldBall.pos.x+64/2-50)/iHero_1.speed.vx;
				else 
					tx=0;
				if(abs(iHero_1.pos.y+128/2-iGoldBall.pos.y+64/2)>50)
					ty=abs(iHero_1.pos.y+128/2-iGoldBall.pos.y+64/2-50)/iHero_1.speed.vy;
				else 
					ty=0;
				tmax=tx>ty?tx:ty;
				if (tmax<iGoldBall.visible_time_left)
					return GOLDBALL_REACHABLE_BY_HERO1;
		///////////////
				if(abs(iHero_2.pos.x+128/2-iGoldBall.pos.x+64/2)>50)
					tx=abs(iHero_2.pos.x+128/2-iGoldBall.pos.x+64/2-50)/iHero_2.speed.vx;
				else 
					tx=0;
				if(abs(iHero_2.pos.y+128/2-iGoldBall.pos.y+64/2)>50)
					ty=abs(iHero_2.pos.y+128/2-iGoldBall.pos.y+64/2-50)/iHero_2.speed.vy;
				else 
					ty=0;
				tmax=tx>ty?tx:ty;
				if (tmax<iGoldBall.visible_time_left)
					return GOLDBALL_REACHABLE_BY_HERO2;*/		
	}
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
int mindistance=10;
TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pEnemy)
{
	int x1=pEnemy->pos.x+64;
	int y1=pEnemy->pos.y+64;
	int x2=pHero->pos.x+64;
	int y2=pHero->pos.y+64;
	if (x2 <= x1+mindistance && x2 >= x1-mindistance && y2 > y1)return TC_DIRECTION_TOP;
	if (x2 < x1 && y2 <= y1+mindistance && y2 >= y1-mindistance)return TC_DIRECTION_RIGHT;
	if (x2 <= x1+mindistance && x2 >= x1-mindistance && y2 < y1)return TC_DIRECTION_BOTTOM;
	if (x2 > x1 && y2 <= y1+mindistance && y2 >= y1-mindistance)return TC_DIRECTION_LEFT;
	if (x2 > x1 && y2 > y1)return TC_DIRECTION_LEFTTOP;
	if (x2 < x1 && y2 > y1)return TC_DIRECTION_RIGHTTOP;
	if (x2 < x1 && y2 < y1)return TC_DIRECTION_RIGHTBOTTOM;
	if (x2 > x1 && y2 < y1)return TC_DIRECTION_LEFTBOTTOM;
}
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)
{
	int x1=pBall->pos.x+32;
	int y1=pBall->pos.y+32;
	int x2=pHero->pos.x+64;
	int y2=pHero->pos.y+64;
	if (x2 <= x1+mindistance && x2 >= x1-mindistance && y2 > y1)return TC_DIRECTION_TOP;
	if (x2 < x1 && y2 <= y1+mindistance && y2 >= y1-mindistance)return TC_DIRECTION_RIGHT;
	if (x2 <= x1+mindistance && x2 >= x1-mindistance && y2 < y1)return TC_DIRECTION_BOTTOM;
	if (x2 > x1 && y2 <= y1+mindistance && y2 >= y1-mindistance)return TC_DIRECTION_LEFT;
	if (x2 > x1 && y2 > y1)return TC_DIRECTION_LEFTTOP;
	if (x2 < x1 && y2 > y1)return TC_DIRECTION_RIGHTTOP;
	if (x2 < x1 && y2 < y1)return TC_DIRECTION_RIGHTBOTTOM;
	if (x2 > x1 && y2 < y1)return TC_DIRECTION_LEFTBOTTOM;
}

TC_Direction GetAttackDirection(TC_Hero *pHero)
{
	int x1=pHero->pos.x+64;
	int y1=pHero->pos.y+64;
	if (y1 >= iEnemyGate.y_lower-64)
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
		if (y1 <= iEnemyGate.y_upper+64)
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

TC_Direction GetGuardDirection(TC_Hero *pHero,int x,int y)
{
	int x1=pHero->pos.x+64;
	int y1=pHero->pos.y+64;
	if (x1<x+mindistance && x1>x-mindistance && y1>y)
	{
		return TC_DIRECTION_TOP;
	}
	if (x1<x+mindistance && x1>x-mindistance && y1<y)
	{
		return TC_DIRECTION_BOTTOM;
	}
	if (x1>x && y1<y+mindistance && y1>y-mindistance)
	{
		return TC_DIRECTION_LEFT;
	}
	if (x1<x && y1<y+mindistance && y1>y-mindistance)
	{
		return TC_DIRECTION_RIGHT;
	}
	if (x1<x&&y1>y)
	{
		return TC_DIRECTION_RIGHTTOP;
	}
	if (x1<x&&y1<y)
	{
		return TC_DIRECTION_RIGHTBOTTOM;
	}
	if (x1>x&&y1<y)
	{
		return TC_DIRECTION_LEFTBOTTOM;
	}
	if (x1>x&&y1>y)
	{
		return TC_DIRECTION_LEFTTOP;
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


	TC_Position forbidpoint;
	
	if(iAttackDirection == TC_DIRECTION_RIGHT)
	{
		forbidpoint.x=iForbiddenArea.right.left-66;
	}
	else
	{
		forbidpoint.x=iForbiddenArea.left.right+3;
	}
	
	int mid=(iOwnGate.x+iEnemyGate.x)/2;

	switch (state)
	{
		case GOLDBALL_REACHABLE_BY_HERO1:
		{

			Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
			break;
		}
		case GOLDBALL_REACHABLE_BY_HERO2:
		{
			Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
			break;
		}
		case GHOSTBALL_IN_SELF_TEAM:
		{
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall, & iGhostBall));
			if((iAttackDirection == TC_DIRECTION_RIGHT&&
				iGhostBall.pos.x+32>=iForbiddenArea.right.left-100)||
				(iAttackDirection == TC_DIRECTION_LEFT&&
				iGhostBall.pos.x+32<=iForbiddenArea.left.right+100))
			{
				Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
				Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall, & iGhostBall));
				if(hHeroWithBall==hHero_2&&
					((iAttackDirection == TC_DIRECTION_RIGHT
					&&iHero_1.pos.x+64>mid&&(iHero_1.pos.x<iHero_2.pos.x
					||iHero_1.pos.x<iEnemy_1.pos.x||iHero_1.pos.x<iEnemy_2.pos.x))
					||(iAttackDirection == TC_DIRECTION_LEFT
					&&iHero_1.pos.x+64<mid&&(iHero_1.pos.x>iHero_2.pos.x
					||iHero_1.pos.x>iEnemy_1.pos.x||iHero_1.pos.x>iEnemy_2.pos.x))))
				{
					Tc_Stop(hHero_1);
				}
			}
			else
			{
				if(iGhostBall.pos.y+32>iEnemyGate.y_lower-32)
				{
					forbidpoint.y=iEnemyGate.y_lower-32;
				}
				else if(iGhostBall.pos.y+32<iEnemyGate.y_upper+32){
					forbidpoint.y=iEnemyGate.y_upper+32;
				}
				else {
					forbidpoint.y=iHeroWithBall.pos.y;
				}
				if(iAttackDirection == TC_DIRECTION_RIGHT){
					if(iEnemy_1.pos.x>iHeroWithBall.pos.x&&(iEnemy_1.pos.y>iHeroWithBall.pos.y-80&&iEnemy_1.pos.y<iHeroWithBall.pos.y+80)){
						break;
					}
					if(iEnemy_2.pos.x>iHeroWithBall.pos.x&&(iEnemy_2.pos.y>iHeroWithBall.pos.y-80&&iEnemy_2.pos.y<iHeroWithBall.pos.y+80)){
						break;
					}
				}
				if(iAttackDirection == TC_DIRECTION_LEFT){
					if(iEnemy_1.pos.x<iHeroWithBall.pos.x&&(iEnemy_1.pos.y>iHeroWithBall.pos.y-80&&iEnemy_1.pos.y<iHeroWithBall.pos.y+80)){
						break;
					}
					if(iEnemy_2.pos.x<iHeroWithBall.pos.x&&(iEnemy_2.pos.y>iHeroWithBall.pos.y-80&&iEnemy_2.pos.y<iHeroWithBall.pos.y+80)){
						break;
					}
				}
				Tc_PassBall(hHeroWithBall, forbidpoint);
			}
			break;
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
		{
			int x,y;	
			y=(iOwnGate.y_lower+iOwnGate.y_upper)/2;
			if(iHero_1.b_is_spelling)
			{
				x=mid;
				y=(iEnemyGate.y_lower+iEnemyGate.y_upper)/2;
				Tc_Move(hHero_2,GetGuardDirection(&iHero_2,x,y));
				break;
			}
			if(iOwnGate.x<iForbiddenArea.left.right){	
				x=iForbiddenArea.left.right;
			}
			else
			{
				x=iForbiddenArea.right.left;
			}		
				if((iAttackDirection == TC_DIRECTION_RIGHT&&(iEnemyWithBall.pos.x+64<iForbiddenArea.left.right+200))
					||(iAttackDirection == TC_DIRECTION_LEFT&&(iEnemyWithBall.pos.x+64>iForbiddenArea.right.left-200)))
				{
					if(iEnemyWithBall.abnormal_type!=TC_SPELLED_BY_NONE && iEnemyWithBall.abnormal_type!=TC_SPELLING)
					{
						goto cc;
					}
					if(iHero_1.b_is_spelling||iHero_2.b_is_spelling)
					{
						goto cc;
					}
					if(iHero_1.b_can_spell)
					{
						if(!Tc_Spell(hHero_1,hEnemyWithBall))
						{
							if(iHero_2.b_can_spell)
								Tc_Spell(hHero_2,hEnemyWithBall);
						}
					}
					else
					{
						if(iHero_2.b_can_spell)
							Tc_Spell(hHero_2,hEnemyWithBall);
					}
				}
cc:				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				if(iHero_1.b_snatch_ghostball)
				{
					Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				}
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				if(iHero_2.b_snatch_ghostball)
				{
					Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				}
			break;
		}
	case GHOSTBALL_IN_NO_TEAM:
		{
			Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			if (iHero_1.b_snatch_ghostball == false) 
			{
				if((iAttackDirection == TC_DIRECTION_RIGHT&&iHero_1.pos.x+64>mid+100&&(iHero_1.pos.x+64<iGhostBall.pos.x+32))
					||(iAttackDirection == TC_DIRECTION_LEFT&&iHero_1.pos.x+64<mid-100&&(iHero_1.pos.x+64>iGhostBall.pos.x+32)))
				{
					Tc_Stop(hHero_1);
				}
				else
				{
					Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				}
			}
			else
			{
				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==false)
				{
					Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				}
			}
			if (iHero_2.b_snatch_ghostball == false)
			{
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			}
			else 
			{
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==false)
				{
					Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				}
			}
			break;
		}
//	case GOLDBALL_REACHABLE:
//		{
//			break;
//		}
	default:
		{
			break;
		}
	}
}