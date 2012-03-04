#include <Windows.h>
#include <tchar.h>
#include <math.h>
#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif.
#include "../inc/TuringCup9ClientAPI.h";
#pragma comment(lib, "../lib/TuringCup9.lib")
#define CLIENT_EXPORT
#include "Tc_Client.h"
CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(_T("water"));
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_HERMIONE,L"HERMIONE");
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"MALFOY");
}
#define NO_TEAM 0
#define SELF_TEAM 1
#define ENEMY_TEAM 2
TC_Handle hHero_1,hHero_2;
TC_Handle hEnemy_1,hEnemy_2;
TC_Handle hHeroWithBall,hHeroWithoutBall;
TC_Handle hEnemyWithBall,hEnemyWithoutBall;
TC_Handle hFreeBall,hGhostBall,hGoldBall;
TC_Ball iFreeBall,iGhostBall,iGoldBall;
TC_Hero iHero_1,iHero_2,iEnemy_1,iEnemy_2, iHeroWithBall, iHeroWithoutBall;
TC_Hero iEnemyWithBall, iEnemyWithoutBall;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_Position GhostPos,FreePos;
TC_Position passTarget,goal,bottom;
void getInfo()
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
}
void set()
{
	GhostPos.x=iGhostBall.pos.x+TC_BALL_WIDTH/2;
	GhostPos.y=iGhostBall.pos.y+TC_BALL_HEIGHT/2;
	FreePos.x=iFreeBall.pos.x+TC_BALL_WIDTH/2;
	FreePos.y=iFreeBall.pos.y+TC_BALL_HEIGHT/2;
	if(iOwnGate.x<iEnemyGate.x)
	{
		passTarget.x=iGhostBall.pos.x+512;
		if(passTarget.x+TC_BALL_WIDTH>=iForbiddenArea.right.left)
			passTarget.x=iForbiddenArea.right.left-TC_BALL_WIDTH-10;
		goal.x=2048;
	}
	if(iOwnGate.x>iEnemyGate.x)
	{
		goal.x=0;
		passTarget.x=iGhostBall.pos.x-512;
		if(passTarget.x<=iForbiddenArea.left.right)
			passTarget.x=iForbiddenArea.left.right+10;
	}	
	passTarget.y=iGhostBall.pos.y;
	goal.y=(iOwnGate.y_lower+iOwnGate.y_upper)/2;
	bottom.x=iHeroWithBall.pos.x;
	bottom.y=704;
}
int get_state()
{
	if (iHero_1.b_ghostball == false && iHero_2.b_ghostball == false && 
		iEnemy_1.b_ghostball == false && iEnemy_2.b_ghostball == false)
	{
		return NO_TEAM;
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
		return SELF_TEAM;
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
		return ENEMY_TEAM;
	}
}



double Distance(TC_Position pos_1,TC_Position pos_2)//两点之间的距离
{
	double xx,yy;
	xx=pos_1.x-pos_2.x;
	yy=pos_1.y-pos_2.y;
	return sqrt(yy*yy+xx*xx);
}
void add(int &expect,TC_Position s,TC_Position e,TC_Speed speed)
{
	expect=1;
	if((e.x-s.x)*speed.vx>0)
		expect++;
	if((e.y-s.y)*speed.vy>0)
		expect++;
	if(abs(e.x-s.x)<TC_SNATCH_DISTANCE_GHOST && speed.vx==0)
		expect++;
	if(abs(e.y-s.y)<TC_SNATCH_DISTANCE_GHOST && speed.vy==0)
		expect++;
}
bool canMove(TC_Position p1,TC_Position p2,TC_Speed speed)
{
	TC_Position heroCur,ballCur;
	bool direction=1;
	for(int i=1;i<30;i++)
	{
		heroCur.x=p1.x+i*speed.vx;
		heroCur.y=p1.y+i*speed.vy;
		if(direction==1)
		{
			ballCur.x=p2.x+i*iFreeBall.speed.vx;
			ballCur.y=p2.y+i*iFreeBall.speed.vy;
		}
		else
		{
			ballCur.x=p2.x-i*iFreeBall.speed.vx;
			ballCur.y=p2.y-i*iFreeBall.speed.vy;
		}
		if(Distance(heroCur,ballCur)<=TC_COLLIDE_WITH_FREEBALL)
			return false;
		if(Distance(heroCur,GhostPos)<=TC_SNATCH_DISTANCE_GHOST)
			break;
	}
	return true;
}
void Move(TC_Handle hHero,TC_Hero hero,TC_Position target)
{
	TC_Position HeroPos;
	TC_Speed speed;

	HeroPos.x=hero.pos.x+TC_HERO_WIDTH/2;
	HeroPos.y=hero.pos.y+TC_HERO_HEIGHT/2; 


	int expect[8],num,max,i;

	if(hero.b_ghostball==false)
	{
		speed.vx=0;
		speed.vy=7;
	}
	else
	{
		speed.vx=0;
		speed.vy=5;
	}
	add(expect[0],HeroPos,target,speed);
	if(iFreeBall.b_visible && !canMove(HeroPos,FreePos,speed))
		expect[0]=0;

	if(hero.b_ghostball==false)
	{
		speed.vx=-5;
		speed.vy=5;
	}
	else
	{
		speed.vx=-4;
		speed.vy=4;
	}
	add(expect[1],HeroPos,target,speed);
	if(iFreeBall.b_visible &&  !canMove(HeroPos,FreePos,speed))
		expect[1]=0;

	if(hero.b_ghostball==false)
	{
		speed.vx=-7;speed.vy=0;
	}
	else
	{
		speed.vx=-5;speed.vy=0;
	}
	add(expect[2],HeroPos,target,speed);
	if(iFreeBall.b_visible && !canMove(HeroPos,FreePos,speed))
		expect[2]=0;
	if(hero.b_ghostball==false)
	{
		speed.vx=-5;speed.vy=-5;
	}
	else
	{
		speed.vx=-4;speed.vy=-4;
	}
	add(expect[3],HeroPos,target,speed);
	if(iFreeBall.b_visible && !canMove(HeroPos,FreePos,speed))
		expect[3]=0;
	if(hero.b_ghostball==false)
	{
		speed.vx=0;speed.vy=-7;
	}
	else
	{
		speed.vx=0;speed.vy=-5;
	}
	add(expect[4],HeroPos,target,speed);
	if(iFreeBall.b_visible && !canMove(HeroPos,FreePos,speed))
		expect[4]=0;
	if(hero.b_ghostball==false)
	{
		speed.vx=5;speed.vy=-5;
	}
	else
	{
		speed.vx=4;speed.vy=-4;
	}
	add(expect[5],HeroPos,target,speed);
	if(iFreeBall.b_visible && !canMove(HeroPos,FreePos,speed))
		expect[5]=0;
	if(hero.b_ghostball==false)
	{
		speed.vx=7;speed.vy=0;
	}
	else
	{
		speed.vx=5;speed.vy=0;
	}
	add(expect[6],HeroPos,target,speed);
	if(iFreeBall.b_visible && !canMove(HeroPos,FreePos,speed))
		expect[6]=0;
	if(hero.b_ghostball==false)
	{
		speed.vx=5;speed.vy=5;
	}
	else
	{
		speed.vx=4;speed.vy=4;
	}
	add(expect[7],HeroPos,target,speed);
	if(iFreeBall.b_visible && !canMove(HeroPos,FreePos,speed))
		expect[7]=0;
	max=0;
	num=0;
	for(i=0;i<8;i++)
		if(max<expect[i])
		{
			max=expect[i];
			num=i;
		}
	Tc_Move(hHero,(TC_Direction)num);
}
bool canSpell(TC_Handle h,TC_Hero hero)
{
	bool inArea=0;
	if(iOwnGate.x<iEnemyGate.x && iEnemyWithBall.pos.x+TC_HERO_WIDTH<=iForbiddenArea.left.right)
		inArea=1;
	if(iOwnGate.x>iEnemyGate.x && iEnemyWithBall.pos.x>iForbiddenArea.right.left)
		inArea=1;
	if(inArea==0)
		return false;
	if(hero.b_can_spell==false)
		return false;
	if(Tc_CanBeSpelled(h,hEnemyWithBall)==false)
		return false;
	if(Distance(hero.pos,iEnemyWithBall.pos)>TC_HERO_SPELL_DISTANCE)
		return false;
	return true;
}
bool canPassBall()
{
	bool inArea=0;
	if(iOwnGate.x<iEnemyGate.x && iHeroWithBall.pos.x+TC_BALL_WIDTH<=iForbiddenArea.right.left-100)
		inArea=1;
	if(iOwnGate.x>iEnemyGate.x && iHeroWithBall.pos.x>=iForbiddenArea.left.right+100)
		inArea=1;
	if(inArea==0)
		return false;
	int x1,x2,y1,y2,k,x,y;
	x1=GhostPos.x;
	y1=GhostPos.y;
	x2=passTarget.x+TC_BALL_WIDTH/2;
	y2=passTarget.y+TC_BALL_HEIGHT/2;
	if(x1==x2)
		x2++;
	k=(y2-y1)/(x2-x1);
	if(iOwnGate.x<iEnemyGate.x)
	{
		x=iEnemy_1.pos.x+TC_HERO_WIDTH/2;	y=iEnemy_1.pos.y+TC_HERO_HEIGHT/2;
		if(x>iHeroWithBall.pos.x+TC_HERO_WIDTH/2+20 && abs(k*(x-x1)+y1-y)/sqrt(double(1)+k*k)<=100)
			return false;
		x=iEnemy_2.pos.x+TC_HERO_WIDTH/2;	y=iEnemy_2.pos.y+TC_HERO_HEIGHT/2;
		if(x>iHeroWithBall.pos.x+TC_HERO_WIDTH/2+20 && abs(k*(x-x1)+y1-y)/sqrt(double(1)+k*k)<=100)
			return false;
	}
	else
	{
		x=iEnemy_1.pos.x+TC_HERO_WIDTH/2;	y=iEnemy_1.pos.y+TC_HERO_HEIGHT/2;
		if(x<iHeroWithBall.pos.x+TC_HERO_WIDTH/2-20 && abs(k*(x-x1)+y1-y)/(sqrt(double(1)+k*k))<=100)
			return false;
		x=iEnemy_2.pos.x+TC_HERO_WIDTH/2;	y=iEnemy_2.pos.y+TC_HERO_HEIGHT/2;
		if(x<iHeroWithBall.pos.x+TC_HERO_WIDTH/2-20 && abs(k*(x-x1)+y1-y)/(sqrt(double(1)+k*k))<=100)
			return false;
	}
	return true;	
}
bool inOwnArea(TC_Hero hero)
{
	if(iOwnGate.x<iEnemyGate.x && hero.pos.x<iForbiddenArea.left.right)
		return true;
	if(iOwnGate.x>iEnemyGate.x && hero.pos.x+TC_HERO_WIDTH>iForbiddenArea.right.left)
		return true;
	return false;
}
CLIENT_EXPORT_API void __stdcall AI()
{
	getInfo();
	set();
	int state=get_state();
	switch(state)
	{
	case NO_TEAM:
		{
			if (iHero_1.b_snatch_ghostball && Tc_SnatchBall(hHero_1,TC_GHOST_BALL)) 
				;
			else
				Move(hHero_1,iHero_1,GhostPos);

			if (iHero_2.b_snatch_ghostball &&Tc_SnatchBall(hHero_2,TC_GHOST_BALL)) 
				;
			else
				Move(hHero_2,iHero_2,GhostPos);
			break;
		}
	case SELF_TEAM:
		{
			if((iEnemy_1.type==TC_HERO_GINNY && iEnemy_1.b_is_spelling)
				|| (iEnemy_2.type==TC_HERO_GINNY && iEnemy_2.b_is_spelling))
			{
				if(iOwnGate.x<iEnemyGate.x && 1848-iHeroWithBall.pos.x<21*5 ||
					iOwnGate.x>iEnemyGate.x && iHeroWithBall.pos.x+TC_HERO_WIDTH-200<21*5)
					Move(hHeroWithBall,iHeroWithBall,goal);
				else
					Tc_PassBall(hHeroWithBall,bottom);
				
				if((iEnemy_1.type==TC_HERO_RON && iEnemy_1.b_is_spelling || 
					iEnemy_2.type==TC_HERO_RON && iEnemy_2.b_is_spelling || 
					iHeroWithBall.abnormal_type!=TC_SPELLED_BY_NONE) 
					&& iHeroWithoutBall.b_snatch_ghostball && Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL))
					;
				else
					Move(hHeroWithoutBall,iHeroWithoutBall,GhostPos);
				break;
			}
			if(iHero_1.b_can_spell && iHero_2.b_can_spell)
			{
				if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_1)&& Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2))
				{
					if(Tc_Spell(hHeroWithBall,hEnemy_1))
						;
					else
						Move(hHeroWithBall,iHeroWithBall,goal);

					if(Tc_Spell(hHeroWithoutBall,hEnemy_2))
						;
					else
						Move(hHeroWithoutBall,iHeroWithoutBall,GhostPos);						
					break;
				}
				if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_2)&& Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_1))
				{
					if(Tc_Spell(hHeroWithBall,hEnemy_2))
						;
					else
						Move(hHeroWithBall,iHeroWithBall,goal);
					if(Tc_Spell(hHeroWithoutBall,hEnemy_1))
						;
					else
						Move(hHeroWithoutBall,iHeroWithoutBall,GhostPos);						
					break;
				}
			}
			if(canPassBall()==true && Tc_PassBall(hHeroWithBall,passTarget))
				;
			else
				Move(hHeroWithBall,iHeroWithBall,goal);

			if((iEnemy_1.type==TC_HERO_RON && iEnemy_1.b_is_spelling || 
				iEnemy_2.type==TC_HERO_RON && iEnemy_2.b_is_spelling || 
				iHeroWithBall.abnormal_type!=TC_SPELLED_BY_NONE) 
				&& iHeroWithoutBall.b_snatch_ghostball && Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL))
				;
			else
				Move(hHeroWithoutBall,iHeroWithoutBall,GhostPos);
			break;
		}
	case ENEMY_TEAM:
		{
			if(iHero_2.b_is_spelling == false && canSpell(hHero_1,iHero_1)==true &&  Tc_Spell(hHero_1,hEnemyWithBall))
			{
				if(iHero_2.b_snatch_ghostball &&Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					;
				else
					Move(hHero_2,iHero_2,GhostPos);
			}
			else
			{
				if(iHero_1.b_is_spelling ==false && canSpell(hHero_2,iHero_2)==true && Tc_Spell(hHero_2,hEnemyWithBall))
					;
				else
				{
					if(iHero_2.b_snatch_ghostball &&Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
						;
					else
						Move(hHero_2,iHero_2,GhostPos);
				}
				if(iHero_1.b_snatch_ghostball && Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
					;
				else
					Move(hHero_1,iHero_1,GhostPos);
			}
			break;
		}
	}
}