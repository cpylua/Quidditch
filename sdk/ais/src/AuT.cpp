//NOTICE ：球在英雄手中，被任意球砸晕后，那个英雄返回的是手中无球的状态
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
	Tc_SetTeamName(L"AuT");
	
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"h1");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_RON,L"h2");
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
TC_Position middle2,middle4;
double t_hHero_1,t_hEnemy_1,t_hHero_2,t_hEnemy_2,GOLD_BALL_hHero_1,iHero_1_speed,GOLD_BALL_hHero_2,iHero_2_speed,GOLD_BALL_hEnemy_1,iEnemy_1_speed,GOLD_BALL_hEnemy_2,iEnemy_2_speed;

TC_Direction GetCeLue(TC_Hero *pHero, TC_Position pMiddle)////////////////////////////////
{
	if (pHero->pos.x > pMiddle.x )return TC_DIRECTION_LEFT;
	if (pHero->pos.x < pMiddle.x)return TC_DIRECTION_RIGHT;
}
int GetState()   
{
	
	//	if(iGoldBall.b_visible)    ////////////////
	//	{
	//		
	//		return GOLDBALL_REACHABLE;
	//	}
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
	if (iEnemy_1.b_snatch_ghostball == true || iEnemy_2.b_snatch_ghostball == true)
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
	if (pHero->pos.x > pEnemy->pos.x && abs(pHero->pos.y - pEnemy->pos.y)<5)return TC_DIRECTION_LEFT;
	if (abs(pHero->pos.x - pEnemy->pos.x)<5 && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x < pEnemy->pos.x && abs(pHero->pos.y - pEnemy->pos.y)<5)return TC_DIRECTION_RIGHT;
	if (abs(pHero->pos.x - pEnemy->pos.x)<5 && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_TOP;
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_LEFTBOTTOM;
}
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)
{
	if (abs(pHero->pos.x - pBall->pos.x)<5 && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pBall->pos.x && abs(pHero->pos.y - pBall->pos.y)<5)return TC_DIRECTION_LEFT;
	if (pHero->pos.x < pBall->pos.x && abs(pHero->pos.y - pBall->pos.y)<5)return TC_DIRECTION_RIGHT;
	if (abs(pHero->pos.x - pBall->pos.x)<5 && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_TOP;
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_LEFTBOTTOM;
}

TC_Direction GetAttackDirection(TC_Hero *pHero)
{
	int temp;
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		temp=129;
	}
	else 
	{
		temp=-1;
	}
	if(pHero->pos.y>=iEnemyGate.y_lower-128)
	{
		if(pHero->pos.x==iEnemyGate.x)return TC_DIRECTION_TOP;
		if(iAttackDirection==TC_DIRECTION_RIGHT)return TC_DIRECTION_RIGHTTOP;
		return TC_DIRECTION_LEFTTOP;
	}
	if(pHero->pos.y<iEnemyGate.y_upper+1)
	{
		if(pHero->pos.x==iEnemyGate.x)return TC_DIRECTION_BOTTOM;
		if(iAttackDirection==TC_DIRECTION_RIGHT)return TC_DIRECTION_RIGHTBOTTOM;
		return TC_DIRECTION_LEFTBOTTOM;
	}
	return iAttackDirection;
}
double getenemytoball(TC_Hero *pHero)
{
	return (pHero->pos.x-iGhostBall.pos.x)*(pHero->pos.x-iGhostBall.pos.x)+(pHero->pos.y-iGhostBall.pos.y)*(pHero->pos.y-iGhostBall.pos.y);
}
bool Duoqiu (TC_Hero *pHero)
{
	double x1,y1,x2,y2;
	int i;
	x1=pHero->pos.x;
	y1=pHero->pos.y;
	x2=iFreeBall.pos.x;
	y2=iFreeBall.pos.y;
	for(i=1;i<=20;i++)
	{
		if(((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))<18432)return 1;
		x1+=pHero->speed.vx;
		y1+=pHero->speed.vy;
		x2+=iFreeBall.speed.vx;
		y2+=iFreeBall.speed.vy;
	}
	return 0;//装不到
}

int dianzhi(TC_Ball a,int x, int y){ //a为球 x,y是点  u.path.pos_end.x
	return (a.u.path.pos_start.x-a.u.path.pos_end.x)*(y-a.u.path.pos_end.y)-(a.u.path.pos_start.y-a.u.path.pos_end.y)*(x-a.u.path.pos_end.x);
}
int xielu(TC_Ball a){
	return -1*(a.u.path.pos_start.x-a.u.path.pos_end.x)*(a.u.path.pos_start.y-a.u.path.pos_end.y);
}

void Move (TC_Hero *pHero,TC_Direction aaa,TC_Handle hHeroa)
{
	int i;
	TC_Hero ppHero;
	TC_Ball iiFreeBall;
	ppHero.pos.x=pHero->pos.x+64;
	ppHero.pos.y=pHero->pos.y+64;
	iiFreeBall.pos.x=iFreeBall.pos.x+32;
	iiFreeBall.pos.y=iFreeBall.pos.y+32;
	iiFreeBall.u.path.pos_start.x=iFreeBall.u.path.pos_start.x+32;
	iiFreeBall.u.path.pos_start.y=iFreeBall.u.path.pos_start.y+32;
	iiFreeBall.u.path.pos_end.x=iFreeBall.u.path.pos_end.x+32;
	iiFreeBall.u.path.pos_end.y=iFreeBall.u.path.pos_end.y+32;
	bool flag=0;
	double a,b,c;
	a=iiFreeBall.speed.vx*iiFreeBall.speed.vx+iiFreeBall.speed.vy*iiFreeBall.speed.vx;
	b=2*((ppHero.pos.x-iiFreeBall.pos.x)*iiFreeBall.speed.vx+(ppHero.pos.y-iiFreeBall.pos.y)*iiFreeBall.speed.vy);
	c=(ppHero.pos.x-iiFreeBall.pos.x)*(ppHero.pos.x-iiFreeBall.pos.x)+(ppHero.pos.y-iiFreeBall.pos.y)*(ppHero.pos.x-iiFreeBall.pos.x);
	flag=Duoqiu(pHero);
	if(flag==0||iFreeBall.b_visible==0)Tc_Move(hHeroa,aaa);
	else                          //求下游走球现在和未来于英雄现在位置的最近距离
	{
		
		if((b/(2*a)>iFreeBall.u.moving_time_left&&((ppHero.pos.x-iiFreeBall.u.path.pos_end.x)*(ppHero.pos.x-iiFreeBall.u.path.pos_end.x)+(ppHero.pos.y-iiFreeBall.u.path.pos_end.y)*(ppHero.pos.y-iiFreeBall.u.path.pos_end.y))>18432)||(b/(2*a)<=iFreeBall.u.moving_time_left&&(4*a*c-b*b)/(4*a)>18432))
		{
			Tc_Stop(hHeroa);
		}//停下来
		else
		{
			if(iAttackDirection==TC_DIRECTION_RIGHT)
			{
				if(dianzhi(iiFreeBall,ppHero.pos.x, ppHero.pos.y)<25&&dianzhi(iiFreeBall,ppHero.pos.x, ppHero.pos.y)>-25)
				{   
					int ly=(iiFreeBall.u.path.pos_start.y>iiFreeBall.u.path.pos_end.y)?(iiFreeBall.u.path.pos_start.y-iiFreeBall.u.path.pos_end.y):(iiFreeBall.u.path.pos_end.y>iiFreeBall.u.path.pos_start.y);
					int lx=(iiFreeBall.u.path.pos_start.x>iiFreeBall.u.path.pos_end.x)?(iiFreeBall.u.path.pos_start.x-iiFreeBall.u.path.pos_end.x):(iiFreeBall.u.path.pos_end.x>iiFreeBall.u.path.pos_start.x);
					if(ly>lx)
						Tc_Move(hHeroa,TC_DIRECTION_RIGHT);
					else
					{
						Tc_Move(hHeroa,TC_DIRECTION_TOP);
					}
				}
				else 
					if(dianzhi(iiFreeBall,ppHero.pos.x, ppHero.pos.y)<=-25) // 直线上面
					{
						if(xielu(iiFreeBall)<0) Tc_Move(hHeroa,TC_DIRECTION_RIGHT);
						else Tc_Move(hHeroa,TC_DIRECTION_TOP); // 向上
					}
					else//直线下面
					{
						if(xielu(iiFreeBall)<0) Tc_Move(hHeroa,TC_DIRECTION_BOTTOM);// 向下
						else Tc_Move(hHeroa,TC_DIRECTION_RIGHT);
					}
					
			}
			else//向左攻击
			{
				if(dianzhi(iiFreeBall,ppHero.pos.x, ppHero.pos.y)<25&&dianzhi(iiFreeBall,ppHero.pos.x, ppHero.pos.y)>-25)
				{
					int ly=(iiFreeBall.u.path.pos_start.y>iiFreeBall.u.path.pos_end.y)?(iiFreeBall.u.path.pos_start.y-iiFreeBall.u.path.pos_end.y):(iiFreeBall.u.path.pos_end.y>iiFreeBall.u.path.pos_start.y);
					int lx=(iiFreeBall.u.path.pos_start.x>iiFreeBall.u.path.pos_end.x)?(iiFreeBall.u.path.pos_start.x-iiFreeBall.u.path.pos_end.x):(iiFreeBall.u.path.pos_end.x>iiFreeBall.u.path.pos_start.x);
					if(ly>lx)
						Tc_Move(hHeroa,TC_DIRECTION_LEFT);
					else
					{
						Tc_Move(hHeroa,TC_DIRECTION_TOP);
					}
				}
				else 
					if(dianzhi(iiFreeBall,ppHero.pos.x, ppHero.pos.y)<-25) // 直线上面
					{
						if(xielu(iiFreeBall)<0) Tc_Move(hHeroa,TC_DIRECTION_TOP);
						else Tc_Move(hHeroa,TC_DIRECTION_LEFT); 
					}
					else//直线下面
					{
						if(xielu(iiFreeBall)<0) Tc_Move(hHeroa,TC_DIRECTION_LEFT);// 向下
						else Tc_Move(hHeroa,TC_DIRECTION_BOTTOM);
					}
					
					
			}
			
			/*	if(iiFreeBall.u.path.pos_start.x==iiFreeBall.u.path.pos_end.x)
			{
			if(iAttackDirection==TC_DIRECTION_RIGHT)
			{
			if(pHero.pos.x>iiFreeBall.u.path.pos_start.x)Tc_Move(hHeroa,TC_DIRECTION_RIGHT);
			else Tc_Stop(hHeroa);
			}
			else
			{
			if(pHero.pos.x<iiFreeBall.u.path.pos_start.x)Tc_Move(hHeroa,TC_DIRECTION_LEFT);
			else Tc_Stop(hHeroa);
			}
			}
			else 
			{
			if(iAttackDirection==TC_DIRECTION_RIGHT)
			{
			
			  }
		}*/
		}
	}
	
}

CLIENT_EXPORT_API void __stdcall AI()
{
	bool Flag1=0,Flag2=0;
	middle2.x=(iOwnGate.x+iEnemyGate.x)/2;/////////////////////////////////
	middle2.y=(iOwnGate.y_upper+iOwnGate.y_lower)/2;/////////////////////////////////
	middle4.x=(iOwnGate.x+middle2.x)/2;/////////////////////////////////
	middle4.y=middle2.y;/////////////////////////////////
	
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
	double e1,e2,h1,h2; //e1：enemy1到球的距离。e2：enemy2到球的距离。h1：hero1到球的距离。h2：hero2到球的距离
	e1=(iGhostBall.pos.x-iEnemy_1.pos.x)*(iGhostBall.pos.x-iEnemy_1.pos.x)+(iGhostBall.pos.y-iEnemy_1.pos.y)*(iGhostBall.pos.y-iEnemy_1.pos.y);
	e2=(iGhostBall.pos.x-iEnemy_2.pos.x)*(iGhostBall.pos.x-iEnemy_2.pos.x)+(iGhostBall.pos.y-iEnemy_2.pos.y)*(iGhostBall.pos.y-iEnemy_2.pos.y);
	h1=(iGhostBall.pos.x-iHero_1.pos.x)*(iGhostBall.pos.x-iHero_1.pos.x)+(iGhostBall.pos.y-iHero_1.pos.y)*(iGhostBall.pos.y-iHero_1.pos.y);
	h2=(iGhostBall.pos.x-iHero_2.pos.x)*(iGhostBall.pos.x-iHero_2.pos.x)+(iGhostBall.pos.y-iHero_2.pos.y)*(iGhostBall.pos.y-iHero_2.pos.y);		
	switch (state)
	{
	case GHOSTBALL_IN_NO_TEAM:
		{
			if(iGoldBall.b_visible)
			{
				if(iHero_1.b_snatch_goldball)Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
				if(iHero_2.b_snatch_goldball)Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
			}
			if(iAttackDirection==TC_DIRECTION_RIGHT)    //不能用abs，只能分开写  
			{
				if(iHero_1.b_snatch_ghostball&&Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
				{
					Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
					Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
					break;
				}
				else Move(&iHero_1,GetBallDirection(&iHero_1, &iGhostBall),hHero_1);
				if(iHero_2.b_snatch_ghostball&&Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
				{
					Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
					Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
					break;
				}
				else Move(&iHero_2,GetBallDirection(&iHero_2, &iGhostBall),hHero_2);
				if(iGhostBall.pos.x<iForbiddenArea.left.right+100)
				{
					if((h1<h2?h1:h2)<(e1<e2?e1:e2))  //有英雄比对方距球近
					{
						Move(&iHero_1,GetBallDirection(&iHero_1, &iGhostBall),hHero_1);
						Move(&iHero_2,GetBallDirection(&iHero_2, &iGhostBall),hHero_2);
					}
					if((h1<h2?h1:h2)>=(e1<e2?e1:e2))
					{
						if(e1<e2&&e2<(h1>h2?h1:h2))
						{
							if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2, hEnemy_1)){if(Tc_Spell(hHero_2, hEnemy_1))break;}     //fangmo
							if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, hEnemy_1)){if(Tc_Spell(hHero_1, hEnemy_1))break;}
							
						}
						if(e1>=e2&&e1<(h1>h2?h1:h2))
						{
							if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2, hEnemy_2)){if(Tc_Spell(hHero_2, hEnemy_2))break;}    //fangmo
							if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, hEnemy_2)){if(Tc_Spell(hHero_1, hEnemy_2))break;}
						}
					}
				}
			}
			if(iAttackDirection==TC_DIRECTION_LEFT)
			{
				if(iHero_1.b_snatch_ghostball&&Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
				{
					Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
					Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
					break;
				}
				else Move(&iHero_1,GetBallDirection(&iHero_1, &iGhostBall),hHero_1);
				if(iHero_2.b_snatch_ghostball&&Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
				{
					Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
					Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
					break;
				}
				else Move(&iHero_2,GetBallDirection(&iHero_2, &iGhostBall),hHero_2);
				if(iGhostBall.pos.x>iForbiddenArea.right.left-100)
				{
					if((h1<h2?h1:h2)<(e1<e2?e1:e2))  //有英雄比对方距球近
					{
						Move(&iHero_1,GetBallDirection(&iHero_1, &iGhostBall),hHero_1);
						Move(&iHero_2,GetBallDirection(&iHero_2, &iGhostBall),hHero_2);
					}
					if((h1<h2?h1:h2)>(e1<e2?e1:e2))
					{
						if(e1<e2&&e2<(h1>h2?h1:h2))
						{
							if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2, hEnemy_1)){if(Tc_Spell(hHero_2, hEnemy_1))break;}  //fangmo
							if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, hEnemy_1)){if(Tc_Spell(hHero_1, hEnemy_1))break;}
						}
						if(e1>=e2&&e1<(h1>h2?h1:h2))
						{
							if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2, hEnemy_2)){if(Tc_Spell(hHero_2, hEnemy_2))break;}
							if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, hEnemy_2)){if(Tc_Spell(hHero_1, hEnemy_2))break;}
						}
					}
				}
				
			}
			break;
		}
	case GHOSTBALL_IN_SELF_TEAM:
		{
			if(iGoldBall.b_visible)
			{
				if(iHero_1.b_snatch_goldball)Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
				if(iHero_2.b_snatch_goldball)Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
			}
			TC_Position passball;
			TC_Hero iHeroPosition;
			if(iAttackDirection==TC_DIRECTION_RIGHT)       //传球时可以优化下，不只是向前传，如果前面有对手还可以向队友传
			{		
				if(iEnemyGate.x-iGhostBall.pos.x<500)
				{
					if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, &iEnemy_1))
					{
							Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
					}
					if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2, &iEnemy_2))
					{
							Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					}
				}
				if(iHero_1.b_ghostball)        //球在hero1手中
				{
					if((iHero_1.speed.vx==0&&iHero_1.speed.vy==0)||iHero_1.abnormal_type==TC_SPELLED_BY_MALFOY||iHero_1.abnormal_type==TC_SPELLED_BY_FREEBALL||iHero_1.abnormal_type==TC_SPELLED_BY_HERMIONE)
					{
						if(iHero_2.b_snatch_ghostball){if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))break;}
					}
					passball.y=iHero_1.pos.y;
					if(iHero_1.pos.x+TC_PASSBALL_DISTANCE<iForbiddenArea.right.left-64)passball.x=iHero_1.pos.x+TC_PASSBALL_DISTANCE;
					else 
					{
						passball.x=iForbiddenArea.right.left-64;
						passball.y=(iEnemyGate.y_upper+iEnemyGate.y_lower)/2;
					}
					if(iGhostBall.pos.x<iForbiddenArea.right.left-200){if(Tc_PassBall(hHero_1, passball))break;}
					Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
					//			system("pause");
					iHeroPosition.pos.x=iHero_1.pos.x-30;
					iHeroPosition.pos.y=iHero_1.pos.y;
					Move(&iHero_2,GetEnemyDirection(&iHero_2,&iHeroPosition),hHero_2);         //两个英雄和在一起走，可以破解吹球和吹人
					if(iGhostBall.pos.x>iForbiddenArea.right.left-200)
					{
						if((iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_GINNY)&&iEnemy_1.b_is_spelling)
						{
							if(Tc_CanBeSpelled(hHero_1, hEnemy_1))
							{
								if(Tc_Spell(hHero_1, hEnemy_1))
								{
									Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
									break;
								}
							}
							if(iEnemy_1.type==TC_HERO_RON&&h2<300)
							{
								passball.y=iHero_2.pos.y;
								passball.x=iForbiddenArea.right.left-70;
							}
							else
							{
								passball.y=iHero_1.pos.y;
								passball.x=iForbiddenArea.right.left-70;
							}
							if(Tc_PassBall(hHero_1, passball))
							{
								if(iHero_2.b_snatch_ghostball)
								{
									if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
									{
										Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
										Move(&iHero_1,GetEnemyDirection(&iHero_1,&iHero_2),hHero_1);
										break;
									}
								}
								Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
								Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
							}
						}
						if((iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_GINNY)&&iEnemy_2.b_is_spelling)
						{
							if(Tc_CanBeSpelled(hHero_1, hEnemy_2))
							{
								if(Tc_Spell(hHero_1, hEnemy_2))
								{
									Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
									break;
								}
							}
							if(iEnemy_2.type==TC_HERO_RON&&h2<300)
							{
								passball.y=iHero_2.pos.y;
								passball.x=iForbiddenArea.right.left-70;
							}
							else
							{
								passball.y=iHero_1.pos.y;
								passball.x=iForbiddenArea.right.left-70;
							}
							if(Tc_PassBall(hHero_1, passball))
							{
								if(iHero_2.b_snatch_ghostball)
								{
									if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
									{
										Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
										Move(&iHero_1,GetEnemyDirection(&iHero_1,&iHero_2),hHero_1);
										break;
									}
								}
								Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
								Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
							}
						}
					}
				}
				if(iHero_2.b_ghostball)       //球在hero2手中
				{
					if((iHero_2.speed.vx==0&&iHero_2.speed.vy==0)||iHero_2.abnormal_type==TC_SPELLED_BY_MALFOY||iHero_2.abnormal_type==TC_SPELLED_BY_FREEBALL||iHero_2.abnormal_type==TC_SPELLED_BY_HERMIONE)
					{
						if(iHero_1.b_snatch_ghostball){if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))break;}
					}
					passball.y=iHero_2.pos.y;
					if(iHero_2.pos.x+TC_PASSBALL_DISTANCE-64<iForbiddenArea.right.left-10)passball.x=iHero_2.pos.x+TC_PASSBALL_DISTANCE;
					else 
					{
						passball.x=iForbiddenArea.right.left-64;
						passball.y=(iEnemyGate.y_upper+iEnemyGate.y_lower)/2;
					}
					if(iGhostBall.pos.x<iForbiddenArea.right.left-200)
					{
						if(Tc_PassBall(hHero_2, passball))
						{
							Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
							Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
							break;
						}
					}
					Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
					iHeroPosition.pos.x=iHero_2.pos.x-30;
					iHeroPosition.pos.y=iHero_2.pos.y;
					Move(&iHero_1,GetEnemyDirection(&iHero_1,&iHeroPosition),hHero_1); 
					if(iGhostBall.pos.x>iForbiddenArea.right.left-100)
					{
						if((iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_GINNY)&&iEnemy_1.b_is_spelling)
						{
							if(Tc_CanBeSpelled(hHero_1, hEnemy_1)){if(Tc_Spell(hHero_1, hEnemy_1))break;}
							if(iEnemy_1.type==TC_HERO_RON&&h1<300)
							{
								passball.y=iHero_1.pos.y;
								passball.x=iForbiddenArea.right.left-70;
							}
							else
							{
								passball.y=iHero_2.pos.y;
								passball.x=iForbiddenArea.right.left-70;
							}
							if(Tc_PassBall(hHero_2, passball))
							{
								if(iHero_1.b_snatch_ghostball)
								{
									if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
									{
										Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
										Move(&iHero_2,GetEnemyDirection(&iHero_2,&iHero_1),hHero_2);
										break;
									}
								}
								Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
								Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
							}
						}
						if((iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_GINNY)&&iEnemy_2.b_is_spelling)
						{
							if(Tc_CanBeSpelled(hHero_1, hEnemy_2)){if(Tc_Spell(hHero_1, hEnemy_2))break;}
							if(iEnemy_2.type==TC_HERO_RON&&h1<300)
							{
								passball.y=iHero_1.pos.y;
								passball.x=iForbiddenArea.right.left-70;
							}
							else
							{
								passball.y=iHero_2.pos.y;
								passball.x=iForbiddenArea.right.left-70;
							}
							if(Tc_PassBall(hHero_2, passball))
							{
								if(iHero_1.b_snatch_ghostball)
								{
									if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
									{
										Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
										Move(&iHero_2,GetEnemyDirection(&iHero_2,&iHero_1),hHero_2);
										break;
									}
								}
								Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
								Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
							}
						}
					}
				}
				
			}
			//出现一个bug，球被吹回中线后，两个英雄不去抢
			if(iAttackDirection==TC_DIRECTION_LEFT)
			{
				if(iGhostBall.pos.x-iEnemyGate.x<500)
				{
					if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, &iEnemy_1))
					{
							Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
					}
					if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2, &iEnemy_2))
					{
							Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					}
				}
				if(iHero_1.b_ghostball)        //球在hero1手中
				{
					if((iHero_1.speed.vx==0&&iHero_1.speed.vy==0)||iHero_1.abnormal_type==TC_SPELLED_BY_MALFOY||iHero_1.abnormal_type==TC_SPELLED_BY_FREEBALL||iHero_1.abnormal_type==TC_SPELLED_BY_HERMIONE)
					{
						if(iHero_2.b_snatch_ghostball){if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))break;}
					}
					passball.y=iHero_1.pos.y;
					if(iHero_1.pos.x-(TC_PASSBALL_DISTANCE-64)>iForbiddenArea.left.right)passball.x=iHero_1.pos.x-(TC_PASSBALL_DISTANCE-64);
					else 
					{
						passball.x=iForbiddenArea.left.right;
						passball.y=(iEnemyGate.y_upper+iEnemyGate.y_lower)/2;
					}
					if(iGhostBall.pos.x-iForbiddenArea.left.right>100)if(Tc_PassBall(hHero_1, passball))break;
					Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
					//			system("pause");
					iHeroPosition.pos.y=iHero_1.pos.y;
					iHeroPosition.pos.x=iHero_1.pos.x+30;
					Move(&iHero_2,GetEnemyDirection(&iHero_2,&iHeroPosition),hHero_2);         //两个英雄和在一起走，可以破解吹球和吹人
					if(iGhostBall.pos.x<iForbiddenArea.left.right+200)
					{
						if((iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_GINNY)&&iEnemy_1.b_is_spelling)
						{
							if(Tc_CanBeSpelled(hHero_1, hEnemy_1))
							{
								if(Tc_Spell(hHero_1, hEnemy_1))
								{
									Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
									break;
								}
							}
							if(iEnemy_1.type==TC_HERO_RON&&h2<300)
							{
								passball.y=iHero_2.pos.y;
								passball.x=iForbiddenArea.left.right+70;
							}
							else
							{
								passball.y=iHero_1.pos.y;
								passball.x=iForbiddenArea.left.right+70;
							}
							if(Tc_PassBall(hHero_1, passball))
							{
								if(iHero_2.b_snatch_ghostball)
								{
									if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
									{
										Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
										Move(&iHero_1,GetEnemyDirection(&iHero_1,&iHero_2),hHero_1);
										break;
									}
								}
								Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
								Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
							}
						}
						if((iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_GINNY)&&iEnemy_2.b_is_spelling)
						{
							if(Tc_CanBeSpelled(hHero_1, hEnemy_2))
							{
								if(Tc_Spell(hHero_1, hEnemy_2))
								{
									Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
									break;
								}
								
							}
							if(iEnemy_2.type==TC_HERO_RON&&h2<300)
							{
								passball.y=iHero_2.pos.y;
								passball.x=iForbiddenArea.left.right+70;
							}
							else
							{
								passball.y=iHero_1.pos.y;
								passball.x=iForbiddenArea.left.right+70;
							}
							if(Tc_PassBall(hHero_1, passball))
							{
								if(iHero_2.b_snatch_ghostball)
								{
									if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
									{
										Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
										Move(&iHero_1,GetEnemyDirection(&iHero_1,&iHero_2),hHero_1);
										break;
									}
								}
								Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
								Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
							}
						}
					}
				}
				if(iHero_2.b_ghostball)       //球在hero2手中
				{
						if((iHero_2.speed.vx==0&&iHero_2.speed.vy==0)||iHero_2.abnormal_type==TC_SPELLED_BY_MALFOY||iHero_2.abnormal_type==TC_SPELLED_BY_FREEBALL||iHero_2.abnormal_type==TC_SPELLED_BY_HERMIONE)
						{
							if(iHero_1.b_snatch_ghostball){if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))break;}
						}
						passball.y=iHero_2.pos.y;
						if(iHero_2.pos.x-(TC_PASSBALL_DISTANCE-64)>iForbiddenArea.left.right+10)passball.x=iHero_2.pos.x-(TC_PASSBALL_DISTANCE-64);
						else 
						{
							passball.x=iForbiddenArea.left.right;
							passball.y=(iEnemyGate.y_upper+iEnemyGate.y_lower)/2;
						}					
						if(iGhostBall.pos.x-iForbiddenArea.left.right>100){if(Tc_PassBall(hHero_2, passball))break;}
						Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
						iHeroPosition.pos.y=iHero_2.pos.y;
						iHeroPosition.pos.x=iHero_2.pos.x+30;
						Move(&iHero_1,GetEnemyDirection(&iHero_1,&iHeroPosition),hHero_1); 
						if(iGhostBall.pos.x<iForbiddenArea.left.right+100)
						{
							if((iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_GINNY)&&iEnemy_1.b_is_spelling)
							{
								if(Tc_CanBeSpelled(hHero_1, hEnemy_1)){if(Tc_Spell(hHero_1, hEnemy_1))break;}
								if(iEnemy_1.type==TC_HERO_RON&&h1<300)
								{
									passball.y=iHero_1.pos.y;
									passball.x=iForbiddenArea.left.right+70;
								}
								else
								{
									passball.y=iHero_2.pos.y;
									passball.x=iForbiddenArea.left.right+70;
								}
								if(Tc_PassBall(hHero_2, passball))
								{
								if(iHero_1.b_snatch_ghostball)
								{
									if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
									{
										Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
										Move(&iHero_2,GetEnemyDirection(&iHero_2,&iHero_1),hHero_2);
										break;
									}
								}
								Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
								Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
								}
							}
							if((iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_GINNY)&&iEnemy_2.b_is_spelling)
							{
								if(Tc_CanBeSpelled(hHero_1, hEnemy_2)){if(Tc_Spell(hHero_1, hEnemy_2))break;}
								if(iEnemy_2.type==TC_HERO_RON&&h1<300)
								{
									passball.y=iHero_1.pos.y;
									passball.x=iForbiddenArea.left.right+70;
								}
								else
								{
									passball.y=iHero_2.pos.y;
									passball.x=iForbiddenArea.left.right+70;
								}
								if(Tc_PassBall(hHero_2, passball))
								{
								if(iHero_1.b_snatch_ghostball)
								{
									if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
									{
										Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
										Move(&iHero_2,GetEnemyDirection(&iHero_2,&iHero_1),hHero_2);
										break;
									}
								}
								Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
								Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
								}
							}
						}
					}
				}
				break;
			}
		case GHOSTBALL_IN_ENEMY_TEAM:
			{
				if(iGoldBall.b_visible)
				{
				if(iHero_1.b_snatch_goldball)Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
				if(iHero_2.b_snatch_goldball)Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
				}
				if(iAttackDirection==TC_DIRECTION_RIGHT)
				{
					if(iGhostBall.pos.x>iForbiddenArea.right.left-100)
					{
						if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, hEnemyWithBall))
						{
							Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						}
					}
					if(iGhostBall.pos.x<iForbiddenArea.left.right+150)   //球到了我们球门前的传球禁区了
					{
						if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2, hEnemyWithBall)&&iHero_1.b_is_spelling==0)
						{
							if(Tc_Spell(hHero_2,hEnemyWithBall))
							{
							//	Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
							//	Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
								Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								break;
							}
						}
						if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, hEnemyWithBall)&&iHero_2.b_is_spelling==0)
						{
							if(Tc_Spell(hHero_1,hEnemyWithBall))
							{
							//	Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
							//	Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
								Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								break;
							}
						}
					}
					if(iHero_1.b_snatch_ghostball&&Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
					{
				//		Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
				//		Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}
				//	else Move(&iHero_1,GetBallDirection(&iHero_1, &iGhostBall),hHero_1);
					else Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					if(iHero_2.b_snatch_ghostball&&Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					{
				//		Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
				//		Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}
				//	else Move(&iHero_2,GetBallDirection(&iHero_2, &iGhostBall),hHero_2);
					else Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				}
				
				if(iAttackDirection==TC_DIRECTION_LEFT)
				{
					if(iGhostBall.pos.x>iForbiddenArea.left.right+100)
					{
						if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, hEnemyWithBall))
						{
							Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						}
					}
					if(iGhostBall.pos.x>iForbiddenArea.right.left-150)   //球到了我们球门前的传球禁区了
					{
						if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2, hEnemyWithBall)&&iHero_1.b_is_spelling==0)
						{
							if(Tc_Spell(hHero_2,hEnemyWithBall))
							{
						//		Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
						//		Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
								Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								break;
							}
						}
						if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1, hEnemyWithBall)&&iHero_2.b_is_spelling==0)
						{
							if(Tc_Spell(hHero_1,hEnemyWithBall))
							{
						//		Move(&iHero_1,GetBallDirection(&iHero_1,&iGhostBall),hHero_1);
						//		Move(&iHero_2,GetBallDirection(&iHero_2,&iGhostBall),hHero_2);
								Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								break;
							}
						}
					}
					if(iHero_1.b_snatch_ghostball&&Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
					{
				//		Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
				//		Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}
				//	else Move(&iHero_1,GetBallDirection(&iHero_1, &iGhostBall),hHero_1);
					else Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					if(iHero_2.b_snatch_ghostball&&Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					{
				//		Move(&iHero_1,GetAttackDirection(&iHero_1),hHero_1);
				//		Move(&iHero_2,GetAttackDirection(&iHero_2),hHero_2);
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}
				//	else Move(&iHero_2,GetBallDirection(&iHero_2, &iGhostBall),hHero_2);		
					else Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				}
				break;
			}
	}
}
//有bug。在禁区时候好像乱传球，还有下边界时不走直线。。。没有躲球