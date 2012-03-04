/*
队名：SUMMER
成员：0706550203冯朴夏，0706550233张萌
*/
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
	Tc_SetTeamName(L"SUMMER");
	
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"Hello");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_RON,L"Hey");
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
TC_Handle hGameInfo;
TC_Ball iFreeBall, iGhostBall, iGoldBall;
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2, iHeroWithBall, iHeroWithoutBall, iEnemyWithBall, iEnemyWithoutBall;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_Direction iAttackDirection;
TC_GameInfo iGameInfo;

TC_Direction GetAttackDirection(TC_Hero *pHero)
{
	if (pHero->pos.y >= iEnemyGate.y_lower-130)
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
//获取球场状态
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
		if (iEnemy_1.b_ghostball)
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
//获得敌人方向
TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pEnemy)
{
	if (pHero->pos.x > pEnemy->pos.x+2 && pHero->pos.y > pEnemy->pos.y+2)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x <= pEnemy->pos.x+2 && pHero->pos.x >= pEnemy->pos.x-2 && pHero->pos.y >= pEnemy->pos.y+10)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pEnemy->pos.x-2 && pHero->pos.y > pEnemy->pos.y+2)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x <= pEnemy->pos.x-2 && pHero->pos.y >= pEnemy->pos.y-2 && pHero->pos.y <= pEnemy->pos.y+10)return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pEnemy->pos.x-2 && pHero->pos.y < pEnemy->pos.y-2)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x >= pEnemy->pos.x-2 && pHero->pos.x <= pEnemy->pos.x+2 && pHero->pos.y <= pEnemy->pos.y-10)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pEnemy->pos.x+2 && pHero->pos.y < pEnemy->pos.y-2)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x >= pEnemy->pos.x+2 && pHero->pos.y >= pEnemy->pos.y-2 && pHero->pos.y <= pEnemy->pos.y+10)return TC_DIRECTION_LEFT;
}
//获得球的方向
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)
{
	if (pHero->pos.x > pBall->pos.x+2 && pHero->pos.y > pBall->pos.y+2)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x <= pBall->pos.x+2 && pHero->pos.x >= pBall->pos.x-2 && pHero->pos.y >= pBall->pos.y+10)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pBall->pos.x-2 && pHero->pos.y > pBall->pos.y+2)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x <= pBall->pos.x-2 && pHero->pos.y >= pBall->pos.y-2 && pHero->pos.y <= pBall->pos.y+10)return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pBall->pos.x-2 && pHero->pos.y < pBall->pos.y-2)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x >= pBall->pos.x-2 && pHero->pos.x <= pBall->pos.x+2 && pHero->pos.y <= pBall->pos.y-10)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pBall->pos.x+2 && pHero->pos.y < pBall->pos.y-2)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x >= pBall->pos.x+2 && pHero->pos.y >= pBall->pos.y-2 && pHero->pos.y <= pBall->pos.y+10)return TC_DIRECTION_LEFT;
}
//获得定点方向
TC_Direction GetPiontDirection(TC_Hero hero,TC_Position pos)
{
	if(hero.pos.x>pos.x+2&&hero.pos.y>pos.y+2)return TC_DIRECTION_LEFTTOP;
	if(hero.pos.x>pos.x+2&&hero.pos.y<pos.y-2)return TC_DIRECTION_LEFTBOTTOM;
	if(hero.pos.x<pos.x-2&&hero.pos.y>pos.y+2)return TC_DIRECTION_RIGHTTOP;
	if(hero.pos.x<pos.x-2&&hero.pos.y<pos.y-2)return TC_DIRECTION_RIGHTBOTTOM;
	if(hero.pos.x<=pos.x+2&&hero.pos.x>=pos.x-2&&hero.pos.y>=pos.y+10)return TC_DIRECTION_TOP;
	if(hero.pos.x>=pos.x-2&&hero.pos.x<=pos.x+2&&hero.pos.y<=pos.y-10)return TC_DIRECTION_BOTTOM;
	if(hero.pos.x>=pos.x+2&&hero.pos.y>=pos.y-2&&hero.pos.y<=pos.y+10)return TC_DIRECTION_LEFT;
	if(hero.pos.x<=pos.x-2&&hero.pos.y>=pos.y-2&&hero.pos.y<=pos.y+10)return TC_DIRECTION_RIGHT;
}
//获得英雄之间的距离
double GetDisHero(TC_Hero hero1,TC_Hero hero2)
{
	double a,b;
	a=(hero1.pos.x-hero2.pos.x)*(hero1.pos.x-hero2.pos.x);
	b=(hero1.pos.y-hero2.pos.y)*(hero1.pos.y-hero2.pos.y);
	return sqrt(a+b);
}
//获得英雄和人之间的距离
double GetDisBall(TC_Hero hero,TC_Ball ball)
{
	double a,b;
	a=(hero.pos.x-ball.pos.x)*(hero.pos.x-ball.pos.x);
	b=(hero.pos.y-ball.pos.y)*(hero.pos.y-ball.pos.y);
	return sqrt(a+b);
}
//获得自由球路径直线的斜率a(y=ax+b)
float GetA(TC_Ball freeball)
{
	if(abs(freeball.u.path.pos_start.x-freeball.u.path.pos_end.x)<=1)return 100;
	return (freeball.u.path.pos_start.y-freeball.u.path.pos_end.y)/(freeball.u.path.pos_start.x-freeball.u.path.pos_end.x);
}
//获得自由球路径直线的b
float GetB(TC_Ball freeball)
{
	return (freeball.u.path.pos_start.x*freeball.u.path.pos_end.y-freeball.u.path.pos_start.y*freeball.u.path.pos_end.x)/(freeball.u.path.pos_start.x-freeball.u.path.pos_end.x);
}
//获得上下位置
bool GetUp(float a,float b,TC_Position pos)
{
	if(pos.y<a*pos.x+b)return true;
	else return false;
}
//获得自由球与英雄之间的距离
float GetDistance(TC_Hero hero,TC_Ball freeball)
{
	return sqrt(float((hero.pos.x+64-freeball.pos.x)*(hero.pos.x+64-freeball.pos.x)+(hero.pos.y+64-freeball.pos.y)*(hero.pos.y+64-freeball.pos.y)));
}
//判断是否在禁区
bool GetInAreaR(TC_Position pos,TC_ForbiddenArea forbiddenarea)
{
	if(pos.x>=forbiddenarea.right.left&&pos.y>=forbiddenarea.right.top&&pos.y<=forbiddenarea.right.bottom)return true;
	else return false;
}
bool GetInAreaL(TC_Position pos,TC_ForbiddenArea forbiddenarea)
{
	if(pos.x<=forbiddenarea.left.right&&pos.y>=forbiddenarea.left.top&&pos.y<=forbiddenarea.left.bottom)return true;
	else return false;
}
//传球判断
TC_Position GetPassPosition(TC_Hero herowb,TC_Hero herowob,TC_Hero enemy1,TC_Hero enemy2,TC_Direction attack,TC_ForbiddenArea forbiddenarea)
{
	TC_Position pos;
	pos.x=5000;
	pos.y=2000;
	//向右攻击
	if(attack==TC_DIRECTION_RIGHT)
	{
	if(herowb.pos.x<forbiddenarea.right.left)
	{
		//向前传球
		if(enemy1.pos.x<herowb.pos.x&&enemy2.pos.x<herowb.pos.x)
		{
			pos.x=herowb.pos.x+500;
			pos.y=herowb.pos.y;
			if(GetInAreaR(pos,forbiddenarea))
			{
				if(herowb.pos.x<forbiddenarea.right.left-200)
				{
					pos.x=forbiddenarea.right.left-64;
					pos.y=384;
				}
			}
			return pos;
		}
		//半场上方传球
		if(herowb.pos.y<=384)
		{
			if(herowob.pos.x>enemy1.pos.x&&herowob.pos.x>enemy2.pos.x&&herowb.pos.x<herowob.pos.x)
			{
				pos.x=herowb.pos.x+400;
				pos.y=herowb.pos.y+250;
				return pos;
			}
			if(enemy1.pos.y<=herowb.pos.y&&enemy2.pos.y<=herowb.pos.y)
			{
				pos.x=herowb.pos.x+400;
				pos.y=herowb.pos.y+250;
				if(GetInAreaR(pos,forbiddenarea))
				{
					if(herowb.pos.x<forbiddenarea.right.left-200)
					{
						pos.x=forbiddenarea.right.left-64;
						pos.y=384;
					}
				}
				return pos;
			}
			if(enemy1.pos.y>herowb.pos.y&&enemy2.pos.y>herowb.pos.y)
			{
				if(enemy1.pos.x<=herowb.pos.x+50&&enemy2.pos.x<=herowb.pos.x+50)
				{
					pos.x=herowb.pos.x+500;
					pos.y=herowb.pos.y;
					if(GetInAreaR(pos,forbiddenarea))
					{
						if(herowb.pos.x<forbiddenarea.right.left-200)
						{
							pos.x=forbiddenarea.right.left+50;
							pos.y=forbiddenarea.right.top-64;
						}
					}
					return pos;
				}
				if(enemy1.pos.x<herowb.pos.x+100&&enemy2.pos.x<herowb.pos.x+100)
				{
					if(!enemy1.b_snatch_ghostball&&!enemy2.b_snatch_ghostball)
					{
						pos.x=herowb.pos.x+500;
						pos.y=herowb.pos.y;
						if(GetInAreaR(pos,forbiddenarea))
						{
							if(herowb.pos.x<forbiddenarea.right.left-200)
							{
							pos.x=forbiddenarea.right.left+50;
							pos.y=forbiddenarea.right.top-64;
							}
						}
					}
					return pos;
				}
				if((enemy1.pos.x<herowb.pos.x-150&&enemy2.pos.y-100>herowb.pos.y)||(enemy2.pos.x<herowb.pos.x-150&&enemy1.pos.y-100>herowb.pos.y))
				{
					pos.x=herowb.pos.x+500;
					pos.y=herowb.pos.y;
					if(GetInAreaR(pos,forbiddenarea))
					{
						if(herowb.pos.x<forbiddenarea.right.left-200)
						{
							pos.x=forbiddenarea.right.left+50;
							pos.y=forbiddenarea.right.top-64;
						}
					}
					return pos;
				}
				if((enemy1.pos.x<herowb.pos.x-150&&enemy2.pos.y-100<herowb.pos.y)||(enemy2.pos.x<herowb.pos.x-150&&enemy1.pos.y-100<herowb.pos.y))
				{
					pos.x=herowb.pos.x+400;
					pos.y=herowb.pos.y+250;
					if(GetInAreaR(pos,forbiddenarea))
					{
						if(herowb.pos.x<forbiddenarea.right.left-200)
						{
							pos.x=forbiddenarea.right.left+50;
							pos.y=forbiddenarea.right.top-64;
						}
					}
					return pos;
				}
			}
			//敌人一上一下
			if((enemy1.pos.x<herowb.pos.x&&enemy2.pos.x>herowb.pos.x))
			{
				pos.x=herowb.pos.x+300;
				pos.y=(enemy1.pos.y+enemy2.pos.y)/2;
				if(GetInAreaR(pos,forbiddenarea))
				{
					if(herowb.pos.x<forbiddenarea.right.left-200&&enemy2.pos.y>herowb.pos.y)
					{
						pos.x=forbiddenarea.right.left+50;
						pos.y=forbiddenarea.right.top-64;
					}
					if(herowb.pos.x<forbiddenarea.right.left-200&&enemy2.pos.y<herowb.pos.y)
					{
						pos.x=forbiddenarea.right.left-64;
						pos.y=384;
					}
				}
				return pos;
			}
			if((enemy1.pos.x>herowb.pos.x&&enemy2.pos.x<herowb.pos.x))
			{
				pos.x=herowb.pos.x+300;
				pos.y=(enemy1.pos.y+enemy2.pos.y)/2;
				if(GetInAreaR(pos,forbiddenarea))
				{
					if(herowb.pos.x<forbiddenarea.right.left-200&&enemy1.pos.y>herowb.pos.y)
					{
						pos.x=forbiddenarea.right.left+50;
						pos.y=forbiddenarea.right.top-64;
					}
					if(herowb.pos.x<forbiddenarea.right.left-200&&enemy1.pos.y<herowb.pos.y)
					{
						pos.x=forbiddenarea.right.left-64;
						pos.y=384;
					}
				}
				return pos;
			}
			if(abs(herowb.pos.y-enemy1.pos.y)<50&&abs(herowb.pos.y-enemy2.pos.y)<50)
			{
				pos.x=herowb.pos.x+400;
				pos.x=herowb.pos.y+250;
				return pos;
			}
		}
		//半场下方
		else
		{
			if(herowob.pos.x>enemy1.pos.x&&herowob.pos.x>enemy2.pos.x&&herowb.pos.x<herowob.pos.x)
			{
				pos.x=herowb.pos.x+400;
				pos.y=herowb.pos.y-250;
				return pos;
			}
			if(enemy1.pos.y>=herowb.pos.y&&enemy2.pos.y>=herowb.pos.y)
			{
				pos.x=herowb.pos.x+400;
				pos.y=herowb.pos.y-250;
				if(GetInAreaR(pos,forbiddenarea))
				{
					if(herowb.pos.x<forbiddenarea.left.right-250)
					{
						pos.x=forbiddenarea.right.left-64;
						pos.y=384;
					}
				}
				return pos;
			}
			if(enemy1.pos.y<herowb.pos.y&&enemy2.pos.y<herowb.pos.y)
			{
				if(enemy1.pos.x<=herowb.pos.x+50&&enemy2.pos.x<=herowb.pos.x+50)
				{
					pos.x=herowb.pos.x+500;
					pos.y=herowb.pos.y;
					if(GetInAreaR(pos,forbiddenarea))
					{
						if(herowb.pos.x<forbiddenarea.right.left-200)
						{
							pos.x=forbiddenarea.right.left+50;
							pos.y=forbiddenarea.right.bottom+1;
						}
					}
					return pos;
				}
				if(enemy1.pos.x<herowb.pos.x+100&&enemy2.pos.x>herowb.pos.x+100)
				{
					if(!enemy1.b_snatch_ghostball&&!enemy2.b_snatch_ghostball)
					{
						pos.x=herowb.pos.x+500;
						pos.y=herowb.pos.y;
						if(GetInAreaR(pos,forbiddenarea))
						{
							if(herowb.pos.x<forbiddenarea.right.left-200)
							{
							pos.x=forbiddenarea.right.left+50;
							pos.y=forbiddenarea.right.bottom+1;
							}
						}
					}
					return pos;
				}
				if((enemy1.pos.x<herowb.pos.x-150&&enemy2.pos.x>herowb.pos.x)||(enemy2.pos.x<herowb.pos.x-150&&enemy1.pos.x>herowb.pos.x))
				{
					pos.x=herowb.pos.x+500;
					pos.y=herowb.pos.y;
					if(GetInAreaR(pos,forbiddenarea))
					{
						if(herowb.pos.x<forbiddenarea.right.left-200)
						{
						pos.x=forbiddenarea.right.left+50;
						pos.y=forbiddenarea.right.bottom+1;
						}
					}
					return pos;
				}
			}
			if((enemy1.pos.x<herowb.pos.x&&enemy2.pos.x>herowb.pos.x))
			{
				pos.x=herowb.pos.x+300;
				pos.y=(enemy1.pos.y+enemy2.pos.y)/2;
				if(GetInAreaR(pos,forbiddenarea))
				{
					if(herowb.pos.x<forbiddenarea.right.left-200&&enemy2.pos.y>herowb.pos.y)
					{
						pos.x=forbiddenarea.right.left+50;
						pos.y=forbiddenarea.right.bottom+1;
					}
					if(herowb.pos.x<forbiddenarea.right.left-200&&enemy2.pos.y<herowb.pos.y)
					{
						pos.x=forbiddenarea.right.left-64;
						pos.y=384;
					}
				}
				return pos;
			}
			if((enemy1.pos.x>herowb.pos.x&&enemy2.pos.x<herowb.pos.x))
			{
				pos.x=herowb.pos.x+300;
				pos.y=(enemy1.pos.y+enemy2.pos.y)/2;
				if(GetInAreaR(pos,forbiddenarea))
				{
					if(herowb.pos.x<forbiddenarea.right.left-200&&enemy1.pos.y>herowb.pos.y)
					{
						pos.x=forbiddenarea.right.left+50;
						pos.y=forbiddenarea.right.bottom+1;
					}
					if(herowb.pos.x<forbiddenarea.right.left-200&&enemy1.pos.y<herowb.pos.y)
					{
						pos.x=forbiddenarea.right.left-64;
						pos.y=384;
					}
				}
				return pos;
			}
			if(abs(herowb.pos.y-enemy1.pos.y)<50&&abs(herowb.pos.y-enemy2.pos.y)<50)
			{
				pos.x=herowb.pos.x+400;
				pos.x=herowb.pos.y-250;
				return pos;
			}
		}
	}
			
	}
	//向左攻击
	else
	{
		if(herowb.pos.x>forbiddenarea.left.right)
	{
		//向前传球
		if(enemy1.pos.x>herowb.pos.x&&enemy2.pos.x>herowb.pos.x)
		{
			pos.x=herowb.pos.x-500;
			pos.y=herowb.pos.y;
			if(GetInAreaL(pos,forbiddenarea))
			{
				if(herowb.pos.x>forbiddenarea.left.right+150)
				{
					pos.x=forbiddenarea.left.right+1;
					pos.y=384;
				}
			}
			return pos;
		}
		//半场上方传球
		if(herowb.pos.y<384)
		{
			if(herowob.pos.x<enemy1.pos.x&&herowob.pos.x<enemy2.pos.x&&herowb.pos.x>herowob.pos.x)
			{
				pos.x=herowb.pos.x-400;
				pos.y=herowb.pos.y+250;
				return pos;
			}
			if(enemy1.pos.y<=herowb.pos.y&&enemy2.pos.y<=herowb.pos.y)
			{
				pos.x=herowb.pos.x-400;
				pos.y=herowb.pos.y+250;
				if(GetInAreaL(pos,forbiddenarea))
				{
					if(herowb.pos.x>forbiddenarea.left.right+150)
					{
					pos.x=forbiddenarea.left.right+1;
					pos.y=384;
					}
				}
				return pos;
			}
			if(enemy1.pos.y>herowb.pos.y&&enemy2.pos.y>herowb.pos.y)
			{
				if(enemy1.pos.x>=herowb.pos.x-50&&enemy2.pos.x>=herowb.pos.x-50)
				{
					pos.x=herowb.pos.x-500;
					pos.y=herowb.pos.y;
					if(GetInAreaL(pos,forbiddenarea))
					{
						if(herowb.pos.x>forbiddenarea.left.right+200)
						{
						pos.x=forbiddenarea.left.right-50;
						pos.y=forbiddenarea.left.top-64;
						}
					}
					return pos;
				}
				if(enemy1.pos.x>herowb.pos.x-100&&enemy2.pos.x>herowb.pos.x-100)
				{
					if(!enemy1.b_snatch_ghostball&&!enemy2.b_snatch_ghostball)
					{
						pos.x=herowb.pos.x+500;
						pos.y=herowb.pos.y;
						if(GetInAreaL(pos,forbiddenarea))
						{
							if(herowb.pos.x>forbiddenarea.left.right+200)
							{
							pos.x=forbiddenarea.right.left+50;
							pos.y=forbiddenarea.right.top-64;
							}
						}
					}
					return pos;
				}
				if((enemy1.pos.x>herowb.pos.x+150&&enemy2.pos.x<herowb.pos.x)||(enemy2.pos.x>herowb.pos.x+150&&enemy1.pos.x<herowb.pos.x))
				{
					pos.x=herowb.pos.x-500;
					pos.y=herowb.pos.y;
					if(GetInAreaL(pos,forbiddenarea))
					{
						if(herowb.pos.x>forbiddenarea.left.right+200)
						{
						pos.x=forbiddenarea.left.right+1;
						pos.y=forbiddenarea.left.top-64;
						}
					}
					return pos;
				}
			}
			if((enemy1.pos.x<herowb.pos.x&&enemy2.pos.x>herowb.pos.x))
			{
				pos.x=herowb.pos.x-400;
				pos.y=(enemy1.pos.y+enemy2.pos.y)/2;
				if(GetInAreaL(pos,forbiddenarea))
				{
					if(herowb.pos.x>forbiddenarea.left.right+200&&enemy1.pos.y>herowb.pos.y)
					{
						pos.x=forbiddenarea.left.right+1;
						pos.y=forbiddenarea.left.top-64;
					}
					if(herowb.pos.x>forbiddenarea.left.right+200&&enemy1.pos.y<herowb.pos.y)
					{
						pos.x=forbiddenarea.left.right+1;
						pos.y=384;
					}
				}
				return pos;
			}
			if((enemy1.pos.x>herowb.pos.x&&enemy2.pos.x<herowb.pos.x))
			{
				pos.x=herowb.pos.x-400;
				pos.y=(enemy1.pos.y+enemy2.pos.y)/2;
				if(GetInAreaL(pos,forbiddenarea))
				{
					if(herowb.pos.x>forbiddenarea.left.right+200&&enemy2.pos.y>herowb.pos.y)
					{
						pos.x=forbiddenarea.left.right+1;
						pos.y=forbiddenarea.left.top-64;
					}
					if(herowb.pos.x>forbiddenarea.left.right+200&&enemy2.pos.y<herowb.pos.y)
					{
						pos.x=forbiddenarea.left.right+1;
						pos.y=384;
					}
				}
				return pos;
			}
			if(abs(herowb.pos.y-enemy1.pos.y)<50&&abs(herowb.pos.y-enemy2.pos.y)<50)
			{
				pos.x=herowb.pos.x-400;
				pos.x=herowb.pos.y+250;
				return pos;
			}
		}
		//下方
		else
		{
			if(herowob.pos.x<enemy1.pos.x&&herowob.pos.x<enemy2.pos.x&&herowb.pos.x>herowob.pos.x)
			{
				pos.x=herowb.pos.x-400;
				pos.y=herowb.pos.y-250;
				return pos;
			}
			if(enemy1.pos.y<=herowb.pos.y&&enemy2.pos.y<=herowb.pos.y)
			{
				pos.x=herowb.pos.x-400;
				pos.y=herowb.pos.y-250;
				if(GetInAreaL(pos,forbiddenarea))
				{
					if(herowb.pos.x>forbiddenarea.left.right+150)
					{
					pos.x=forbiddenarea.left.right+1;
					pos.y=384;
					}
				}
				return pos;
			}
			if(enemy1.pos.y>herowb.pos.y&&enemy2.pos.y>herowb.pos.y)
			{
				if(enemy1.pos.x>=herowb.pos.x-50&&enemy2.pos.x>=herowb.pos.x-50)
				{
					pos.x=herowb.pos.x-500;
					pos.y=herowb.pos.y;
					if(GetInAreaL(pos,forbiddenarea))
					{
						if(herowb.pos.x>forbiddenarea.left.right+200)
						{
						pos.x=forbiddenarea.left.right+1;
						pos.y=forbiddenarea.left.bottom+1;
						}
					}
					return pos;
				}
				if(enemy1.pos.x>herowb.pos.x-100&&enemy2.pos.x>herowb.pos.x-100)
				{
					if(!enemy1.b_snatch_ghostball&&!enemy2.b_snatch_ghostball)
					{
						pos.x=herowb.pos.x-500;
						pos.y=herowb.pos.y;
						if(GetInAreaL(pos,forbiddenarea))
						{
							if(herowb.pos.x>forbiddenarea.left.right+200)
							{
							pos.x=forbiddenarea.left.right+1;
							pos.y=forbiddenarea.left.bottom+1;
							}
						}
					}
					return pos;
				}
				if((enemy1.pos.x>herowb.pos.x+150&&enemy2.pos.x<herowb.pos.x)||(enemy2.pos.x>herowb.pos.x+150&&enemy1.pos.x<herowb.pos.x))
				{
					pos.x=herowb.pos.x-500;
					pos.y=herowb.pos.y;
					if(GetInAreaL(pos,forbiddenarea))
					{
						if(herowb.pos.x>forbiddenarea.left.right+200)
						{
						pos.x=forbiddenarea.left.right+1;
						pos.y=forbiddenarea.left.bottom+1;
						}
					}
					return pos;
				}
				if((enemy1.pos.x<herowb.pos.x&&enemy2.pos.x>herowb.pos.x))
				{
					pos.x=herowb.pos.x-400;
					pos.y=(enemy1.pos.y+enemy2.pos.y)/2;
					if(GetInAreaL(pos,forbiddenarea))
					{
						if(herowb.pos.x>forbiddenarea.left.right+200&&enemy1.pos.y<herowb.pos.y)
						{
							pos.x=forbiddenarea.left.right+1;
							pos.y=forbiddenarea.left.bottom+1;
						}
						if(herowb.pos.x>forbiddenarea.left.right+200&&enemy1.pos.y>herowb.pos.y)
						{
							pos.x=forbiddenarea.left.right+1;
							pos.y=384;
						}
					}
					return pos;
				}
				if((enemy1.pos.x>herowb.pos.x&&enemy2.pos.x<herowb.pos.x))
				{
					pos.x=herowb.pos.x-400;
					pos.y=(enemy1.pos.y+enemy2.pos.y)/2;
					if(GetInAreaL(pos,forbiddenarea))
					{
						if(herowb.pos.x>forbiddenarea.left.right+200&&enemy2.pos.y<herowb.pos.y)
						{
							pos.x=forbiddenarea.left.right+1;
							pos.y=forbiddenarea.left.bottom+1;
						}
						if(herowb.pos.x>forbiddenarea.left.right+200&&enemy2.pos.y>herowb.pos.y)
						{
							pos.x=forbiddenarea.left.right+1;
							pos.y=384;
						}
					}
					return pos;
				}
			}
			if(abs(herowb.pos.y-enemy1.pos.y)<50&&abs(herowb.pos.y-enemy2.pos.y)<50)
			{
				pos.x=herowb.pos.x-400;
				pos.x=herowb.pos.y-250;
				return pos;
			}
		}
	}
	}
}
//进攻躲球
TC_Direction GetADirection(TC_Hero hero,TC_Ball freeball,TC_Direction attackdirection)
{
	TC_Position start,end;
	float a,b,m;
	a=GetA(freeball);
	b=GetB(freeball);
	m=(freeball.u.path.pos_start.y+freeball.u.path.pos_end.y)/2;
	if(freeball.u.path.pos_start.x>=freeball.u.path.pos_end.x)
	{
		start.x=freeball.u.path.pos_start.x;
		start.y=freeball.u.path.pos_start.y;
		end.x=freeball.u.path.pos_end.x;
		end.y=freeball.u.path.pos_end.y;
	}
	else
	{
		start.x=freeball.u.path.pos_end.x;
		start.y=freeball.u.path.pos_end.y;
		end.x=freeball.u.path.pos_start.x;
		end.y=freeball.u.path.pos_start.y;
	}
	if(attackdirection==TC_DIRECTION_RIGHT)
	{
		if(a>0.75)
		{
			if(GetUp(a,b,hero.pos))return GetAttackDirection(&hero);
			if(GetDistance(hero,freeball)>=100)return GetAttackDirection(&hero);
			else
			{
				return TC_DIRECTION_LEFTBOTTOM;
			}
		}
		if(a>=0&&a<=0.75)
		{
			if(GetUp(a,b,hero.pos))return GetAttackDirection(&hero);
			if(GetDistance(hero,freeball)>=100)return GetAttackDirection(&hero);
		else
			{
				return TC_DIRECTION_BOTTOM;
			}
		}
		if(a<0&&a>=-0.75)
		{
			if(!GetUp(a,b,hero.pos))return GetAttackDirection(&hero);
			if(GetDistance(hero,freeball)>=100)return GetAttackDirection(&hero);
			else
			{
				return TC_DIRECTION_TOP;
			}
		}
		if(a<-0.75)
		{
			if(!GetUp(a,b,hero.pos))return GetAttackDirection(&hero);
			if(GetDistance(hero,freeball)>=100)return GetAttackDirection(&hero);
			else
			{
				return TC_DIRECTION_LEFTTOP;
			}
		}
	}
	else
	{
		if(a>0.75)
		{
			if(!GetUp(a,b,hero.pos))return GetAttackDirection(&hero);
			if(GetDistance(hero,freeball)>=100)return GetAttackDirection(&hero);
			else
			{
				return TC_DIRECTION_RIGHTTOP;
			}
		}
		if(a>=0&&a<=0.75)
		{
			if(!GetUp(a,b,hero.pos))return GetAttackDirection(&hero);
			if(GetDistance(hero,freeball)>=100)return GetAttackDirection(&hero);
			else
			{
				return TC_DIRECTION_TOP;
			}
		}
		if(a<0&&a>=-0.75)
		{
			if(GetUp(a,b,hero.pos))return GetAttackDirection(&hero);
			if(GetDistance(hero,freeball)>=100)return GetAttackDirection(&hero);
			else
			{
				return TC_DIRECTION_BOTTOM;
			}
		}
		if(a<-0.75)
		{
			if(GetUp(a,b,hero.pos))return GetAttackDirection(&hero);
			if(GetDistance(hero,freeball)>=100)return GetAttackDirection(&hero);
			else
			{
				return TC_DIRECTION_RIGHTBOTTOM;
			}
		}
	}
}
//获得飞贼上下相反位置方向
TC_Direction GetGoldUD(TC_Hero hero,TC_Ball goldball)
{
	TC_Position pos;
	pos.x=goldball.pos.x;
	pos.y=TC_MAP_HEIGHT-goldball.pos.y;
	if(hero.pos.x>pos.x+2&&hero.pos.y>pos.y+2)return TC_DIRECTION_LEFTTOP;
	if(hero.pos.x>pos.x+2&&hero.pos.y<pos.y-2)return TC_DIRECTION_LEFTBOTTOM;
	if(hero.pos.x<pos.x-2&&hero.pos.y>pos.y+2)return TC_DIRECTION_RIGHTTOP;
	if(hero.pos.x<pos.x-2&&hero.pos.y<pos.y-2)return TC_DIRECTION_RIGHTBOTTOM;
	if(hero.pos.x<=pos.x+2&&hero.pos.x>=pos.x-2&&hero.pos.y>=pos.y+10)return TC_DIRECTION_TOP;
	if(hero.pos.x>=pos.x-2&&hero.pos.x<=pos.x+2&&hero.pos.y<=pos.y-10)return TC_DIRECTION_BOTTOM;
	if(hero.pos.x>=pos.x+2&&hero.pos.y>=pos.y-2&&hero.pos.y<=pos.y+10)return TC_DIRECTION_LEFT;
	if(hero.pos.x<=pos.x-2&&hero.pos.y>=pos.y-2&&hero.pos.y<=pos.y+10)return TC_DIRECTION_RIGHT;
}
//获得飞贼左右相反位置方向
TC_Direction GetGoldRL(TC_Hero hero,TC_Ball goldball)
{
	TC_Position pos;
	pos.x=TC_MAP_WIDTH-goldball.pos.x;
	pos.y=goldball.pos.y;
	if(hero.pos.x>pos.x+2&&hero.pos.y>pos.y+2)return TC_DIRECTION_LEFTTOP;
	if(hero.pos.x>pos.x+2&&hero.pos.y<pos.y-2)return TC_DIRECTION_LEFTBOTTOM;
	if(hero.pos.x<pos.x-2&&hero.pos.y>pos.y+2)return TC_DIRECTION_RIGHTTOP;
	if(hero.pos.x<pos.x-2&&hero.pos.y<pos.y-2)return TC_DIRECTION_RIGHTBOTTOM;
	if(hero.pos.x<=pos.x+2&&hero.pos.x>=pos.x-2&&hero.pos.y>=pos.y+10)return TC_DIRECTION_TOP;
	if(hero.pos.x>=pos.x-2&&hero.pos.x<=pos.x+2&&hero.pos.y<=pos.y-10)return TC_DIRECTION_BOTTOM;
	if(hero.pos.x>=pos.x+2&&hero.pos.y>=pos.y-2&&hero.pos.y<=pos.y+10)return TC_DIRECTION_LEFT;
	if(hero.pos.x<=pos.x-2&&hero.pos.y>=pos.y-2&&hero.pos.y<=pos.y+10)return TC_DIRECTION_RIGHT;
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
	Tc_GetGameInfo(hHero_1,&iGameInfo);

	Tc_GetOwnGate(hHero_1,&iOwnGate);
	Tc_GetEnemyGate(hEnemy_1,&iEnemyGate);
	Tc_GetForbiddenArea(&iForbiddenArea);
	
	TC_Position pos;
	if (iOwnGate.x < iEnemyGate.x)
	{
		iAttackDirection = TC_DIRECTION_RIGHT;
	}
	else
	{
		iAttackDirection = TC_DIRECTION_LEFT;
	}
	int state = GetState();
	if(iGoldBall.b_visible)
	{
		if(GetDisBall(iHero_1,iGoldBall)<=250&&iGoldBall.u.slow_down_steps_left>0)
		{
			if(iHero_1.b_snatch_goldball)
			{
				if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL));
			}
			Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
		}
		if(GetDisBall(iHero_2,iGoldBall)<=250&&iGoldBall.u.slow_down_steps_left>0)
		{
			if(iHero_2.b_snatch_goldball)
			{
				if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL));
			}
			Tc_Move(hHero_2,GetBallDirection(&iHero_1,&iGoldBall));
		}
		if(GetDisBall(iHero_1,iGoldBall)<=120)
		{
			if(iHero_1.b_snatch_goldball)
			{
				if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL));
			}
			Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
		}
		if(GetDisBall(iHero_2,iGoldBall)<=120)
		{
			if(iHero_2.b_snatch_goldball)
			{
				if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL));
			}
			Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
		}
		if(GetDisBall(iEnemy_1,iGoldBall)<=150)
		{
			if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2,hEnemy_1)&&iEnemy_1.abnormal_type==TC_SPELLED_BY_NONE)
			{
				Tc_Spell(hHero_2,hEnemy_1);
			}
			if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1,hEnemy_1)&&iEnemy_1.abnormal_type==TC_SPELLED_BY_NONE)
			{
				Tc_Spell(hHero_1,hEnemy_1);
			}
		}
		if(GetDisBall(iEnemy_2,iGoldBall)<=150)
		{
			if(iHero_2.b_can_spell&&Tc_CanBeSpelled(hHero_2,hEnemy_2)&&iEnemy_2.abnormal_type==TC_SPELLED_BY_NONE)
			{
				Tc_Spell(hHero_2,hEnemy_2);
			}
			if(iHero_1.b_can_spell&&Tc_CanBeSpelled(hHero_1,hEnemy_2)&&iEnemy_2.abnormal_type==TC_SPELLED_BY_NONE)
			{
				Tc_Spell(hHero_1,hEnemy_2);
			}
		}
	}
		if(iGoldBall.b_visible&&iGameInfo.score_self+2<=iGameInfo.score_enemy)
		{
			if(iGoldBall.pos.x>iForbiddenArea.left.right-100&&iGoldBall.pos.x<iForbiddenArea.right.left+100)
			{
				if(iGoldBall.pos.y<384)
				{
					Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
					Tc_Move(hHero_2,GetGoldUD(iHero_2,iGoldBall));
				}
				else
				{
					Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
					Tc_Move(hHero_1,GetGoldUD(iHero_1,iGoldBall));
				}
			}
			if(iGoldBall.pos.x<iForbiddenArea.left.right-100||iGoldBall.pos.x>iForbiddenArea.right.left+100)
			{
				if(iGoldBall.pos.x<1024)
				{
					Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
					Tc_Move(hHero_2,GetGoldRL(iHero_2,iGoldBall));
				}
				else
				{
					Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
					Tc_Move(hHero_1,GetGoldRL(iHero_1,iGoldBall));
				}
			}
		}
		if(iGoldBall.b_visible&&iGameInfo.score_self>iGameInfo.score_enemy+1)
		{
			if(iEnemy_1.b_ghostball||iEnemy_2.b_ghostball)
			{
				if(iEnemy_1.b_ghostball)
				{
					if(iHero_1.b_snatch_goldball)
					{
						if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL));
					}
					if(iHero_2.b_snatch_goldball)
					{
						if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL));
					}
					Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemy_1));
					Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemy_2));
					if(abs(iEnemy_1.pos.x-iOwnGate.x)<300)
					{
						if(iEnemy_1.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_1,hEnemy_1))
							Tc_Spell(hHero_1,hEnemy_1);
					}
				}
				if(iEnemy_2.b_ghostball)
				{
					if(iHero_1.b_snatch_goldball)
					{
						if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL));
					}
					if(iHero_2.b_snatch_goldball)
					{
						if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL));
					}
					Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemy_1));
					Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemy_2));
					if(abs(iEnemy_2.pos.x-iOwnGate.x)<300)
					{
						if(iEnemy_2.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_2,hEnemy_2))
							Tc_Spell(hHero_2,hEnemy_2);
					}
				}
			}
			if(iHero_1.b_ghostball||iHero_2.b_ghostball)
			{
				if(iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE||iHeroWithBall.abnormal_type==TC_SPELLED_BY_RON)
				{
					if(iHeroWithoutBall.b_snatch_ghostball)
						Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
				}
				if(iEnemy_1.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_GINNY)
				{
					if(iAttackDirection==TC_DIRECTION_RIGHT)
					{
						pos.x=iForbiddenArea.right.left+50;
						pos.y=iForbiddenArea.right.top-64;
						if((iEnemy_1.type==TC_HERO_GINNY&&iEnemy_1.b_is_spelling)||(iEnemy_2.type==TC_HERO_GINNY&&iEnemy_2.b_is_spelling))
						{
							if(iEnemy_1.pos.x+200<iHeroWithBall.pos.x&&iEnemy_2.pos.x+200<iHeroWithBall.pos.x)
							{
								pos.x=iForbiddenArea.right.left-65;
								pos.y=(iForbiddenArea.right.bottom+iForbiddenArea.right.top)/2;
							}
							if(iEnemy_1.pos.y>iHeroWithBall.pos.y&&iEnemy_2.pos.y>iHeroWithBall.pos.y)
							{
								pos.x=iForbiddenArea.right.left+50;
								pos.y=iForbiddenArea.right.top-64;
							}
							if(iEnemy_1.pos.y<iHeroWithBall.pos.y&&iEnemy_2.pos.y<iHeroWithBall.pos.y)
							{
									pos.x=iForbiddenArea.right.left+50;
								pos.y=iForbiddenArea.right.bottom+1;
							}
							Tc_PassBall(hHeroWithBall,pos);
						}
						}
					else
						{
						pos.x=iForbiddenArea.left.right+50;
						pos.y=iForbiddenArea.left.top-64;
						if((iEnemy_1.type==TC_HERO_GINNY&&iEnemy_1.b_is_spelling)||(iEnemy_2.type==TC_HERO_GINNY&&iEnemy_2.b_is_spelling))
						{
								if(iEnemy_1.pos.x-200>iHeroWithBall.pos.x&&iEnemy_2.pos.x-200>iHeroWithBall.pos.x)
							{
								pos.x=iForbiddenArea.left.right-50;
								pos.y=(iForbiddenArea.left.bottom+iForbiddenArea.left.top)/2;
							}
							if(iEnemy_1.pos.y>iHeroWithBall.pos.y&&iEnemy_2.pos.y>iHeroWithBall.pos.y)
							{
								pos.x=iForbiddenArea.left.right-50;
								pos.y=iForbiddenArea.left.top-64;
							}
							if(iEnemy_1.pos.y<iHeroWithBall.pos.y&&iEnemy_2.pos.y<iHeroWithBall.pos.y)
							{
								pos.x=iForbiddenArea.left.right-50;
								pos.y=iForbiddenArea.left.bottom+1;
							}
							Tc_PassBall(hHeroWithBall,pos);
						}	
					}	
				}	
				if(iEnemy_1.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_RON)
				{
					if(iAttackDirection==TC_DIRECTION_RIGHT)
					{
						pos.x=iForbiddenArea.right.left+50;
						pos.y=iForbiddenArea.right.top-64;
						if((iEnemy_1.type==TC_HERO_RON&&iEnemy_1.b_is_spelling)||(iEnemy_2.type==TC_HERO_RON&&iEnemy_2.b_is_spelling))
						{
							if(iEnemy_1.pos.x+200<iHeroWithBall.pos.x&&iEnemy_2.pos.x+200<iHeroWithBall.pos.x)
							{
								pos.x=iForbiddenArea.right.left-65;
								pos.y=(iForbiddenArea.right.bottom+iForbiddenArea.right.top)/2;
							}
							if(iEnemy_1.pos.y>iHeroWithBall.pos.y&&iEnemy_2.pos.y>iHeroWithBall.pos.y)
							{
								pos.x=iForbiddenArea.right.left+50;
								pos.y=iForbiddenArea.right.top-64;
							}
							if(iEnemy_1.pos.y<iHeroWithBall.pos.y&&iEnemy_2.pos.y<iHeroWithBall.pos.y)
							{
								pos.x=iForbiddenArea.right.left+50;
								pos.y=iForbiddenArea.right.bottom+1;
							}
							Tc_PassBall(hHeroWithBall,pos);
						}
					}
					else
					{
						pos.x=iForbiddenArea.left.right+50;
						pos.y=iForbiddenArea.left.top-64;
						if((iEnemy_1.type==TC_HERO_RON&&iEnemy_1.b_is_spelling)||(iEnemy_2.type==TC_HERO_RON&&iEnemy_2.b_is_spelling))
						{
							if(iEnemy_1.pos.x-200>iHeroWithBall.pos.x&&iEnemy_2.pos.x-200>iHeroWithBall.pos.x)
							{
								pos.x=iForbiddenArea.left.right-50;
								pos.y=(iForbiddenArea.left.bottom+iForbiddenArea.left.top)/2;
							}
							if(iEnemy_1.pos.y>iHeroWithBall.pos.y&&iEnemy_2.pos.y>iHeroWithBall.pos.y)
							{
								pos.x=iForbiddenArea.left.right-50;
								pos.y=iForbiddenArea.left.top-64;
							}
							if(iEnemy_1.pos.y<iHeroWithBall.pos.y&&iEnemy_2.pos.y<iHeroWithBall.pos.y)
							{
								pos.x=iForbiddenArea.left.right-50;
								pos.y=iForbiddenArea.left.bottom+1;
							}
							Tc_PassBall(hHeroWithBall,pos);
						}
					}
				}
				Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
				Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iHero_1));
				if(iHeroWithBall.pos.x<iForbiddenArea.right.left-150&&iAttackDirection==TC_DIRECTION_RIGHT)
				{
					Tc_PassBall(hHeroWithBall,GetPassPosition(iHeroWithBall,iHeroWithoutBall,iEnemy_1,iEnemy_2,iAttackDirection,iForbiddenArea));
				}
				if(iHeroWithBall.pos.x>iForbiddenArea.left.right+50&&iAttackDirection==TC_DIRECTION_LEFT)
				{
					Tc_PassBall(hHeroWithBall,GetPassPosition(iHeroWithBall,iHeroWithoutBall,iEnemy_1,iEnemy_2,iAttackDirection,iForbiddenArea));
				}
			}
			else
			{
				Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemy_1));
				Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemy_2));
			}
		}
	else
	{
	switch(state)
	{
	case GHOSTBALL_IN_NO_TEAM:
		{
			if(iHero_1.b_snatch_ghostball)
			{
				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
					break;
			}
			if(iHero_2.b_snatch_ghostball)
			{
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					break;
			}
			if(iGhostBall.speed.vx==0&&iGhostBall.speed.vy==0)
			{
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			}	
			else
			{
				Tc_Move(hHero_1,GetPiontDirection(iHero_1,iGhostBall.u.target));
				Tc_Move(hHero_2,GetPiontDirection(iHero_2,iGhostBall.u.target));
			}
			//技能施放
			if(iHero_1.curr_blue>1700)
			{
				if(GetDisBall(iEnemy_1,iGhostBall)<=GetDisBall(iEnemy_2,iGhostBall))
				{
					if(GetDisBall(iEnemy_1,iGhostBall)<200&&iEnemy_1.abnormal_type==TC_SPELLED_BY_NONE&&!iHero_2.b_is_spelling&&Tc_CanBeSpelled(hHero_1,hEnemy_1))
					{
						Tc_Spell(hHero_1,hEnemy_1);
					}
				}
				else
				{
					if(GetDisBall(iEnemy_2,iGhostBall)<200&&iEnemy_2.abnormal_type==TC_SPELLED_BY_NONE&&!iHero_2.b_is_spelling&&Tc_CanBeSpelled(hHero_1,hEnemy_2))
					{
						Tc_Spell(hHero_1,hEnemy_2);
					}
				}
			}
			break;
		}
	case GHOSTBALL_IN_SELF_TEAM:
		{
			if(iFreeBall.b_visible)
			{
				Tc_Move(hHeroWithBall,GetADirection(iHeroWithBall,iFreeBall,iAttackDirection));
			}
			else
			{
				Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			}
			Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHeroWithBall));
			if(iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE||iHeroWithBall.abnormal_type==TC_SPELLED_BY_RON)
			{
				if(iHeroWithoutBall.b_snatch_ghostball)
					Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
			}
			if(iAttackDirection==TC_DIRECTION_RIGHT)
			{
				if(iHeroWithBall.pos.x>iEnemy_1.pos.x+200&&iHeroWithBall.pos.x>iEnemy_2.pos.x+200)
				{
					if(iEnemy_1.b_can_spell&&GetDisHero(iHeroWithBall,iEnemy_1)<=GetDisHero(iHeroWithBall,iEnemy_2)&&iEnemy_1.abnormal_type==TC_SPELLED_BY_NONE)
					{
						if(Tc_CanBeSpelled(hHero_1,hEnemy_1))Tc_Spell(hHero_1,hEnemy_1);
					}
					if(iEnemy_2.b_can_spell&&GetDisHero(iHeroWithBall,iEnemy_1)>GetDisHero(iHeroWithBall,iEnemy_2)&&iEnemy_2.abnormal_type==TC_SPELLED_BY_NONE)
					{
						if(Tc_CanBeSpelled(hHero_1,hEnemy_2))Tc_Spell(hHero_1,hEnemy_2);
					}
				}
			}
			else
			{
				if(iHeroWithBall.pos.x<iEnemy_1.pos.x-200&&iHeroWithBall.pos.x-iEnemy_2.pos.x-200)
				{
					if(iEnemy_1.b_can_spell&&GetDisHero(iHeroWithBall,iEnemy_1)<=GetDisHero(iHeroWithBall,iEnemy_2)&&iEnemy_1.abnormal_type==TC_SPELLED_BY_NONE)
					{
						if(Tc_CanBeSpelled(hHero_1,hEnemy_1))Tc_Spell(hHero_1,hEnemy_1);
					}
					if(iEnemy_2.b_can_spell&&GetDisHero(iHeroWithBall,iEnemy_1)>GetDisHero(iHeroWithBall,iEnemy_2)&&iEnemy_2.abnormal_type==TC_SPELLED_BY_NONE)
					{
						if(Tc_CanBeSpelled(hHero_1,hEnemy_2))Tc_Spell(hHero_1,hEnemy_2);
					}
				}
			}
			if(iEnemy_1.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_GINNY)
			{
				if(iAttackDirection==TC_DIRECTION_RIGHT)
				{
					pos.x=iForbiddenArea.right.left+100;
					pos.y=iForbiddenArea.right.top-64;
					if((iEnemy_1.type==TC_HERO_GINNY&&iEnemy_1.b_is_spelling)||(iEnemy_2.type==TC_HERO_GINNY&&iEnemy_2.b_is_spelling))
					{
						if(iEnemy_1.pos.x+200<iHeroWithBall.pos.x&&iEnemy_2.pos.x+200<iHeroWithBall.pos.x)
						{
							pos.x=iForbiddenArea.right.left-65;
							pos.y=(iForbiddenArea.right.bottom+iForbiddenArea.right.top)/2;
						}
						if(iEnemy_1.pos.y>iHeroWithBall.pos.y&&iEnemy_2.pos.y>iHeroWithBall.pos.y)
						{
							pos.x=iForbiddenArea.right.left+100;
							pos.y=iForbiddenArea.right.top-64;
						}
						if(iEnemy_1.pos.y<iHeroWithBall.pos.y&&iEnemy_2.pos.y<iHeroWithBall.pos.y)
						{
							pos.x=iForbiddenArea.right.left+100;
							pos.y=iForbiddenArea.right.bottom+1;
						}
						Tc_PassBall(hHeroWithBall,pos);
						break;
					}
				}
				else
				{
					pos.x=iForbiddenArea.left.right+50;
					pos.y=iForbiddenArea.left.top-64;
					if((iEnemy_1.type==TC_HERO_GINNY&&iEnemy_1.b_is_spelling)||(iEnemy_2.type==TC_HERO_GINNY&&iEnemy_2.b_is_spelling))
					{
						if(iEnemy_1.pos.x-200>iHeroWithBall.pos.x&&iEnemy_2.pos.x-200>iHeroWithBall.pos.x)
						{
							pos.x=iForbiddenArea.left.right+1;
							pos.y=(iForbiddenArea.left.bottom+iForbiddenArea.left.top)/2;
						}
						if(iEnemy_1.pos.y>iHeroWithBall.pos.y&&iEnemy_2.pos.y>iHeroWithBall.pos.y)
						{
							pos.x=iForbiddenArea.left.right-100;
							pos.y=iForbiddenArea.left.top-64;
						}
						if(iEnemy_1.pos.y<iHeroWithBall.pos.y&&iEnemy_2.pos.y<iHeroWithBall.pos.y)
						{
							pos.x=iForbiddenArea.left.right-100;
							pos.y=iForbiddenArea.left.bottom+1;
						}
						Tc_PassBall(hHeroWithBall,pos);
						break;
					}
				}
			}
			if(iEnemy_1.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_RON)
			{
				if(iAttackDirection==TC_DIRECTION_RIGHT)
				{
					pos.x=iForbiddenArea.right.left+50;
					pos.y=iForbiddenArea.right.top-64;
					if((iEnemy_1.type==TC_HERO_RON&&iEnemy_1.b_is_spelling)||(iEnemy_2.type==TC_HERO_RON&&iEnemy_2.b_is_spelling))
					{
						if(iEnemy_1.pos.x+200<iHeroWithBall.pos.x&&iEnemy_2.pos.x+200<iHeroWithBall.pos.x)
						{
							pos.x=iForbiddenArea.right.left-65;
							pos.y=(iForbiddenArea.right.bottom+iForbiddenArea.right.top)/2;
						}
						if(iEnemy_1.pos.y>iHeroWithBall.pos.y&&iEnemy_2.pos.y>iHeroWithBall.pos.y)
						{
							pos.x=iForbiddenArea.right.left+100;
							pos.y=iForbiddenArea.right.top-64;
						}
						if(iEnemy_1.pos.y<iHeroWithBall.pos.y&&iEnemy_2.pos.y<iHeroWithBall.pos.y)
						{
							pos.x=iForbiddenArea.right.left+100;
							pos.y=iForbiddenArea.right.bottom+1;
						}
						Tc_PassBall(hHeroWithBall,pos);
						break;
					}
				}
				else
				{
					pos.x=iForbiddenArea.left.right+50;
					pos.y=iForbiddenArea.left.top-64;
					if((iEnemy_1.type==TC_HERO_RON&&iEnemy_1.b_is_spelling)||(iEnemy_2.type==TC_HERO_RON&&iEnemy_2.b_is_spelling))
					{
						if(iEnemy_1.pos.x-200>iHeroWithBall.pos.x&&iEnemy_2.pos.x-200>iHeroWithBall.pos.x)
						{
							pos.x=iForbiddenArea.left.right+1;
							pos.y=(iForbiddenArea.left.bottom+iForbiddenArea.left.top)/2;
						}
						if(iEnemy_1.pos.y>iHeroWithBall.pos.y&&iEnemy_2.pos.y>iHeroWithBall.pos.y)
						{
							pos.x=iForbiddenArea.left.right-100;
							pos.y=iForbiddenArea.left.top-64;
						}
						if(iEnemy_1.pos.y<iHeroWithBall.pos.y&&iEnemy_2.pos.y<iHeroWithBall.pos.y)
						{
							pos.x=iForbiddenArea.left.right-100;
							pos.y=iForbiddenArea.left.bottom+1;
						}
						Tc_PassBall(hHeroWithBall,pos);
						break;
					}
				}
			}
			Tc_PassBall(hHeroWithBall,GetPassPosition(iHeroWithBall,iHeroWithoutBall,iEnemy_1,iEnemy_2,iAttackDirection,iForbiddenArea));
			Tc_PassBall(hHeroWithBall,GetPassPosition(iHeroWithBall,iHeroWithoutBall,iEnemy_1,iEnemy_2,iAttackDirection,iForbiddenArea));
			break;
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
		{
			if(iHero_1.b_snatch_ghostball)
			{
				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
					break;
			}
			if(iHero_2.b_snatch_ghostball)
			{
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					break;
			}
			if(iAttackDirection==TC_DIRECTION_RIGHT)
			{
				pos.x=iGhostBall.pos.x-20;
				pos.y=iGhostBall.pos.y;
			}
			else
			{
				pos.x=iGhostBall.pos.x+20;
				pos.y=iGhostBall.pos.y;
			}
			if(iGhostBall.pos.x<iForbiddenArea.left.right||iGhostBall.pos.x>iForbiddenArea.right.left)
			{
				Tc_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
				Tc_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));
			}
			else
			{
				Tc_Move(hHero_1,GetPiontDirection(iHero_1,pos));
				Tc_Move(hHero_2,GetPiontDirection(iHero_2,pos));
			}
			//技能施放
			if(iAttackDirection==TC_DIRECTION_RIGHT)
			{
				if(iEnemyWithBall.pos.x<iForbiddenArea.left.right)
				{
					if(iEnemyWithBall.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_2,hEnemyWithBall)&&!iHero_1.b_is_spelling)
					{
						Tc_Spell(hHero_2,hEnemyWithBall);
						break;
					}
					if(iEnemyWithBall.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_1,hEnemyWithBall)&&!iHero_2.b_is_spelling)
					{
						Tc_Spell(hHero_1,hEnemyWithBall);
						break;
					}
				}
				if(iEnemyWithBall.pos.x<iHero_1.pos.x-200&&iEnemyWithBall.pos.x<iHero_2.pos.x-200)
				{
					if(iEnemyWithBall.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_1,hEnemyWithBall)&&!iHero_2.b_is_spelling)
					{
						Tc_Spell(hHero_1,hEnemyWithBall);
						break;
					}
				}
				if(iHero_2.curr_blue>1400)
				{
					if(iEnemyWithBall.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_2,hEnemyWithBall)&&!iHero_1.b_is_spelling)
						Tc_Spell(hHero_2,hEnemyWithBall);
					break;
				}
			}
			else
			{
				if(iEnemyWithBall.pos.x>iForbiddenArea.right.left)
				{
					if(iEnemyWithBall.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_2,hEnemyWithBall)&&!iHero_1.b_is_spelling)
					{
						Tc_Spell(hHero_2,hEnemyWithBall);
						break;
					}
					if(iEnemyWithBall.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_1,hEnemyWithBall)&&!iHero_2.b_is_spelling)
					{
						Tc_Spell(hHero_1,hEnemyWithBall);
						break;
					}
				}
				if(iEnemyWithBall.pos.x>iHero_1.pos.x+200&&iEnemyWithBall.pos.x>iHero_2.pos.x+200)
				{
					if(iEnemyWithBall.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_1,hEnemyWithBall)&&!iHero_2.b_is_spelling)
					{
						Tc_Spell(hHero_1,hEnemyWithBall);
						break;
					}
				}
				if(iHero_2.curr_blue>1400)
				{
					if(iEnemyWithBall.abnormal_type==TC_SPELLED_BY_NONE&&Tc_CanBeSpelled(hHero_2,hEnemyWithBall)&&!iHero_1.b_is_spelling)
						Tc_Spell(hHero_2,hEnemyWithBall);
					break;
				}
			}
			break;
		}
	}
	}
}