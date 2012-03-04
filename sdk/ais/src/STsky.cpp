/*
队名：STsky
队员：窦煜峰  陶翔  沈宇亮
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

Copy "$(ProjectDir)TuringCup9.exe" 
to the Command field.(Without quotation marks)
*/

CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(_T("STsky"));
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"OO");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_GINNY,L"XX");
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
TC_Position  pos_target,pos1,pos2;
double dis1 ,dis2;
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
int diss(int x1,int y1,int x2,int y2)
{
	return int(sqrt(double((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))));
}

TC_Direction GetDirection(TC_Hero *pHero,int x,int y)
{
	if (abs(pHero->pos.x+TC_HERO_WIDTH/2-x)<7 && pHero->pos.y+TC_HERO_HEIGHT/2 > y)return TC_DIRECTION_TOP;
	else if (pHero->pos.x+TC_HERO_WIDTH/2 < x && abs(pHero->pos.y+TC_HERO_HEIGHT/2-y)<7)return TC_DIRECTION_RIGHT;
	else if (abs(pHero->pos.x+TC_HERO_WIDTH/2-x)<7 && pHero->pos.y+TC_HERO_HEIGHT/2 < y)return TC_DIRECTION_BOTTOM;
	else if (pHero->pos.x+TC_HERO_WIDTH/2 > x && abs(pHero->pos.y+TC_HERO_HEIGHT/2-y)<7)return TC_DIRECTION_LEFT;
	else if (pHero->pos.x+TC_HERO_WIDTH/2 > x && pHero->pos.y+TC_HERO_HEIGHT/2 > y)return TC_DIRECTION_LEFTTOP;
	else if (pHero->pos.x+TC_HERO_WIDTH/2 < x && pHero->pos.y+TC_HERO_HEIGHT/2 > y)return TC_DIRECTION_RIGHTTOP;
	else if (pHero->pos.x+TC_HERO_WIDTH/2 < x && pHero->pos.y+TC_HERO_HEIGHT/2 < y)return TC_DIRECTION_RIGHTBOTTOM;
	else return TC_DIRECTION_LEFTBOTTOM;
}


TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pEnemy)
{
	int x=pEnemy->pos.x+TC_HERO_WIDTH/2;
	int y=pEnemy->pos.y+TC_HERO_HEIGHT/2;
	return GetDirection(pHero,x,y);
}
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)
{
	int x=pBall->pos.x+TC_BALL_WIDTH/2;
	int y=pBall->pos.y+TC_BALL_HEIGHT/2;
	return GetDirection(pHero,x,y);
}

TC_Direction GetAttackDirection(TC_Hero *pHero)
{
	if (pHero->pos.y >= iEnemyGate.y_lower-TC_HERO_HEIGHT-2)
	{
		return GetDirection(pHero,pHero->pos.x+TC_HERO_WIDTH/2,pHero->pos.y-20+TC_HERO_HEIGHT/2);
	}
	else
	{
		if (pHero->pos.y <= iEnemyGate.y_upper+2)
		{
			return GetDirection(pHero,pHero->pos.x+TC_HERO_WIDTH/2,pHero->pos.y+20+TC_HERO_HEIGHT/2);
		}
		else
		{
			if (iAttackDirection==TC_DIRECTION_RIGHT)
				return GetDirection(pHero,TC_MAP_WIDTH,pHero->pos.y+TC_HERO_HEIGHT/2);
			else
				return GetDirection(pHero,0,pHero->pos.y+TC_HERO_HEIGHT/2);
		}
	}
}


TC_Position nextPosition(TC_Position ret,TC_Direction d,int ONESTEP)
{//此ret下在d方向下下一步位置，ONESTEP是英雄的只有水平速度时的绝对值
	int add;
	if(ONESTEP==5)
		add=1;
	else add=2;
	switch (d)
	{
	case TC_DIRECTION_BOTTOM:
		ret.y+=ONESTEP;
		break;
	case TC_DIRECTION_LEFTBOTTOM:
		ret.y+=ONESTEP-add;
		ret.x-=ONESTEP-add;
		break;
	case TC_DIRECTION_LEFT:
		ret.x-=ONESTEP;
		break;
	case TC_DIRECTION_LEFTTOP:
		ret.x-=ONESTEP-add;
		ret.y-=ONESTEP-add;
		break;
	case TC_DIRECTION_TOP:
		ret.y-=ONESTEP;
		break;
	case TC_DIRECTION_RIGHTTOP:
		ret.y-=ONESTEP-add;
		ret.x+=ONESTEP-add;
		break;
	case TC_DIRECTION_RIGHT:
		ret.x+=ONESTEP;
		break;
	case TC_DIRECTION_RIGHTBOTTOM:
		ret.x+=ONESTEP-add;
		ret.y+=ONESTEP-add;
		break;
	};
	return ret;
}
TC_Direction DIR[8]={
	TC_DIRECTION_BOTTOM,
	TC_DIRECTION_LEFTBOTTOM,
	TC_DIRECTION_LEFT,
	TC_DIRECTION_LEFTTOP,
	TC_DIRECTION_TOP,
	TC_DIRECTION_RIGHTTOP,
	TC_DIRECTION_RIGHT,
	TC_DIRECTION_RIGHTBOTTOM
};
int vdir[8][2]={0,1,-1,1,-1,0,-1,-1,0,-1,1,-1,1,0,1,1};
bool is_can_be_peng(TC_Hero * phero,TC_Direction d,int OFFS,int ONESTEP)
{

	TC_Position hp=phero->pos;
	TC_Position bp=iFreeBall.pos;
	TC_Speed sp=iFreeBall.speed;
	int hx,hy,bx,by;
	int i;
	for(i=1;i<=OFFS;i++)
	{
		if(i>iFreeBall.u.moving_time_left) return false;//球如果消失不用判断了
		hp=nextPosition(hp,d,ONESTEP);
		bp.x+=sp.vx;
		bp.y+=sp.vy;
		if((bp.y-iFreeBall.u.path.pos_start.y)*(bp.y-iFreeBall.u.path.pos_end.y)<0 && (bp.x-iFreeBall.u.path.pos_start.x)*(bp.x-iFreeBall.u.path.pos_end.x)<0 )
		{
		}
		else {
			sp.vx=-sp.vx;
			sp.vy=-sp.vy;
		}
		hx=hp.x+TC_HERO_WIDTH/2;
		hy=hp.y+TC_HERO_HEIGHT/2;
		bx=bp.x+TC_BALL_WIDTH/2;
		by=bp.y+TC_BALL_HEIGHT/2;
		double dis=sqrt(double( (hx-bx)*(hx-bx)+(hy-by)*(hy-by) ));
		if(dis<65)//会相撞
			break;
	}
	return i<=OFFS;
}
bool Move(TC_Handle h,TC_Direction d)//安全移动
{
	int ONESTEP;//只有水平速度时的的绝对值
	int OFFS;//预判步
	TC_Hero hero;
	Tc_GetHeroInfo(h,&hero);
	if(hero.b_ghostball)
	{
		ONESTEP=5;
		OFFS=20;
	}
	else 
	{
		ONESTEP=7;
		OFFS=20;
	}
	if(iFreeBall.b_visible==false||(!is_can_be_peng(&hero,d,OFFS,ONESTEP)))//当前不会碰
		return Tc_Move(h,d);
	TC_Direction md1,md2;
	for(int i=1;i<=4;++i)
	{
		md1=DIR[(d+i)%8];
		md2=DIR[(d+8-i)%8];
		if(!is_can_be_peng(&hero,md1,OFFS,ONESTEP))//不被砸到
		{
			if(is_can_be_peng(&hero,md2,OFFS,ONESTEP))//会砸到
				return Tc_Move(h,md1);

			if(vdir[md1][0]*iFreeBall.speed.vx<0||vdir[md1][1]*iFreeBall.speed.vy<0)
				return Tc_Move(h,md1);//md1与自由球反反向
			else 
				return Tc_Move(h,md2);
		}

		if(!is_can_be_peng(&hero,md2,OFFS,ONESTEP))//第一个会被砸到，第二个不会
			return Tc_Move(h,md2);	
	}
	return Tc_Move(h,d);//必被砸到
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
	bool t=iGoldBall.b_visible;
	int d1=0,d2=0,k1,k2,d3=100000,d4=100000;
	d1=diss(iEnemy_1.pos.x,iEnemy_1.pos.y,iGhostBall.pos.x,iGhostBall.pos.y);
	d2=diss(iEnemy_2.pos.x,iEnemy_2.pos.y,iGhostBall.pos.x,iGhostBall.pos.y);
	d3=diss(iEnemy_1.pos.x,iEnemy_1.pos.y,iGoldBall.pos.x,iGoldBall.pos.y);
	d4=diss(iEnemy_2.pos.x,iEnemy_2.pos.y,iGoldBall.pos.x,iGoldBall.pos.y);
	k1=diss(iHero_1.pos.x,iHero_1.pos.y,iEnemy_1.pos.x,iEnemy_1.pos.y);
	k2=diss(iHero_1.pos.x,iHero_1.pos.y,iEnemy_2.pos.x,iEnemy_2.pos.y);
	if (t==true)
	{
		if (iHero_1.b_snatch_goldball==true)
		{
			Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
		}
		if (iHero_2.b_snatch_goldball==true)
		{
			Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
		}
	}
	if (t==true&&d1>512&&d2>512)
	{
		if(iHero_1.b_ghostball==true) 
		{
			pos2.x=iHero_1.pos.x+10;
			pos2.y=iHero_1.pos.y+10;
			Tc_PassBall(hHero_1,pos2);
		}
		else switch (state)
		{
		case GHOSTBALL_IN_SELF_TEAM:
			{
				int dis,d11,d22;
				int ddiiss=2000;
				if(iAttackDirection == TC_DIRECTION_RIGHT)  //向右攻
				{
					if(iForbiddenArea.right.left>iHeroWithBall.pos.x+TC_BALL_WIDTH+20)//不在禁区内
					{
						int dd1=0,dd2=0;
						if(iGhostBall.pos.x>iEnemy_1.pos.x||iEnemy_1.b_snatch_ghostball==false) dd1=1;
						if(iGhostBall.pos.x>iEnemy_2.pos.x||iEnemy_2.b_snatch_ghostball==false) dd2=1;
						if(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2<iEnemyGate.y_upper)//在上方
						{
							d11=iForbiddenArea.right.left-20-iHeroWithBall.pos.x-TC_BALL_WIDTH;
							d22=iEnemyGate.y_upper+70-iHeroWithBall.pos.y-TC_BALL_HEIGHT/2;
							dis=int(sqrt(double(d11*d11+d22*d22)));
							if(dis<TC_PASSBALL_DISTANCE)   //不能传最大距离
							{
								pos_target.x=iForbiddenArea.right.left-20;
								pos_target.y=iEnemyGate.y_upper+70;
								Tc_PassBall(hHeroWithBall,pos_target);
							}
							else  //能穿最大距离
							{
								if(dd1==1&&dd2==1)
								{
									if(iHeroWithBall.pos.x+TC_BALL_WIDTH+TC_PASSBALL_DISTANCE<iForbiddenArea.right.left-20)
										pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+TC_PASSBALL_DISTANCE;
									else pos_target.x=iForbiddenArea.right.left-20;
									pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
									Tc_PassBall(hHeroWithBall,pos_target);
								}
								else{
									if(dd1==0&&d1>100) {ddiiss=d1/2;}
									if(dd2==0&&d2>100) {if(d2/2<ddiiss) ddiiss=d2/2;}
									if(ddiiss<2000)
									{
										if(iHeroWithBall.pos.x+TC_BALL_WIDTH+ddiiss<iForbiddenArea.right.left-20)
											pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+ddiiss;
										else pos_target.x=iForbiddenArea.right.left-20;
										pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
										Tc_PassBall(hHeroWithBall,pos_target);
									}
								}
								if(ddiiss==2000)  Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
							}
						}
						else if(iHeroWithBall.pos.y>iEnemyGate.y_lower)   //在下方
						{
							d11=iForbiddenArea.right.left-20-iHeroWithBall.pos.x-TC_BALL_WIDTH;
							d22=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iEnemyGate.y_lower+70;
							dis=int(sqrt(double(d11*d11+d22*d22)));
							if(dis<TC_PASSBALL_DISTANCE)   //不能传最大距离
							{
								pos_target.x=iForbiddenArea.right.left-20;
								pos_target.y=iEnemyGate.y_lower-70;
								Tc_PassBall(hHeroWithBall,pos_target);
							}
							else  //能穿最大距离
							{
								if(dd1==1&&dd2==1)
								{
									if(iHeroWithBall.pos.x+TC_BALL_WIDTH+TC_PASSBALL_DISTANCE<iForbiddenArea.right.left-20)
										pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+TC_PASSBALL_DISTANCE;
									else pos_target.x=iForbiddenArea.right.left-20;
									pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
									Tc_PassBall(hHeroWithBall,pos_target);
								}
								else{
									if(dd1==0&&d1>100) {ddiiss=d1/2;}
									if(dd2==0&&d2>100) {if(d2/2<ddiiss) ddiiss=d2/2;}
									if(ddiiss<2000)
									{
										if(iHeroWithBall.pos.x+TC_BALL_WIDTH+ddiiss<iForbiddenArea.right.left-20)
											pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+ddiiss;
										else pos_target.x=iForbiddenArea.right.left-20;
										pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
										Tc_PassBall(hHeroWithBall,pos_target);
									}
								}
								if(ddiiss==2000)  Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
							}
						}
						else   //在中间
						{
							if(iHeroWithBall.pos.x+TC_BALL_WIDTH+TC_PASSBALL_DISTANCE<iForbiddenArea.right.left-20)
								pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+TC_PASSBALL_DISTANCE;
							else pos_target.x=iForbiddenArea.right.left-20;
							pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
							Tc_PassBall(hHeroWithBall,pos_target);
						}
					}
					else   //在禁区内
					{
						double m1,m2,m3,m4,c1;
						m1=abs(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.right.top+50);
						m2=abs(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.right.bottom-40);
						m3=abs(iHeroWithoutBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.right.top+50);
						m4=abs(iHeroWithoutBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.right.bottom-40);
						pos1.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2;
						pos1.y=iForbiddenArea.right.top-50;
						c1=m1;
						if(c1>m2)
						{
							pos1.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2;
							pos1.y=iForbiddenArea.right.bottom+40;
							c1=m2;
						}
						if(iEnemy_1.b_is_spelling==true)
						{
							if(iEnemy_1.type==TC_HERO_GINNY||iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_MALFOY||iEnemy_1.type==TC_HERO_HERMIONE)
							{
								Tc_PassBall(hHeroWithBall,pos1);					
							}
						}
						if(iEnemy_2.b_is_spelling==true)
						{
							if(iEnemy_2.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_MALFOY||iEnemy_2.type==TC_HERO_HERMIONE)
							{
								Tc_PassBall(hHeroWithBall,pos1);	//							
							}
						}
						if(iEnemy_1.b_is_spelling==false&&iEnemy_2.b_is_spelling==false) Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
					}
				}
				else   //向左攻
				{
					if(iForbiddenArea.left.right<iHeroWithBall.pos.x-20)//不在禁区内
					{
						int dd1=0,dd2=0;
						if(iGhostBall.pos.x<iEnemy_1.pos.x||iEnemy_1.b_snatch_ghostball==false) dd1=1;
						if(iGhostBall.pos.x<iEnemy_2.pos.x||iEnemy_2.b_snatch_ghostball==false) dd2=1;
						if(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2<iEnemyGate.y_upper)//在上方
						{
							d11=iForbiddenArea.left.right+20-iHeroWithBall.pos.x;
							d22=iEnemyGate.y_upper+70-iHeroWithBall.pos.y-TC_BALL_HEIGHT/2;
							dis=int(sqrt(double(d11*d11+d22*d22)));
							if(dis<TC_PASSBALL_DISTANCE)   //不能传最大距离
							{
								pos_target.x=iForbiddenArea.left.right+20;
								pos_target.y=iEnemyGate.y_upper+70;
								Tc_PassBall(hHeroWithBall,pos_target);
							}
							else  //能穿最大距离
							{
								if(dd1==1&&dd2==1)
								{
									if(iHeroWithBall.pos.x-TC_PASSBALL_DISTANCE<iForbiddenArea.left.right+20)
										pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-TC_PASSBALL_DISTANCE;
									else pos_target.x=iForbiddenArea.left.right+20;
									pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
									Tc_PassBall(hHeroWithBall,pos_target);
								}
								else{
									if(dd1==0&&d1>100) {ddiiss=d1/2;}
									if(dd2==0&&d2>100) {if(d2/2<ddiiss) ddiiss=d2/2;}
									if(ddiiss<2000)
									{
										if(iHeroWithBall.pos.x-ddiiss<iForbiddenArea.left.right+20)
											pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-ddiiss;
										else pos_target.x=iForbiddenArea.left.right+20;
										pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
										Tc_PassBall(hHeroWithBall,pos_target);
									}
								}
								if(ddiiss==2000)  Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
							}
						}
						else if(iHeroWithBall.pos.y>iEnemyGate.y_lower)   //在下方
						{
							d11=iForbiddenArea.left.right+20-iHeroWithBall.pos.x;
							d22=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iEnemyGate.y_lower+70;
							dis=int(sqrt(double(d11*d11+d22*d22)));
							if(dis<TC_PASSBALL_DISTANCE)   //不能传最大距离
							{
								pos_target.x=iForbiddenArea.left.right+20;
								pos_target.y=iEnemyGate.y_lower-70;
								Tc_PassBall(hHeroWithBall,pos_target);
							}
							else  //能穿最大距离
							{
								if(dd1==1&&dd2==1)
								{
									if(iHeroWithBall.pos.x-TC_PASSBALL_DISTANCE<iForbiddenArea.left.right+20)
										pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-TC_PASSBALL_DISTANCE;
									else pos_target.x=iForbiddenArea.left.right+20;
									pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
									Tc_PassBall(hHeroWithBall,pos_target);
								}
								else{
									if(dd1==0&&d1>100) {ddiiss=d1/2;}
									if(dd2==0&&d2>100) {if(d2/2<ddiiss) ddiiss=d2/2;}
									if(ddiiss<2000)
									{
										if(iHeroWithBall.pos.x-ddiiss<iForbiddenArea.left.right+20)
											pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-ddiiss;
										else pos_target.x=iForbiddenArea.left.right+20;
										pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
										Tc_PassBall(hHeroWithBall,pos_target);
									}
								}
								if(ddiiss==2000)  Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
							}
						}
						else   //在中间
						{
							if(iHeroWithBall.pos.x-TC_PASSBALL_DISTANCE>iForbiddenArea.left.right+20)
								pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-TC_PASSBALL_DISTANCE;
							else pos_target.x=iForbiddenArea.left.right+20;
							pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
							Tc_PassBall(hHeroWithBall,pos_target);
						}
					}
					else   //在禁区内
					{
						double m1,m2,m3,m4,c1;
						m1=abs(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.left.top+70);
						m2=abs(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.left.bottom-50);
						m3=abs(iHeroWithoutBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.left.top+70);
						m4=abs(iHeroWithoutBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.left.bottom-50);
						pos1.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2;
						pos1.y=iForbiddenArea.left.top-70;
						c1=m1;
						if(c1>m2)
						{
							pos1.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2;
							pos1.y=iForbiddenArea.left.bottom+50;
							c1=m2;
						}
						if(iEnemy_1.b_is_spelling==true)
						{
							if(iEnemy_1.type==TC_HERO_GINNY||iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_MALFOY||iEnemy_1.type==TC_HERO_HERMIONE)
							{
								Tc_PassBall(hHeroWithBall,pos1);
							}
						}
						if(iEnemy_2.b_is_spelling==true)
						{
							if(iEnemy_2.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_MALFOY||iEnemy_2.type==TC_HERO_HERMIONE)
							{
								Tc_PassBall(hHeroWithBall,pos1);
							}
						}
						if(iEnemy_1.b_is_spelling==false&&iEnemy_2.b_is_spelling==false) Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
					}
				}
				int l=0;
				if (iHero_1.b_can_spell==true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iEnemy_1.pos.x+64,iEnemy_1.pos.y+64)<500)
				{
					l=1;
					Tc_Spell(hHero_1,hEnemy_1);
				}
				if (l==0&&iHero_1.b_can_spell==true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iEnemy_2.pos.x+64,iEnemy_2.pos.y+64)<500)
				{
					l=2;
					Tc_Spell(hHero_1,hEnemy_2);
				}
				if (l==0)
					Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemy_1));
				break;
			}
		case GHOSTBALL_IN_NO_TEAM:
			{
				if (iHero_2.b_snatch_ghostball==true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<60)
				{
					Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				}
				else
					Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				if (k1<k2)
				{
					if (iHero_1.b_can_spell==true&&k1<500)
					{
						Tc_Spell(hHero_1,hEnemy_1);
					}
					else
						Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemy_1));
				}
				else 
				{
					if (iHero_1.b_can_spell==true&&k2<500)
					{
						Tc_Spell(hHero_1,hEnemy_2);
					}
					else
						Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemy_2));
				}
				break;
			}
		}

	}
	else
	{
		int l=0;
		if (t==true&&(d3<500||d4<500))
		{
			if(iHero_1.b_ghostball==true) 
			{
				pos2.x=iHero_2.pos.x+10;
				pos2.y=iHero_2.pos.y+10;
				Tc_PassBall(hHero_1,pos2);
			}
			else{
				if (iHero_1.b_can_spell==true&&k1<500)
				{
					l=1;
					Tc_Spell(hHero_1,hEnemy_1);
				}
				if (iHero_1.b_can_spell==true&&k2<500)
				{
					l=1;
					Tc_Spell(hHero_1,hEnemy_2);
				}
			}
		}
		if (l==0)
			switch (state)
		{
			case GHOSTBALL_IN_SELF_TEAM:
				{
					int dis,d11,d22;
					int ddiiss=2000;
					//Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
					if(iAttackDirection == TC_DIRECTION_RIGHT)  //向右攻
					{
						if(iHeroWithBall.pos.x>iForbiddenArea.right.left-50)     
						{
							if(iEnemy_1.b_is_spelling==false&&iHero_1.b_can_spell==true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iEnemy_1.pos.x+64,iEnemy_1.pos.y+64)<500)
							{
								Tc_Spell(hHero_1,hEnemy_1);					
							}
							else if(iEnemy_2.b_is_spelling==false&&iHero_1.b_can_spell==true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iEnemy_2.pos.x+64,iEnemy_2.pos.y+64)<500)
							{
								Tc_Spell(hHero_1,hEnemy_2);
							}
							//else Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
							//if((iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE||iEnemy_1.b_is_spelling==true||iEnemy_2.b_is_spelling==true)&&iHeroWithoutBall.b_snatch_ghostball==true&&diss(iHeroWithoutBall.pos.x+64,iHeroWithoutBall.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<60)  Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						}
						if(((d1<60&&d2<60&&iEnemy_1.b_snatch_ghostball==true&&iEnemy_2.b_snatch_ghostball==true)||iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE||iEnemy_1.b_is_spelling==true||iEnemy_2.b_is_spelling==true)&&iHeroWithoutBall.b_snatch_ghostball==true&&diss(iHeroWithoutBall.pos.x+64,iHeroWithoutBall.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<60)  Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						else Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));

						if(iForbiddenArea.right.left>iHeroWithBall.pos.x+TC_BALL_WIDTH+20)//不在禁区内
						{
							int dd1=0,dd2=0;
							if(iGhostBall.pos.x+32>iEnemy_1.pos.x+64||iEnemy_1.b_snatch_ghostball==false) dd1=1;
							if(iGhostBall.pos.x+32>iEnemy_2.pos.x+64||iEnemy_2.b_snatch_ghostball==false) dd2=1;
							if(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2<iEnemyGate.y_upper)//在上方
							{
								d11=iForbiddenArea.right.left-20-iHeroWithBall.pos.x-TC_BALL_WIDTH;
								d22=iEnemyGate.y_upper+70-iHeroWithBall.pos.y-TC_BALL_HEIGHT/2;
								dis=int(sqrt(double(d11*d11+d22*d22)));
								if(dis<TC_PASSBALL_DISTANCE)   //不能传最大距离
								{
									pos_target.x=iForbiddenArea.right.left-20;
									pos_target.y=iEnemyGate.y_upper+70;
									Tc_PassBall(hHeroWithBall,pos_target);
								}
								else  //能穿最大距离
								{
									if(dd1==1&&dd2==1)
									{
										if(iHeroWithBall.pos.x+TC_BALL_WIDTH+TC_PASSBALL_DISTANCE<iForbiddenArea.right.left-20)
											pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+TC_PASSBALL_DISTANCE;
										else pos_target.x=iForbiddenArea.right.left-20;
										pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
										Tc_PassBall(hHeroWithBall,pos_target);
									}
									else{
										if(dd1==0&&d1>100) {ddiiss=d1/2;}
										if(dd2==0&&d2>100) {if(d2/2<ddiiss) ddiiss=d2/2;}
										if(ddiiss<2000)
										{
											if(iHeroWithBall.pos.x+TC_BALL_WIDTH+ddiiss<iForbiddenArea.right.left-20)
												pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+ddiiss;
											else pos_target.x=iForbiddenArea.right.left-20;
											pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
											Tc_PassBall(hHeroWithBall,pos_target);
										}
									}
									if(ddiiss==2000)  Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
								}
							}
							else if(iHeroWithBall.pos.y>iEnemyGate.y_lower)   //在下方
							{
								d11=iForbiddenArea.right.left-20-iHeroWithBall.pos.x-TC_BALL_WIDTH;
								d22=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iEnemyGate.y_lower+70;
								dis=int(sqrt(double(d11*d11+d22*d22)));
								if(dis<TC_PASSBALL_DISTANCE)   //不能传最大距离
								{
									pos_target.x=iForbiddenArea.right.left-20;
									pos_target.y=iEnemyGate.y_lower-70;
									Tc_PassBall(hHeroWithBall,pos_target);
								}
								else  //能穿最大距离
								{
									if(dd1==1&&dd2==1)
									{
										if(iHeroWithBall.pos.x+TC_BALL_WIDTH+TC_PASSBALL_DISTANCE<iForbiddenArea.right.left-20)
											pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+TC_PASSBALL_DISTANCE;
										else pos_target.x=iForbiddenArea.right.left-20;
										pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
										Tc_PassBall(hHeroWithBall,pos_target);
									}
									else{
										if(dd1==0&&d1>100) {ddiiss=d1/2;}
										if(dd2==0&&d2>100) {if(d2/2<ddiiss) ddiiss=d2/2;}
										if(ddiiss<2000)
										{
											if(iHeroWithBall.pos.x+TC_BALL_WIDTH+ddiiss<iForbiddenArea.right.left-20)
												pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+ddiiss;
											else pos_target.x=iForbiddenArea.right.left-20;
											pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
											Tc_PassBall(hHeroWithBall,pos_target);
										}
									}
									if(ddiiss==2000)  Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
								}
							}
							else   //在中间
							{
								if(iHeroWithBall.pos.x+TC_BALL_WIDTH+TC_PASSBALL_DISTANCE<iForbiddenArea.right.left-20)
									pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2+TC_PASSBALL_DISTANCE;
								else pos_target.x=iForbiddenArea.right.left-20;
								pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
								Tc_PassBall(hHeroWithBall,pos_target);
							}
						}
						else   //在禁区内
						{
							double m1,m2,m3,m4,c1;
							m1=abs(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.right.top+50);
							m2=abs(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.right.bottom-40);
							m3=abs(iHeroWithoutBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.right.top+50);
							m4=abs(iHeroWithoutBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.right.bottom-40);
							pos1.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2;
							pos1.y=iForbiddenArea.right.top-50;
							c1=m1;
							if(c1>m2)
							{
								pos1.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2;
								pos1.y=iForbiddenArea.right.bottom+40;
								c1=m2;
							}

							if(iEnemy_1.b_is_spelling==true)
							{
								if(iEnemy_1.type==TC_HERO_GINNY||iEnemy_1.type==TC_HERO_RON)
								{
									Tc_PassBall(hHeroWithBall,pos1);
								}
							}
							if(iEnemy_2.b_is_spelling==true)
							{
								if(iEnemy_2.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_RON)
								{
									Tc_PassBall(hHeroWithBall,pos1);
								}
							}
							if(iEnemy_1.b_is_spelling==false&&iEnemy_2.b_is_spelling==false) Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
						}
					}
					else   //向左攻
					{
						if(iHeroWithoutBall.pos.x<iForbiddenArea.left.right+60) 
						{
							if(iEnemy_1.b_is_spelling==false&&iHero_1.b_can_spell==true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iEnemy_1.pos.x+64,iEnemy_1.pos.y+64)<500)
							{
								Tc_Spell(hHero_1,hEnemy_1);					
							}
							else if(iEnemy_2.b_is_spelling==false&&iHero_1.b_can_spell==true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iEnemy_2.pos.x+64,iEnemy_2.pos.y+64)<500)
							{
								Tc_Spell(hHero_1,hEnemy_2);
							}
							//else Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
							//if((iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE||iEnemy_1.b_is_spelling==true||iEnemy_2.b_is_spelling==true)&&iHeroWithoutBall.b_snatch_ghostball==true&&diss(iHeroWithoutBall.pos.x+64,iHeroWithoutBall.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<60)  Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						}
						if(((d1<60&&d2<60&&iEnemy_1.b_snatch_ghostball==true&&iEnemy_2.b_snatch_ghostball==true)||iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE||iEnemy_1.b_is_spelling==true||iEnemy_2.b_is_spelling==true)&&iHeroWithoutBall.b_snatch_ghostball==true&&diss(iHeroWithoutBall.pos.x+64,iHeroWithoutBall.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<60)  Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						else Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
						if(iForbiddenArea.left.right<iHeroWithBall.pos.x-20)//不在禁区内
						{
							int dd1=0,dd2=0;
							if(iGhostBall.pos.x+32<iEnemy_1.pos.x+64||iEnemy_1.b_snatch_ghostball==false) dd1=1;
							if(iGhostBall.pos.x+32<iEnemy_2.pos.x+64||iEnemy_2.b_snatch_ghostball==false) dd2=1;
							if(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2<iEnemyGate.y_upper)//在上方
							{
								d11=iForbiddenArea.left.right+20-iHeroWithBall.pos.x;
								d22=iEnemyGate.y_upper+70-iHeroWithBall.pos.y-TC_BALL_HEIGHT/2;
								dis=int(sqrt(double(d11*d11+d22*d22)));
								if(dis<TC_PASSBALL_DISTANCE)   //不能传最大距离
								{
									pos_target.x=iForbiddenArea.left.right+20;
									pos_target.y=iEnemyGate.y_upper+70;
									Tc_PassBall(hHeroWithBall,pos_target);
								}
								else  //能穿最大距离
								{
									if(dd1==1&&dd2==1)
									{
										if(iHeroWithBall.pos.x-TC_PASSBALL_DISTANCE<iForbiddenArea.left.right+20)
											pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-TC_PASSBALL_DISTANCE;
										else pos_target.x=iForbiddenArea.left.right+20;
										pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
										Tc_PassBall(hHeroWithBall,pos_target);
									}
									else{
										if(dd1==0&&d1>100) {ddiiss=d1/2;}
										if(dd2==0&&d2>100) {if(d2/2<ddiiss) ddiiss=d2/2;}
										if(ddiiss<2000)
										{
											if(iHeroWithBall.pos.x-ddiiss<iForbiddenArea.left.right+20)
												pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-ddiiss;
											else pos_target.x=iForbiddenArea.left.right+20;
											pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
											Tc_PassBall(hHeroWithBall,pos_target);
										}
									}
									if(ddiiss==2000)  Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
								}
							}
							else if(iHeroWithBall.pos.y>iEnemyGate.y_lower)   //在下方
							{
								d11=iForbiddenArea.left.right+20-iHeroWithBall.pos.x;
								d22=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iEnemyGate.y_lower+70;
								dis=int(sqrt(double(d11*d11+d22*d22)));
								if(dis<TC_PASSBALL_DISTANCE)   //不能传最大距离
								{
									pos_target.x=iForbiddenArea.left.right+20;
									pos_target.y=iEnemyGate.y_lower-70;
									Tc_PassBall(hHeroWithBall,pos_target);
								}
								else  //能穿最大距离
								{
									if(dd1==1&&dd2==1)
									{
										if(iHeroWithBall.pos.x-TC_PASSBALL_DISTANCE<iForbiddenArea.left.right+20)
											pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-TC_PASSBALL_DISTANCE;
										else pos_target.x=iForbiddenArea.left.right+20;
										pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
										Tc_PassBall(hHeroWithBall,pos_target);
									}
									else{
										if(dd1==0&&d1>100) {ddiiss=d1/2;}
										if(dd2==0&&d2>100) {if(d2/2<ddiiss) ddiiss=d2/2;}
										if(ddiiss<2000)
										{
											if(iHeroWithBall.pos.x-ddiiss<iForbiddenArea.left.right+20)
												pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-ddiiss;
											else pos_target.x=iForbiddenArea.left.right+20;
											pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
											Tc_PassBall(hHeroWithBall,pos_target);
										}
									}
									if(ddiiss==2000)  Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
								}
							}
							else   //在中间
							{
								if(iHeroWithBall.pos.x-TC_PASSBALL_DISTANCE>iForbiddenArea.left.right+20)
									pos_target.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2-TC_PASSBALL_DISTANCE;
								else pos_target.x=iForbiddenArea.left.right+20;
								pos_target.y=iHeroWithBall.pos.y+TC_BALL_HEIGHT/2;
								Tc_PassBall(hHeroWithBall,pos_target);
							}
						}
						else   //在禁区内
						{
							double m1,m2,m3,m4,c1;
							m1=abs(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.left.top+50);
							m2=abs(iHeroWithBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.left.bottom-40);
							m3=abs(iHeroWithoutBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.left.top+50);
							m4=abs(iHeroWithoutBall.pos.y+TC_BALL_HEIGHT/2-iForbiddenArea.left.bottom-40);
							pos1.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2;
							pos1.y=iForbiddenArea.left.top-50;
							c1=m1;
							if(c1>m2)
							{
								pos1.x=iHeroWithBall.pos.x+TC_BALL_WIDTH/2;
								pos1.y=iForbiddenArea.left.bottom+40;
								c1=m2;
							}

							if(iEnemy_1.b_is_spelling==true)
							{
								if(iEnemy_1.type==TC_HERO_GINNY||iEnemy_1.type==TC_HERO_RON)
								{
									Tc_PassBall(hHeroWithBall,pos1);
								}
							}

							if(iEnemy_2.b_is_spelling==true)
							{
								if(iEnemy_2.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_RON)
								{
									Tc_PassBall(hHeroWithBall,pos1); 
								}
							}
							if(iEnemy_1.b_is_spelling==false&&iEnemy_2.b_is_spelling==false) Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
						}
					}
					break;
				}
			case GHOSTBALL_IN_ENEMY_TEAM:
				{
					/*if (iHero_1.b_snatch_ghostball == true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
					{
					Tc_SnatchBall(hHero_1,TC_GHOST_BALL); 
					}
					else 	Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					if (iHero_2.b_snatch_ghostball == true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
					{
					Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
					}
					else 	Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));*/
					if (iAttackDirection == TC_DIRECTION_RIGHT)  //向右攻
					{
						if (iEnemyWithBall.pos.x - iOwnGate.x<=300)    //离球门线400之内属于危险区域，放魔法
						{
							if (iHero_2.b_can_spell==true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iEnemyWithBall.pos.x+64,iEnemyWithBall.pos.y+64)<=500)
							{
								Tc_Spell(hHero_2,hEnemyWithBall);
								Move(hHero_1,TC_DIRECTION_RIGHT);
								break;
							}
							else if(iHero_2.b_is_spelling==true) 
							{
								Move(hHero_1,TC_DIRECTION_RIGHT);
								break;
							}
							else{
								if (iHero_1.b_snatch_ghostball == true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
								{
									Tc_SnatchBall(hHero_1,TC_GHOST_BALL); 
								}
								else 	Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								if (iHero_2.b_snatch_ghostball == true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
								{
									Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
								}
								else 	Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								break;
							}
							/*else if(iHero_1.b_is_spelling==false&&(iHero_1.b_can_spell==false||diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iEnemyWithBall.pos.x+64,iEnemyWithBall.pos.y+64)>500)&&iHero_2.b_can_spell==true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iEnemyWithBall.pos.x+64,iEnemyWithBall.pos.y+64)<=500)
							{
							Tc_Spell(hHero_2,hEnemyWithBall);
							Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							break;
							}*/
						}
						else{
							if (iHero_1.b_snatch_ghostball == true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
							{
								Tc_SnatchBall(hHero_1,TC_GHOST_BALL); 
							}
							else 	Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							if (iHero_2.b_snatch_ghostball == true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
							{
								Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
							}
							else 	Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							break;
						}
					}
					else
					{
						if (iOwnGate.x-iEnemyWithBall.pos.x <= 350)   //离球门线400之内属于危险区域，放魔法
						{
							if (iHero_2.b_can_spell==true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iEnemyWithBall.pos.x+64,iEnemyWithBall.pos.y+64)<=500)
							{
								Tc_Spell(hHero_2,hEnemyWithBall);
								Move(hHero_1,TC_DIRECTION_LEFT);						
								break;
							}
							else if(iHero_2.b_is_spelling==true)
							{
								Move(hHero_1,TC_DIRECTION_LEFT);						
								break;
							}
							else{
								if (iHero_1.b_snatch_ghostball == true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
								{
									Tc_SnatchBall(hHero_1,TC_GHOST_BALL); 
								}
								else 	Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								if (iHero_2.b_snatch_ghostball == true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
								{
									Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
								}
								else 	Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								break;
							}
							/*else if((iHero_1.b_is_spelling==false&&iHero_1.b_can_spell==false||diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iEnemyWithBall.pos.x+64,iEnemyWithBall.pos.y+64)>500)&&iHero_2.b_can_spell==true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iEnemyWithBall.pos.x+64,iEnemyWithBall.pos.y+64)<=500)
							{
							Tc_Spell(hHero_2,hEnemyWithBall);
							Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							break;
							}*/
						}
						else{
							if (iHero_1.b_snatch_ghostball == true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
							{
								Tc_SnatchBall(hHero_1,TC_GHOST_BALL); 
							}
							else 	Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							if (iHero_2.b_snatch_ghostball == true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
							{
								Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
							}
							else 	Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							break;
						}
					}

					break;
				}
			case GHOSTBALL_IN_NO_TEAM:
				{
					if (iHero_1.b_snatch_ghostball == true&&diss(iHero_1.pos.x+64,iHero_1.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
					{
						Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
					}
					else 
					{
						Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					}
					if (iHero_2.b_snatch_ghostball==true&&diss(iHero_2.pos.x+64,iHero_2.pos.y+64,iGhostBall.pos.x+32,iGhostBall.pos.y+32)<=60)
					{
						Tc_SnatchBall(hHero_2,TC_GHOST_BALL); 
					}
					else 
					{
						Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
					}
					break;
				}
		}

	}
}