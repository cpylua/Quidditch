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
	Tc_SetTeamName(L"VirusWish");
	Tc_ChooseHero(TC_MEMBER1, TC_HERO_RON, L"WENG");
	Tc_ChooseHero(TC_MEMBER2, TC_HERO_MALFOY, L"XIE");
}

//
// TODO: Rename your DLL's filename with your team name
//

#define DISTANCE 80
#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
#define GOLDBALL_REACHABLE 3
TC_Handle hHero_1, hHero_2;
TC_Handle hEnemy_1, hEnemy_2;
TC_Handle hHeroWithBall, hHeroWithoutBall;
TC_Handle hEnemyWithBall, hEnemyWithoutBall;
TC_Handle hFreeBall, hGhostBall, hGoldBall;
TC_Handle hEnemynear,hEnemyfar;
TC_Ball iFreeBall, iGhostBall, iGoldBall;
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2, iHeroWithBall, iHeroWithoutBall, iEnemyWithBall, iEnemyWithoutBall,iEnemynear,iEnemyfar,iHero;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_Direction iAttackDirection;
int  GinnySpell=0;
int  RonSpell=0;
int  MarfoySpell=0;
int  gHero1=0;
int  gHero2=0;
//获得状态
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


//获得进球方向
TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pEnemy)
{
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y+10)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x == pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y+10)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y+10)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pEnemy->pos.x && abs(pHero->pos.y - pEnemy->pos.y)<10) return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y-10)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x == pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y-10)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y-10)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x > pEnemy->pos.x && abs(pHero->pos.y - pEnemy->pos.y)<10) return TC_DIRECTION_LEFT;
}

//获得贵妃球方向
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)
{
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y > pBall->pos.y+10)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x == pBall->pos.x && pHero->pos.y > pBall->pos.y+10)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y > pBall->pos.y+10)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pBall->pos.x && abs(pHero->pos.y - pBall->pos.y)<10) return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y < pBall->pos.y-10)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x == pBall->pos.x && pHero->pos.y < pBall->pos.y-10)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y < pBall->pos.y-10)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x > pBall->pos.x && abs(pHero->pos.y - pBall->pos.y)<10) return TC_DIRECTION_LEFT;
}


//向左还是向右攻击
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

void AvoidFreeBall(TC_Hero *pHero,TC_Handle hHero)
{
	TC_Position hero,ball;
	hero.x=pHero->pos.x+64;
	hero.y=pHero->pos.y+64;
	ball.x=iFreeBall.pos.x+32;
	ball.y=iFreeBall.pos.y+32;
	int a=iFreeBall.u.path.pos_start.x;
	int b=iFreeBall.u.path.pos_start.y;
	int c=iFreeBall.u.path.pos_end.x;
	int d=iFreeBall.u.path.pos_end.y;
	double xielv1=(b-d)/(c-a);
	double xielv2=(d-b)/(c-a);
	
	int ff;    //ff=1,x正方向，ff=2第一象限方向，ff=3，y正方向，ff=4，第二象限方向。。。
	if(iFreeBall.u.path.pos_start.y==iFreeBall.u.path.pos_end.y)  //水平运动
	{
		if(iFreeBall.u.path.pos_start.x<iFreeBall.u.path.pos_end.x)ff=1;  //向着x正方向
		else ff=5;        //向着x负方向
	}
	if(iFreeBall.u.path.pos_end.x==iFreeBall.u.path.pos_start.x)  //垂直运动
	{
		if(iFreeBall.u.path.pos_start.y<iFreeBall.u.path.pos_end.y)ff=7;     //向下运动，y坐标从上到下
		else ff=3;           //向上运动 
	}
	if(iFreeBall.u.path.pos_start.x<iFreeBall.u.path.pos_end.x)
	{
		if(iFreeBall.u.path.pos_start.y<iFreeBall.u.path.pos_end.y)ff=8;    //方向向第四象限
		else ff=2;   //方向向第一象限
	}
	if(iFreeBall.u.path.pos_start.x>iFreeBall.u.path.pos_end.x)
	{
		if(iFreeBall.u.path.pos_start.y<iFreeBall.u.path.pos_end.y)ff=6;  //方向向第三象限方向
		else ff=4;   //方向向第二象限方向
	}
	
	switch(ff)
	{
	case 2:
	case 6:
		{
			if(xielv1>=1)
			{
				if(ff==2)
				{
					if(hero.y>=b+TC_COLLIDE_WITH_FREEBALL||hero.y<=d-TC_COLLIDE_WITH_FREEBALL)
					{
						Tc_Move(hHero,TC_DIRECTION_RIGHT);
					}
					else
					{
						if(hero.x<(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)
						{
							Tc_Move(hHero,TC_DIRECTION_RIGHT);
						}
						if((hero.x>=(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)&&hero.x<=((((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE+5))
						{
							if((ball.y>hero.y&&iFreeBall.speed.vy>0)||(ball.y<hero.y&&iFreeBall.speed.vy<0))
							{
								Tc_Move(hHero,TC_DIRECTION_RIGHT);
							}
							else
							{
								Tc_Stop(hHero);
							}
						}
						else
						{
							if(pHero->b_ghostball==true)
							{
								Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
							}
							else
							{
								Tc_Move(hHero,GetBallDirection(pHero,&iGhostBall));
							}
						}
					}
				}
				if(ff==6)
				{
					if(hero.y>=d+TC_COLLIDE_WITH_FREEBALL||hero.y<=b-TC_COLLIDE_WITH_FREEBALL)
					{
						Tc_Move(hHero,TC_DIRECTION_RIGHT);
					}
					else
					{
						if(hero.x<(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)
						{
							Tc_Move(hHero,TC_DIRECTION_RIGHT);
						}
						if((hero.x>=(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)&&hero.x<=((((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE+5))
						{
							if((ball.y>hero.y&&iFreeBall.speed.vy>0)||(ball.y<hero.y&&iFreeBall.speed.vy<0))
							{
								Tc_Move(hHero,TC_DIRECTION_RIGHT);
							}
							else
							{
								Tc_Stop(hHero);
							}
						}
						else
						{
							if(pHero->b_ghostball==true)
							{
								Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
							}
							else
							{
								Tc_Move(hHero,GetBallDirection(pHero,&iGhostBall));
							}
						}
					}
				}
			}
			if(xielv1<1)
			{
				if(ff==2)
				{
					if(hero.y>=b+TC_COLLIDE_WITH_FREEBALL||hero.y<=d-TC_COLLIDE_WITH_FREEBALL)
					{
						Tc_Move(hHero,TC_DIRECTION_RIGHT);
					}
					else
					{
						if(hero.x<(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)
						{
							if(hero.y>=(b+d)/2)
								Tc_Move(hHero,TC_DIRECTION_RIGHTTOP);
							else
								Tc_Move(hHero,TC_DIRECTION_RIGHTBOTTOM);
						}
						else
						{
							Tc_Move(hHero,TC_DIRECTION_RIGHTBOTTOM);
						}
					}
				}
				if(ff==6)
				{
					if(hero.y>=d+TC_COLLIDE_WITH_FREEBALL||hero.y<=b-TC_COLLIDE_WITH_FREEBALL)
					{
						Tc_Move(hHero,TC_DIRECTION_RIGHT);
					}
					else
					{
						if(hero.x<(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)
						{
							if(hero.y>=(b+d)/2)
								Tc_Move(hHero,TC_DIRECTION_RIGHTTOP);
							else
								Tc_Move(hHero,TC_DIRECTION_RIGHTBOTTOM);
						}
						else
						{
							Tc_Move(hHero,TC_DIRECTION_RIGHTBOTTOM);
						}
					}
				}
			}
			break;
	}	//结束	
	case 4:
	case 8:
		{
			if(xielv2>=1)
			{
				if(ff==4)
				{
					if(hero.y>=b+TC_COLLIDE_WITH_FREEBALL||hero.y<=d-TC_COLLIDE_WITH_FREEBALL)
					{
						Tc_Move(hHero,TC_DIRECTION_RIGHT);
					}
					else
					{
						if(hero.x<(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)
						{
							Tc_Move(hHero,TC_DIRECTION_RIGHT);
						}
						if((hero.x>=(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)&&hero.x<=((((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE+5))
						{
							if((ball.y>hero.y&&iFreeBall.speed.vy>0)||(ball.y<hero.y&&iFreeBall.speed.vy<0))
							{
								Tc_Move(hHero,TC_DIRECTION_RIGHT);
							}
							else
							{
								Tc_Stop(hHero);
							}
						}
						else
						{
							if(pHero->b_ghostball==true)
							{
								Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
							}
							else
							{
								Tc_Move(hHero,GetBallDirection(pHero,&iGhostBall));
							}
						}
					}
				}
				if(ff==8)
				{
					if(hero.y>=d+TC_COLLIDE_WITH_FREEBALL||hero.y<=b-TC_COLLIDE_WITH_FREEBALL)
					{
						Tc_Move(hHero,TC_DIRECTION_RIGHT);
					}
					else
					{
						if(hero.x<(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)
						{
							Tc_Move(hHero,TC_DIRECTION_RIGHT);
						}
						if((hero.x>=(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)&&hero.x<=((((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE+5))
						{
							if((ball.y>hero.y&&iFreeBall.speed.vy>0)||(ball.y<hero.y&&iFreeBall.speed.vy<0))
							{
								Tc_Move(hHero,TC_DIRECTION_RIGHT);
							}
							else
							{
								Tc_Stop(hHero);
							}
						}
						else
						{
							if(pHero->b_ghostball==true)
							{
								Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
							}
							else
							{
								Tc_Move(hHero,GetBallDirection(pHero,&iGhostBall));
							}
						}
					}
				}
			}
			if(xielv2<1)
			{
				if(ff==4)
				{
					if(hero.y>=b+TC_COLLIDE_WITH_FREEBALL||hero.y<=d-TC_COLLIDE_WITH_FREEBALL)
					{
						Tc_Move(hHero,TC_DIRECTION_RIGHT);
					}
					else
					{
						if(hero.x<(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)
						{
							if(hero.y>=(b+d)/2)
								Tc_Move(hHero,TC_DIRECTION_RIGHTBOTTOM);
							else
								Tc_Move(hHero,TC_DIRECTION_RIGHTTOP);
						}
						else
						{
							Tc_Move(hHero,TC_DIRECTION_RIGHTTOP);
						}
					}
				}
				if(ff==8)
				{
					if(hero.y>=d+TC_COLLIDE_WITH_FREEBALL||hero.y<=b-TC_COLLIDE_WITH_FREEBALL)
					{
						Tc_Move(hHero,TC_DIRECTION_RIGHT);
					}
					else
					{
						if(hero.x<(((a-c)*hero.y-a*d+b*c)/(b-d))-DISTANCE)
						{
							if(hero.y>=(b+d)/2)
								Tc_Move(hHero,TC_DIRECTION_RIGHTBOTTOM);
							else
								Tc_Move(hHero,TC_DIRECTION_RIGHTTOP);
						}
						else
						{
							Tc_Move(hHero,TC_DIRECTION_RIGHTTOP);
						}
					}
				}
			}
			break;
	}
	default:
		{
			break;
		}
	}
}

void hideginny()
{
	if(iEnemy_2.type==TC_HERO_GINNY)
	{
	  if(iEnemy_2.b_is_spelling==true&&GinnySpell==0)
	  {
	    GinnySpell=1;
		if(iHero_1.b_ghostball==true)
		{
		  gHero1=1;
         TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			   target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400;
		  }
		  else
		  {
			   target.x=iForbiddenArea.left.right+64;
		  }
		  target.y=iGhostBall.pos.y;
		  Tc_PassBall(hHero_1,target);
		}
		if(iHero_2.b_ghostball==true)
		{
			gHero2=1;
             TC_Position target;
			  if(iAttackDirection==TC_DIRECTION_RIGHT)
			  { 
				 target.x=iForbiddenArea.right.left-64;
			  }
			  else
			  {
				 target.x=iForbiddenArea.left.right+64;
			  }
			  target.y=iGhostBall.pos.y;
			  Tc_PassBall(hHero_2,target);
		}
	  }
	  if(iEnemy_2.b_is_spelling==false)
	  {
		  GinnySpell=0;
		  gHero2=0;
		  gHero1=0;
	  }
	}
	if(iEnemy_1.type==TC_HERO_GINNY)
	{
	  GinnySpell=1;
      if(iEnemy_1.b_is_spelling==true)
	  {
		if(iHero_1.b_ghostball==true)
		{
			gHero1=1;
         TC_Position target;
		   if(iAttackDirection==TC_DIRECTION_RIGHT)
			target.x=iGhostBall.pos.x-400;
		   else
			target.x=iGhostBall.pos.x+400;
			target.y=iGhostBall.pos.y;
			 Tc_PassBall(hHero_1,target);
		}
		if(iHero_2.b_ghostball==true)
		{
			gHero2=1;
          TC_Position target;
			  if(iAttackDirection==TC_DIRECTION_RIGHT)
			   target.x=iGhostBall.pos.x-400;
			  else
				target.x=iGhostBall.pos.x+400;
			  target.y=iGhostBall.pos.y;
			  Tc_PassBall(hHero_2,target);
		}

	  }
	  if(iEnemy_1.b_is_spelling==false)
	  {
		  GinnySpell=0;
		  gHero2=0;
		  gHero1=0;
	  }
	}
}

void hideron()
{
	if(iEnemy_2.type==TC_HERO_RON)
	{
	  if(iEnemy_2.b_is_spelling==true&&RonSpell==0)
	  {
	    RonSpell=1;
		if(iHero_1.b_ghostball==true)
		{
		  gHero1=1;
         TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			   target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400;
		  }
		  else
		  {
			   target.x=iForbiddenArea.left.right+64;
		  }
		  target.y=iGhostBall.pos.y;
		  Tc_PassBall(hHero_1,target);
		}
		if(iHero_2.b_ghostball==true)
		{
			gHero2=1;
             TC_Position target;
			  if(iAttackDirection==TC_DIRECTION_RIGHT)
			  { 
				 target.x=iForbiddenArea.right.left-64;
			  }
			  else
			  {
				 target.x=iForbiddenArea.left.right+64;
			  }
			  target.y=iGhostBall.pos.y;
			  Tc_PassBall(hHero_2,target);
		}
	  }
	  if(iEnemy_2.b_is_spelling==false)
	  {
		  RonSpell=0;
		  gHero2=0;
		  gHero1=0;
	  }
	}
	if(iEnemy_1.type==TC_HERO_RON)
	{
	  RonSpell=1;
      if(iEnemy_1.b_is_spelling==true)
	  {
		if(iHero_1.b_ghostball==true)
		{
			gHero1=1;
         TC_Position target;
		   if(iAttackDirection==TC_DIRECTION_RIGHT)
			target.x=iGhostBall.pos.x-400;
		   else
			target.x=iGhostBall.pos.x+400;
			target.y=iGhostBall.pos.y;
			 Tc_PassBall(hHero_1,target);
		}
		if(iHero_2.b_ghostball==true)
		{
			gHero2=1;
          TC_Position target;
			  if(iAttackDirection==TC_DIRECTION_RIGHT)
			   target.x=iGhostBall.pos.x-400;
			  else
				target.x=iGhostBall.pos.x+400;
			  target.y=iGhostBall.pos.y;
			  Tc_PassBall(hHero_2,target);
		}

	  }
	  if(iEnemy_1.b_is_spelling==false)
	  {
		  RonSpell=0;
		  gHero2=0;
		  gHero1=0;
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
	double instance1=sqrt((long double)((iHero_1.pos.x-iGhostBall.pos.x)*(iHero_1.pos.x-iGhostBall.pos.x)+(iHero_1.pos.y-iGhostBall.pos.y)*(iHero_1.pos.y-iGhostBall.pos.y)));
	double instance2=sqrt((long double)((iHero_2.pos.x-iGhostBall.pos.x)*(iHero_2.pos.x-iGhostBall.pos.x)+(iHero_2.pos.y-iGhostBall.pos.y)*(iHero_2.pos.y-iGhostBall.pos.y)));
	double instance1_1=sqrt((long double)((iHero_1.pos.x-iEnemy_1.pos.x)*(iHero_1.pos.x-iEnemy_1.pos.x)+(iHero_1.pos.y-iEnemy_1.pos.y)*(iHero_1.pos.y-iEnemy_1.pos.y)));
	double instance2_2=sqrt((long double)((iHero_2.pos.x-iEnemy_2.pos.x)*(iHero_2.pos.x-iEnemy_2.pos.x)+(iHero_2.pos.y-iEnemy_2.pos.y)*(iHero_2.pos.y-iEnemy_2.pos.y)));
	double instance2_b=sqrt((long double)((iHero_2.pos.x-iEnemyWithBall.pos.x)*(iHero_2.pos.x-iEnemyWithBall.pos.x)+(iHero_2.pos.y-iEnemyWithBall.pos.y)*(iHero_2.pos.y-iEnemyWithBall.pos.y)));
	double instance1_w=sqrt((long double)((iHero_1.pos.x-iEnemyWithoutBall.pos.x)*(iHero_1.pos.x-iEnemyWithoutBall.pos.x)+(iHero_1.pos.y-iEnemyWithoutBall.pos.y)*(iHero_1.pos.y-iEnemyWithoutBall.pos.y)));
	double instanceb_1=sqrt((long double)((iHeroWithBall.pos.x-iEnemy_1.pos.x)*(iHeroWithBall.pos.x-iEnemy_1.pos.x)+(iHeroWithBall.pos.y-iEnemy_1.pos.y)*(iHeroWithBall.pos.y-iEnemy_1.pos.y)));
	double instanceb_2=sqrt((long double)((iHeroWithBall.pos.x-iEnemy_2.pos.x)*(iHeroWithBall.pos.x-iEnemy_2.pos.x)+(iHeroWithBall.pos.y-iEnemy_2.pos.y)*(iHeroWithBall.pos.y-iEnemy_2.pos.y)));
	TC_Position point2;
	if(iAttackDirection == TC_DIRECTION_RIGHT)
	point2.x=iForbiddenArea.right.left-50;
	else
	point2.x=iForbiddenArea.left.right+50;
	point2.y=iGhostBall.pos.y;

	TC_Ball  BallPos;
	if(iAttackDirection == TC_DIRECTION_RIGHT)
	BallPos.pos.x=iGhostBall.pos.x-64;
	else
	BallPos.pos.x=iGhostBall.pos.x+32;

	BallPos.pos.y=iGhostBall.pos.y;
    if(iAttackDirection == TC_DIRECTION_RIGHT)
	iHero.pos.x=iHeroWithBall.pos.x-64;
	else
    iHero.pos.x=iHeroWithBall.pos.x+64;
    iHero.pos.y=iHeroWithBall.pos.y;
  
	




	if(instanceb_1<=instanceb_2)
	{
		hEnemynear=hEnemy_1;
		hEnemyfar=hEnemy_2;
	}
	else
	{
		hEnemynear=hEnemy_2;
		hEnemyfar=hEnemy_1;
	}
	
	
	if (iOwnGate.x < iEnemyGate.x)
	{
		iAttackDirection = TC_DIRECTION_RIGHT;
	}
	else
	{
		iAttackDirection = TC_DIRECTION_LEFT;
	}
	int state = GetState();
	
	TC_Position point;
	if(iAttackDirection == TC_DIRECTION_RIGHT)
		point.x=iHeroWithBall.pos.x+TC_PASSBALL_DISTANCE;
	else point.x=iHeroWithBall.pos.x-TC_PASSBALL_DISTANCE+64;
	point.y=iHeroWithBall.pos.y;
	
	TC_Position point1;
	if(iAttackDirection == TC_DIRECTION_RIGHT)
		point1.x=iForbiddenArea.right.left-80;
	else point1.x=iForbiddenArea.left.right;
	point1.y=iEnemyGate.y_lower-128;
	switch(state)
	{
	case GHOSTBALL_IN_NO_TEAM:
		{
			
			if(iGoldBall.b_visible==true)
			{
				if(iHero_1.b_snatch_goldball==true)
				{
					if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL))break;
				}
				if(iHero_2.b_snatch_goldball==true)
				{
					if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL))break;
				}
			}
			//  
			//  
			
			
			if(iHero_1.b_snatch_ghostball == false)
			{
		//			if(iFreeBall.speed.vx>0&&iFreeBall.b_visible==true&&(iFreeBall.pos.x-iHero_1.pos.x)<200&&(iFreeBall.pos.x-iHero_1.pos.x)>0)
		//				AvoidFreeBall(&iHero_1,hHero_1);
		//			else
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			}
			else Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
			
			if(iHero_2.b_snatch_ghostball == false)
			{
		//			if(iFreeBall.speed.vx>0&&iFreeBall.b_visible==true&&(iFreeBall.pos.x-iHero_2.pos.x)<200&&(iFreeBall.pos.x-iHero_2.pos.x)>0)
		//				AvoidFreeBall(&iHero_2,hHero_2);
		//			else
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			}
			else Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
			if((iAttackDirection == TC_DIRECTION_RIGHT&&abs(iGhostBall.pos.x-iForbiddenArea.right.left)<300)||(iAttackDirection==TC_DIRECTION_LEFT&&abs(iGhostBall.pos.x-iForbiddenArea.left.right)<300))
				{
					
					if(Tc_CanBeSpelled(hHero_2,hEnemynear))
					{
						Tc_Spell(hHero_2,hEnemynear);
					//	if(Tc_CanBeSpelled(hHero_1,hEnemyfar))
					//		Tc_Spell(hHero_1,hEnemyfar);
					}
					else
					{
						if(Tc_CanBeSpelled(hHero_1,hEnemynear))
							Tc_Spell(hHero_1,hEnemynear);
						if(Tc_CanBeSpelled(hHero_2,hEnemyfar))
							Tc_Spell(hHero_2,hEnemyfar);
					}
				}
				
			
			break;
		}
	case GHOSTBALL_IN_SELF_TEAM:
		{
			if(iGoldBall.b_visible==true)
			{
				if(iHero_1.b_snatch_goldball==true)
				{
					if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL))break;
				}
				if(iHero_2.b_snatch_goldball==true)
				{
					if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL))break;
				}
			}
			//hide游走球
			//hide游走球   
			if(iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE)
			{
				if(iHeroWithoutBall.b_snatch_ghostball == true )
				{
					Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
					break;
				}
			}
			hideginny();
			hideron();
/*			if(iEnemy_1.type==TC_HERO_GINNY)
			{
				if(iEnemy_1.b_is_spelling==true)
				{
					Tc_PassBall(hHeroWithBall,point2);
						Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
					break;
				}
			}
			
			if(iEnemy_2.type==TC_HERO_GINNY)
			{
				if(iEnemy_2.b_is_spelling==true)
				{
				Tc_PassBall(hHeroWithBall,point2);
					Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
					break;
				}
			}   
			
			if(iEnemy_1.type==TC_HERO_RON)
			{
				if(iEnemy_1.b_is_spelling==true)
				{
				Tc_PassBall(hHeroWithBall,point2);
					Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
					break;
				}
			}
			
			if(iEnemy_2.type==TC_HERO_RON)
			{
				if(iEnemy_2.b_is_spelling==true)
				{
					Tc_PassBall(hHeroWithBall,point2);
						Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
					break;
				}
			}       */
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHero));
			
			if((iAttackDirection == TC_DIRECTION_RIGHT&&iGhostBall.pos.x<iForbiddenArea.right.left-150)||(iAttackDirection==TC_DIRECTION_LEFT&&iGhostBall.pos.x>iForbiddenArea.left.right+100))
			{
				if((iAttackDirection == TC_DIRECTION_RIGHT&&abs(iGhostBall.pos.x-iForbiddenArea.right.left)<300)||(iAttackDirection==TC_DIRECTION_LEFT&&abs(iGhostBall.pos.x-iForbiddenArea.left.right)<300))
				{
					
					if(Tc_CanBeSpelled(hHero_2,hEnemynear))
					{
						Tc_Spell(hHero_2,hEnemynear);
					//	if(Tc_CanBeSpelled(hHero_1,hEnemyfar))
					//		Tc_Spell(hHero_1,hEnemyfar);
					}
					else
					{
						if(Tc_CanBeSpelled(hHero_1,hEnemynear))
							Tc_Spell(hHero_1,hEnemynear);
						if(Tc_CanBeSpelled(hHero_2,hEnemyfar))
							Tc_Spell(hHero_2,hEnemyfar);
					}
				}
				if((iAttackDirection == TC_DIRECTION_RIGHT&&point.x<iForbiddenArea.right.left-100)||(iAttackDirection==TC_DIRECTION_LEFT&&point.x>iForbiddenArea.left.right+100))
					Tc_PassBall(hHeroWithBall,point);
				else
					Tc_PassBall(hHeroWithBall,point1);
				
			}  
			else
			{
			//		if(iFreeBall.b_visible==true&&(iFreeBall.pos.x-iHeroWithBall.pos.x)<200&&(iFreeBall.pos.x-iHeroWithBall.pos.x)>0)
			//			AvoidFreeBall(&iHeroWithBall,hHeroWithBall);
		//			else
			
				Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			//	if(iFreeBall.b_visible==true&&(iFreeBall.pos.x-iHeroWithoutBall.pos.x)<200&&(iFreeBall.pos.x-iHeroWithoutBall.pos.x)>0)
			//			AvoidFreeBall(&iHeroWithoutBall,hHeroWithoutBall);
		//			else
				Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHero));  
			}	
			break;																															
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
		{
			if(iGoldBall.b_visible==true)
			{
				if(iHero_1.b_snatch_goldball==true)
				{
					if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL))break;
				}
				if(iHero_2.b_snatch_goldball==true)
				{
					if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL))break;
				}
			}
			if(iAttackDirection == TC_DIRECTION_RIGHT&&iGhostBall.pos.x<iForbiddenArea.left.right+200)
			{
              if(Tc_CanBeSpelled(hHero_1,hEnemyWithBall)&&iHero_1.b_can_spell==true)
			  {if(Tc_Spell(hHero_1,hEnemyWithBall))break;
			  }
               else 
				   if(iHero_1.b_is_spelling==false&&iHeroWithBall.abnormal_type!=TC_SPELLED_BY_RON&&Tc_CanBeSpelled(hHero_2,hEnemyWithBall)&&iHero_2.b_can_spell==true)
					   Tc_Spell(hHero_2,hEnemyWithBall);
			}
			

			if(iAttackDirection == TC_DIRECTION_LEFT&&iGhostBall.pos.x>iForbiddenArea.right.left-200)
			{
if(Tc_CanBeSpelled(hHero_1,hEnemyWithBall)&&iHero_1.b_can_spell==true){if(Tc_Spell(hHero_1,hEnemyWithBall))break;}
else if(iHero_1.b_is_spelling==false&&iHeroWithBall.abnormal_type!=TC_SPELLED_BY_RON&&Tc_CanBeSpelled(hHero_2,hEnemyWithBall)&&iHero_2.b_can_spell==true)
					   Tc_Spell(hHero_2,hEnemyWithBall);		}
			Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			if (iHero_1.b_snatch_ghostball == true)
			{
				if (Tc_SnatchBall(hHero_1,TC_GHOST_BALL) == true)
				{
					break;
				}
				else
				{
					Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
					Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
				}
				
			}
			if (iHero_2.b_snatch_ghostball == true)
			{
				if (Tc_SnatchBall(hHero_2,TC_GHOST_BALL) == true)
				{
					break;
				}
				else
				{
					Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
					Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
				}
			}
			break;
		}    
	default:
		{
			break;
		}
	}
}
