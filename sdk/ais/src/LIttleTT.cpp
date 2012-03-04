/**********************THE LOG OF TURING CUP*****************************
Author:imfeiteng/abilitytao/cyacm
History:	
*2009/11/22 基本策略制定完毕
*2009/11/23 增加避游走球AI
*2009/11/23 纠正优先级
*2009/11/24 更正避游走球重要BUG
*2009/11/24 增加智能抢金色飞贼AI
*2009/11/24 修正抢金色飞贼AI
*2009/11/25 增加禁区内传球策略，躲避吹球
*2009/11/25 修正边界移动方向错误
*2009/11/25 攻击力增强，暴力抢球能力提升
*2009/11/25 修正传球判断函数
*2009/12/03 纠正（左侧）禁区内传球bug "||写成&&...汗..."
*2009/12/03 调整左右反击线
*2009/12/03 最终完成，经最后测试无误，submit~
/************************************************************************/

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


CLIENT_EXPORT_API void __stdcall Init()
{
	// TODO: Set team name(no Chinese) and choose heros here
	Tc_SetTeamName(L"LIttleTT");
	Tc_ChooseHero(TC_MEMBER1, TC_HERO_GINNY, L"WT");
	Tc_ChooseHero(TC_MEMBER2, TC_HERO_RON,L"SGY");
}
//有关场地信息的宏
#define CENTREX 1024  //球场中心X坐标
#define CENTREY 384 //球场中心Y坐标
#define L_FORBIDDEN_LINE 456 //左侧禁区线
#define R_FORBIDDEN_LINE 1592//右侧禁区线
#define L_CS_LINE (456) //左半场反击线(CounterStrike)
#define R_CS_LINE ( (1592-128) ) //右半场反击线(CounterStrike)，注意这里要减128
#define TC_MAP_WIDTH	2048
#define TC_MAP_HEIGHT	768
#define TC_HERO_SPELL_DISTANCE	512 //两个英雄中心连线的长度小于这个值说明在攻击范围内
#define TC_SNATCH_DISTANCE_GHOST	64 //鬼飞球抢球范围
#define TC_SNATCH_DISTANCE_GOLD		64 //飞贼抢球范围
#define TC_COLLIDE_WITH_FREEBALL	64 //游走球和人的中心距离小于这个值时会被游走球砸
#define TC_PASSBALL_DISTANCE		512 //	最远的传球距离
#define DANGERDIS 40 //定义危险距离，若对方离本方球门线小于这个距离，则考虑放魔法
#define GRAB_GOLD_BALL_LIMITATION  2 //若大于这个分数，去抢金色飞贼


//状态宏区域
int state=0;
//////////////////////////////////////////////////////////////////////////
#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
//////////////////////////////////////////////////////////////////////////

//策略宏区域
int stategy=0;
//////////////////////////////////////////////////////////////////////////
//普通策略
#define NO_STATEGY 0
#define FORCE_GRAB_FREE_BALL 1
#define FORCE_ATTACK 2
#define FORCE_DEFEND 3
//特殊策略
#define SPECIAL_STATEGY 4
#define RACING_FOR_THE_BALL 5


//所有需要的状态
//////////////////////////////////////////////////////////////////////////
TC_Handle hHero_1, hHero_2;
TC_Handle hEnemy_1, hEnemy_2;
TC_Handle hHeroWithBall, hHeroWithoutBall;
TC_Handle hEnemyWithBall, hEnemyWithoutBall;
TC_Handle hFreeBall, hGhostBall, hGoldBall;
TC_Ball iFreeBall, iGhostBall, iGoldBall;
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2, iHeroWithBall, iHeroWithoutBall, iEnemyWithBall, iEnemyWithoutBall;

int warn;
int whoisbeingspelled;
TC_Hero iHeroBeingSpelled;
TC_Gate iOwnGate, iEnemyGate;
TC_GameInfo iOwnGameInfo,iEnemyGameInfo;
TC_Position imove[2];

//imove[0].x=0;
//imove[0].y=0;
//imove[1].x=0;
//imove[1].y=0;
int pmove=0;

TC_ForbiddenArea iForbiddenArea;
TC_Direction iAttackDirection;
//////////////////////////////////////////////////////////////////////////
#define OFFSET 7
//获得到某个敌人的进攻方向，修正抖动误差,允许误差范围为OFFSET
TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pEnemy)
{
	// "↑"
	if ( abs(pHero->pos.x - pEnemy->pos.x) < OFFSET && ( pHero->pos.y - pEnemy->pos.y ) >= OFFSET )
		return TC_DIRECTION_TOP;
	// "↓"
	if ( abs(pHero->pos.x - pEnemy->pos.x) < OFFSET && ( pEnemy->pos.y - pHero->pos.y ) >= OFFSET) 
		return TC_DIRECTION_BOTTOM;
	// "←"
	if ( (pHero->pos.x - pEnemy->pos.x)>=OFFSET && abs(pHero->pos.y - pEnemy->pos.y)< OFFSET )     
		return TC_DIRECTION_LEFT;
	// "→"
	if ( (pEnemy->pos.x - pHero->pos.x)>=OFFSET && abs(pHero->pos.y - pEnemy->pos.y)< OFFSET )   
		return TC_DIRECTION_RIGHT;
	//////////////////////////////////////////////////////////////////////////
	// "↖"
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)
		return TC_DIRECTION_LEFTTOP;
	// "↗"
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)
		return TC_DIRECTION_RIGHTTOP;
	// "↘"
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)
		return TC_DIRECTION_RIGHTBOTTOM;
	// "↙"
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)
		return TC_DIRECTION_LEFTBOTTOM;

	return iAttackDirection;//考虑健壮性

}
//获得到某个球的方向
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)
{
	// "↑"
	if ( abs(pHero->pos.x - (pBall->pos.x-32) ) < OFFSET && ( pHero->pos.y - (pBall->pos.y-32) ) >= OFFSET )  
		return TC_DIRECTION_TOP;
	// "↓"
	if ( abs(pHero->pos.x - (pBall->pos.x-32) ) < OFFSET && ( pBall->pos.y-32 - (pHero->pos.y) ) >= OFFSET)  
		return TC_DIRECTION_BOTTOM;
	// "←"
	if ( (pHero->pos.x - (pBall->pos.x-32) ) >=OFFSET && abs(pHero->pos.y - (pBall->pos.y-32) ) < OFFSET )  
		return TC_DIRECTION_LEFT;
	// "→"
	if ( (pBall->pos.x-32 - (pHero->pos.x) ) >=OFFSET && abs(pHero->pos.y - (pBall->pos.y-32) ) < OFFSET )  
		return TC_DIRECTION_RIGHT;
	//////////////////////////////////////////////////////////////////////////
	// "↖"
	if ( pHero->pos.x > (pBall->pos.x-32)   && pHero->pos.y > (pBall->pos.y-32) )
		return TC_DIRECTION_LEFTTOP;
	// "↗"
	if (pHero->pos.x < (pBall->pos.x-32)  && pHero->pos.y > (pBall->pos.y-32) )
		return TC_DIRECTION_RIGHTTOP;
	// "↘"
	if (pHero->pos.x < (pBall->pos.x-32) && pHero->pos.y < (pBall->pos.y-32)  )
		return TC_DIRECTION_RIGHTBOTTOM;
	// "↙"
	if (pHero->pos.x > (pBall->pos.x-32) && pHero->pos.y < (pBall->pos.y-32) )
		return TC_DIRECTION_LEFTBOTTOM;

	return iAttackDirection;//考虑健壮性

}
//获得两点间的最短进攻方向,从pos1 → pos2
TC_Direction GetPosToPosDirection(TC_Position *pos1,TC_Position *pos2)
{
	// "↑"
	if ( abs(pos1->x - pos2->x) < OFFSET && ( pos1->y - pos2->y ) >= OFFSET )return TC_DIRECTION_TOP;
	// "↓"
	if ( abs(pos1->x - pos2->x) < OFFSET && ( pos2->y - pos1->y ) >= OFFSET) return TC_DIRECTION_BOTTOM;
	// "←"
	if ( (pos1->x - pos2->x)>=OFFSET && abs(pos1->y - pos2->y)< OFFSET )     return TC_DIRECTION_LEFT;
	// "→"
	if ( (pos2->x - pos1->x)>=OFFSET && abs(pos1->y - pos2->y)< OFFSET )     return TC_DIRECTION_RIGHT;
	//////////////////////////////////////////////////////////////////////////
	// "↖"
	if (pos1->x > pos2->x && pos1->y > pos2->y)return TC_DIRECTION_LEFTTOP;
	// "↗"
	if (pos1->x < pos2->x && pos1->y > pos2->y)return TC_DIRECTION_RIGHTTOP;
	// "↘"
	if (pos1->x < pos2->x && pos1->y < pos2->y)return TC_DIRECTION_RIGHTBOTTOM;
	// "↙"
	if (pos1->x > pos2->x && pos1->y < pos2->y)return TC_DIRECTION_LEFTBOTTOM;

	return iAttackDirection;//考虑健壮性
}


//判断两点间最短可达距离，贪心尽量走对角线+左右上下直走,可以用来初步判断谁先抢到球
int GetMinDis(TC_Position *pos1,TC_Position *pos2)
{
	int res=0;
	int dx=abs(pos1->x - pos2->x);
	int dy=abs(pos1->y - pos2->y);
	if(dx>=dy)
	{
		res+=(int)sqrt(double(dy*dy+dy*dy));
		res+=(dx-dy);
		return res;
	}
	else 
	{
		res+=(int)sqrt(double(dx*dx+dx*dx));
		res+=(dy-dx);
		return res;
	}
}

//获得两个点中心连线的距离
int GetLineDis(TC_Position *pos1,TC_Position *pos2)
{
	int res=(int)sqrt( (double) (pos1->x-pos2->x)*(pos1->x-pos2->x)+(pos1->y-pos2->y)*(pos1->y-pos2->y) );
	return res;
}



//获得到球门的进攻方向
TC_Direction GetAttackDirection(TC_Hero *pHero)
{
	if (pHero->pos.y >= iEnemyGate.y_lower-128)
	{
		if (iAttackDirection == TC_DIRECTION_RIGHT)
		{

			if(abs(iEnemyGate.x-pHero->pos.x)<TC_HERO_WIDTH+OFFSET)
				return TC_DIRECTION_TOP;
			else
				return TC_DIRECTION_RIGHTTOP;
		}
		else
		{
			if(abs(iEnemyGate.x-pHero->pos.x)<TC_HERO_WIDTH+OFFSET)
				return TC_DIRECTION_TOP;
			else
				return TC_DIRECTION_LEFTTOP;
		}
	}
	else
	{
		if (pHero->pos.y <= iEnemyGate.y_upper+2)
		{
			if (iAttackDirection == TC_DIRECTION_RIGHT)
			{

				if(abs(iEnemyGate.x-pHero->pos.x)<TC_HERO_WIDTH)
					return TC_DIRECTION_BOTTOM;
				else
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			else
			{
				if(abs(iEnemyGate.x-pHero->pos.x)<TC_HERO_WIDTH)
					return TC_DIRECTION_BOTTOM;
				else
					return TC_DIRECTION_LEFTBOTTOM;
			}

		}
		else
			return iAttackDirection;
	}
}

//可用这个判断球是否在被passing....O(∩_∩)O~   This is Good!
bool IsBallInPassing(TC_Ball *pBall)
{
	if(imove[0].x==imove[1].x&&imove[0].y==imove[1].y)
		return false;
	else
		return true;
}


//截获基本信息
//////////////////////////////////////////////////////////////////////////
int GetBasicInfo()
{
	//获得句柄
	hHero_1 = Tc_GetHeroHandle(TC_MEMBER1);
	hHero_2 = Tc_GetHeroHandle(TC_MEMBER2);
	hEnemy_1 = Tc_GetEnemyHandle(TC_MEMBER1);
	hEnemy_2 = Tc_GetEnemyHandle(TC_MEMBER2);
	hFreeBall = Tc_GetBallHandle(TC_FREE_BALL);
	hGhostBall = Tc_GetBallHandle(TC_GHOST_BALL);
	hGoldBall = Tc_GetBallHandle(TC_GOLD_BALL);
	//获得引用指针
	Tc_GetBallInfo(hGoldBall,&iGoldBall);
	Tc_GetBallInfo(hFreeBall,&iFreeBall);
	Tc_GetBallInfo(hGhostBall,&iGhostBall);
	Tc_GetHeroInfo(hHero_1,&iHero_1);
	Tc_GetHeroInfo(hHero_2,&iHero_2);
	Tc_GetEnemyInfo(hEnemy_1,&iEnemy_1);
	Tc_GetEnemyInfo(hEnemy_2,&iEnemy_2);
	//获得场地信息
	Tc_GetOwnGate(hHero_1,&iOwnGate);
	Tc_GetEnemyGate(hEnemy_1,&iEnemyGate);
	Tc_GetForbiddenArea(&iForbiddenArea);
	Tc_GetGameInfo(hHero_1,&iOwnGameInfo);
	Tc_GetGameInfo(hEnemy_1,&iEnemyGameInfo);
	if (iOwnGate.x < iEnemyGate.x)
		iAttackDirection = TC_DIRECTION_RIGHT;
	else
		iAttackDirection = TC_DIRECTION_LEFT;

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
	return GHOSTBALL_IN_NO_TEAM;
}

//决策函数
//////////////////////////////////////////////////////////////////////////
int UpdateInfo()
{
	pmove++;
	pmove%=2;
	imove[pmove]=iGhostBall.pos;

	//增加被施法标志
	//////////////////////////////////////////////////////////////////////////
	if( (iEnemy_1.b_is_spelling==true||iEnemy_2.b_is_spelling==true)&&warn!=1)
	{
		warn=1;
		if(hHeroWithBall==hHero_1)
			whoisbeingspelled=1;
		else
			whoisbeingspelled=2;
	}
	if(iEnemy_1.b_is_spelling==false&&iEnemy_2.b_is_spelling==false)
		warn=0;
	//////////////////////////////////////////////////////////////////////////

	if(warn==1)
	{

		if(whoisbeingspelled==1)
			iHeroBeingSpelled=iHero_1;

		else
			iHeroBeingSpelled=iHero_2;
	}
	//更新pmove和move数组
	if(state<=3)
	{
		if(state==GHOSTBALL_IN_NO_TEAM)
			return FORCE_GRAB_FREE_BALL;
		else if(state==GHOSTBALL_IN_SELF_TEAM)
			return FORCE_ATTACK;
		else if(state==GHOSTBALL_IN_ENEMY_TEAM)
			return FORCE_DEFEND;
		return SPECIAL_STATEGY;
	}
	else 
		return SPECIAL_STATEGY;
}
//封装save_move(Hhand,TC_DIRECTION),避游走球
//////////////////////////////////////////////////////////////////////
#define BOOMDIS 68//相碰时两图片中心的距离
TC_Position nextPosition(TC_Hero *ph,TC_Direction d,TC_Position ret)
{//此ret下在d方向下下一步位置，ONESTEP是英雄的只有水平速度时的绝对值
	int add,ONESTEP;
	if(ph->b_ghostball)
	{
		add=1;
		ONESTEP=7;
	}
	else 
	{
		ONESTEP=5;
		add=2;
	}
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
int vdir[8][2]={
	0,1,
	-1,1,
	-1,0,
	-1,-1,
	0,-1,
	1,-1,
	1,0,
	1,1
};
bool Boom(TC_Hero * phero,TC_Direction d,int OFFS)
{
	//判断英雄在d方向下，连续OFFS步会撞不
	TC_Position hp=phero->pos;
	TC_Position bp=iFreeBall.pos;
	TC_Speed sp=iFreeBall.speed;
	int hx,hy,bx,by;
	int i;
	for(i=1;i<=OFFS;++i)
	{
		//	if(i>iFreeBall.u.moving_time_left)return false;//球如果消失不用判断了
		hp=nextPosition(phero,d,hp);
		if((bp.y-iFreeBall.u.path.pos_start.y)*(bp.y-iFreeBall.u.path.pos_end.y)<=0 && (bp.x-iFreeBall.u.path.pos_start.x)*(bp.x-iFreeBall.u.path.pos_end.x)<=0 )
		{
		}
		else {
			sp.vx=-sp.vx;
			sp.vy=-sp.vy;
		}
		bp.x+=sp.vx;
		bp.y+=sp.vy;
		hx=hp.x+TC_HERO_WIDTH/2;
		hy=hp.y+TC_HERO_HEIGHT/2;
		bx=bp.x+TC_BALL_WIDTH/2;
		by=bp.y+TC_BALL_HEIGHT/2;
		double dis=sqrt(double( (hx-bx)*(hx-bx)+(hy-by)*(hy-by) ));
		if(dis<BOOMDIS)//会相撞
			break;
	}
	return i<=OFFS;
}
bool Save_Move(TC_Handle h,TC_Direction d)//安全移动
{
	int ONESTEP;//只有水平速度时的的绝对值
	int OFFS;//预判步
	TC_Hero hero;
	Tc_GetHeroInfo(h,&hero);
	if(hero.b_ghostball)
	{
		OFFS=20;
	}
	else 
	{

		OFFS=20;
	}
	if(iFreeBall.b_visible=false||!Boom(&hero,d,OFFS))//当前不会碰
		return Tc_Move(h,d);
	TC_Direction md1,md2;
	for(int i=1;i<=4;++i)
	{
		md1=DIR[(d+i)%8];
		md2=DIR[(d+8-i)%8];
		if(!Boom(&hero,md1,OFFS))//不被砸到
		{
			if(Boom(&hero,md2,OFFS))//会砸到
				return Tc_Move(h,md1);

			if(vdir[md1][0]*iFreeBall.speed.vx<0&&vdir[md1][1]*iFreeBall.speed.vy<0)
				return Tc_Move(h,md1);//md1与自由球反反向
			else 
				return Tc_Move(h,md2);
		}

		if(!Boom(&hero,md2,OFFS))//第一个会被砸到，第二个不会
			return Tc_Move(h,md2);	
	}
	return Tc_Move(h,d);//必被砸到
}
//////////////////////////////////////////////////////////////////////////////

//封装只能抢金色飞贼AI
/*
//////////////////////////////////////////////////////////////////////////
struct catch_gold_ball
{
	int XAWAY;
	int EPS;
	int YAWAY;
	int WAITDIS;
	int DIS;
	TC_Hero ih1,ih2;
	TC_Handle hh1,hh2;
	TC_Position p1,p2;
	int d11,d12,d21,d22;
	void  driveGlodball()
	{
		//确定h1去驱赶飞贼，h2到另一边去劫持
		d11=GetMinDis(&(iHero_1.pos),&p1);
		d12=GetMinDis(&(iHero_1.pos),&p2);
		d21=GetMinDis(&(iHero_2.pos),&p1);
		d22=GetMinDis(&(iHero_2.pos),&p2);
		if(d11+d22+17<d12+d21)
		{
			hh1=hHero_1;
			hh2=hHero_2;
			ih1=iHero_1;
			ih2=iHero_2;
			return ;
		}
		else if(d12+d21+17<d11+d22)
		{
			hh2=hHero_1;
			hh1=hHero_2;
			ih2=iHero_1;
			ih1=iHero_2;
			return ;
		}
		hh1=hHero_1;
		hh2=hHero_2;
		ih1=iHero_1;
		ih2=iHero_2;
		return ;

	}
	bool inCorrer()
	{
		TC_Position correr;
		correr.x=32;
		correr.y=32;
		if(GetMinDis(&(iGoldBall.pos),&correr)<DIS)
		{
			return true;
		}
		correr.x=32;
		correr.y=TC_MAP_HEIGHT-32;
		if(GetMinDis(&(iGoldBall.pos),&correr)<DIS)
		{
			return true;
		}
		correr.x=TC_MAP_WIDTH-32;
		correr.y=32;
		if(GetMinDis(&(iGoldBall.pos),&correr)<DIS)
		{
			return true;
		}
		correr.x=TC_MAP_WIDTH-32;
		correr.y=TC_MAP_HEIGHT-32;
		if(GetMinDis(&(iGoldBall.pos),&correr)<DIS)
		{
			return true;
		}
		return false;
	}
	void run()
	{
		if((iGoldBall.pos.y-iHero_1.pos.y)*(iGoldBall.pos.y-iHero_2.pos.y)<0)
		{
			Save_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
			Save_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
			return ;
		}
		if(iGoldBall.pos.y+TC_BALL_HEIGHT/2<TC_MAP_HEIGHT/2)
		{//穿上
			p1.x=iGoldBall.pos.x+XAWAY;
			p1.y=iGoldBall.pos.y+YAWAY;
			p2.x=iGoldBall.pos.x-XAWAY;
			p2.y=TC_MAP_HEIGHT-WAITDIS;
		}
		else
		{//穿下
			p1.x=iGoldBall.pos.x+XAWAY;
			p1.y=iGoldBall.pos.y-YAWAY;
			p2.x=iGoldBall.pos.x-XAWAY;
			p2.y=WAITDIS;
		}
		driveGlodball();
		if(abs(p1.x-ih1.pos.x)<EPS&&abs(p2.x-ih2.pos.x)<EPS)
		{//开始抓
			Save_Move(hh1,GetBallDirection(&ih1,&iGoldBall));
			TC_Position p;
			p.x=iGoldBall.pos.x;
			if(ih2.pos.y+TC_HERO_HEIGHT/2<TC_MAP_HEIGHT/2)//在上面
				p.y=0;
			else 
				p.y=TC_MAP_HEIGHT;
			//	p.y=TC_MAP_HEIGHT-ih1.pos.y;
			Save_Move(hh2,GetPosToPosDirection(&(ih2.pos),&p));
			return;
		}
		else
		{
			Save_Move(hh1,GetPosToPosDirection(&(ih1.pos),&p1));
			Save_Move(hh2,GetPosToPosDirection(&(ih2.pos),&p2));
			return;
		}

	}
	catch_gold_ball()
	{
		DIS=150;
		EPS=155;
		XAWAY=100;
		YAWAY=355;
		WAITDIS=320;
	}
}CATCH_GOLD_BALL;
//////////////////////////////////////////////////////////////////////////*/

#define NEARDIS 128 //判断离敌人是否比较近的界限，即一个身位
///////////////////////////CREATED_BY_ABILITYTAO_AND_CYACM_2009_NOV///////////////////////////////////////////////
CLIENT_EXPORT_API void __stdcall AI()
{

	//截获状态并生成策略
	state=GetBasicInfo();
	stategy=UpdateInfo();

	//抓金色飞贼优先级最高,仅作尝试，小概率事件^_^
	if(iGoldBall.b_visible==true)
	{
		if(iHero_1.b_snatch_goldball)
			if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL))
				return ;
		else if(iHero_2.b_snatch_goldball)
			if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL))
				return;
		
	}
	//////////////////////////////////////////////////////////////////////////


	//这里貌似有问题，领先的时候也进入这个函数，奇怪！
	//if(iGoldBall.b_visible==true&&( ((int)iOwnGameInfo.score_enemy-(int)iOwnGameInfo.score_self)>=GRAB_GOLD_BALL_LIMITATION ) )
	//{
	//	if(iHero_1.b_snatch_goldball)
	//		Tc_SnatchBall(hHero_1,TC_GOLD_BALL);	
	//	if(iHero_2.b_snatch_goldball)
	//		Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
	//	CATCH_GOLD_BALL.run();
	//	return;
	//}
	/*
	TC_Position t_h;
	TC_Position t_b;
	t_h.x=iHeroWithBall.pos.x+64;
	t_h.y=iHeroWithBall.pos.y+64;
	t_b.x=iFreeBall.pos.x+32;
	t_b.y=iFreeBall.pos.y+32;
	if(GetLineDis(&t_h,&t_b)<100)
	{

	int f=1;
	}//用来调试
	*/


	//如果是从左向右攻
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		switch (stategy)
		{
		case FORCE_ATTACK:
			{

				//////////////////////////////////////////////////////////////////////////
				//优先级最低
				Save_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
				Save_Move(hHeroWithoutBall,GetPosToPosDirection(&iHeroWithoutBall.pos,&iHeroWithBall.pos));

				//拿到球后尽可能往前传，节约时间
				if(iHeroWithBall.pos.x<R_CS_LINE-TC_HERO_WIDTH)
				{
					TC_Position target;
					if(iHeroWithBall.pos.x+TC_PASSBALL_DISTANCE<R_FORBIDDEN_LINE)
						target.x=iHeroWithBall.pos.x+TC_PASSBALL_DISTANCE;
					else
						target.x=R_FORBIDDEN_LINE-1;
					target.y=iHeroWithBall.pos.y;
					if(iHeroWithBall.abnormal_type!=TC_SPELLED_BY_MALFOY&&iHeroWithBall.abnormal_type!=TC_SPELLED_BY_FREEBALL)
						Tc_PassBall(hHeroWithBall,target);
				}
				//如果在前场禁区右侧进攻时，形成混战，吹走一个人，局部二打一
				if(iHero_2.b_ghostball==false&&iHero_2.b_can_spell==true&&iHero_2.curr_blue>2000&&GetLineDis(&iHero_1.pos,&iHero_2.pos)<TC_HERO_SPELL_DISTANCE&&iHero_1.pos.x>(R_CS_LINE+128)&&GetLineDis(&iHero_1.pos,&iEnemy_1.pos)<TC_HERO_WIDTH*2&&GetLineDis(&iHero_1.pos,&iEnemy_2.pos)<TC_HERO_WIDTH*2)
				{
					if(GetLineDis(&iHero_1.pos,&iEnemy_1.pos)<=GetLineDis(&iHero_1.pos,&iEnemy_2.pos))
						Tc_Spell(hHero_2,hEnemy_1);
					else
						Tc_Spell(hHero_2,hEnemy_2);
				}

				//在禁区内遭遇吹球或吹人，如果不能交给队友则传到禁区下侧
				if(iHeroWithBall.pos.x>R_CS_LINE&&iEnemy_1.b_is_spelling==true&&(iEnemy_1.type==TC_HERO_GINNY||iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_MALFOY)&&(iEnemyGate.x-iHeroWithBall.pos.x)>TC_HERO_WIDTH)
				{
					if( (iEnemy_1.type==TC_HERO_GINNY||iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_MALFOY)&&GetLineDis(&iHeroWithBall.pos,&iHeroWithoutBall.pos)>(TC_HERO_WIDTH-64)  )
					{

						TC_Position t;
						t.x=R_FORBIDDEN_LINE+128+64;
						t.y=640;
						Tc_PassBall(hHeroWithBall,t);
					}


				}
				else if(iHeroWithBall.pos.x>R_CS_LINE&&iEnemy_2.b_is_spelling==true&&(iEnemy_2.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_MALFOY)&&(iEnemyGate.x-iHeroWithBall.pos.x)>TC_HERO_WIDTH)
				{
					if( (iEnemy_2.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_MALFOY)&&GetLineDis(&iHeroWithBall.pos,&iHeroWithoutBall.pos)>(TC_HERO_WIDTH-64)  )
					{
						TC_Position t;
						t.x=R_FORBIDDEN_LINE+128+64;
						t.y=640;
						Tc_PassBall(hHeroWithBall,t);
					}
				}
				//如果持球队员被晕了或是被乌龟了，另一个队员夺走球继续进攻
				if( ( ( warn==1&&iHeroBeingSpelled.b_ghostball==true)||(iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE||iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_RON) )  &&GetLineDis(&iHeroWithBall.pos,&iHeroWithoutBall.pos)<=(TC_HERO_WIDTH-32))
				{

					if(iHeroWithoutBall.b_snatch_ghostball==true)
					{
						bool test;//调试用1
						test=Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
						int a=1;//调试用2
					}
					else
						Save_Move(hHeroWithoutBall,GetPosToPosDirection(&iHeroWithoutBall.pos,&iHeroWithBall.pos));
				}
				break;
			}
		case FORCE_DEFEND:
			{

				//////////////////////////////////////////////////////////////////////////
				//暴抢，双人抢，优先级最低
				Save_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
				TC_Position centre;
				centre.x=CENTREX-TC_HERO_WIDTH/2;//让球员中心与中点重合
				centre.y=CENTREY-TC_HERO_WIDTH/2;//同上
				if(iHero_1.abnormal_type==TC_SPELLED_BY_NONE&&GetLineDis(&iHero_1.pos,&iEnemyWithBall.pos)<TC_HERO_SPELL_DISTANCE&&iEnemyWithBall.pos.x<L_CS_LINE&&(iHero_1.b_can_spell==true||iHero_1.b_is_spelling==true) )
				{
					Save_Move(hHero_2,GetPosToPosDirection(&iHero_2.pos,&centre));
				}

				else
					Save_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));

				//如果在中场混战，吹飞一人，二打一
				//if( iEnemy_1.pos.x<CENTREX+TC_HERO_WIDTH && iEnemy_1.pos.x >CENTREX-TC_HERO_WIDTH && iEnemy_2.pos.x<CENTREX+TC_HERO_WIDTH && iEnemy_2.pos.x >CENTREX-TC_HERO_WIDTH&&iHero_1.pos.x<CENTREX+TC_HERO_WIDTH && iHero_1.pos.x >CENTREX-TC_HERO_WIDTH&&iHero_2.pos.x<CENTREX+TC_HERO_WIDTH && iHero_2.pos.x >CENTREX-TC_HERO_WIDTH&&GetLineDis(&iHero_1.pos,&iEnemy_1.pos)<TC_HERO_WIDTH&&GetLineDis(&iHero_1.pos,&iEnemy_2.pos)<TC_HERO_WIDTH&&GetLineDis(&iHero_2.pos,&iEnemy_1.pos)<TC_HERO_WIDTH&&GetLineDis(&iHero_2.pos,&iEnemy_2.pos)<TC_HERO_WIDTH&&iHero_2.curr_blue>2000&&iHero_2.b_can_spell==true)
				//{

				//	if(GetLineDis(&iEnemy_1.pos,&iHero_2.pos)<=GetLineDis(&iEnemy_2.pos,&iHero_2.pos) )
				//	{
				//		Tc_Spell(hHero_2,hHeroWithBall);
				//	}
				//}


				//珍妮施法将球吹回中场

				if(iHero_1.type==TC_HERO_GINNY&&iHero_1.b_can_spell==true&&iEnemyWithBall.pos.x-iOwnGate.x<DANGERDIS&&iHero_2.b_is_spelling==false&&iEnemyWithBall.abnormal_type!=TC_SPELLED_BY_RON)
				{
					Tc_Spell(hHero_1,hEnemyWithBall);
				}

				//如果珍妮不能,RON协助将带球人吹回后场，争取时间,RON的danger值做了微调
				else if(iHero_2.type==TC_HERO_RON&&iHero_2.b_can_spell==true&&iEnemyWithBall.pos.x-iOwnGate.x<(DANGERDIS+10)&&GetLineDis(&iHero_2.pos,&iHeroWithBall.pos)<TC_HERO_SPELL_DISTANCE&&iHero_1.b_is_spelling==false)
				{
					Tc_Spell(hHero_2,hEnemyWithBall);
				}


				if(iHero_1.b_snatch_ghostball==true)
					Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				else if(iHero_2.b_snatch_ghostball==true)
					Tc_SnatchBall(hHero_2,TC_GHOST_BALL);


				//抓金色飞贼优先级最高,仅作尝试，小概率事件^_^
				if(iGoldBall.b_visible==true)
				{
					if(iHero_1.b_snatch_goldball)
						Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
					else if(iHero_2.b_snatch_goldball)
						Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
				}
				//////////////////////////////////////////////////////////////////////////
				break;


			}
		case FORCE_GRAB_FREE_BALL:
			{
				//////////////////////////////////////////////////////////////////////////

				int grabflag=0;
				if (iHero_1.b_snatch_ghostball == true)
				{
					if(IsBallInPassing(&iGhostBall)==true&&iGhostBall.speed.vx>0)
						Save_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					else
					{
						Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
						grabflag=1;
					}

				}
				else
					Save_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));


				//////////////////////////////////////////////////////////////////////////
				if (iHero_2.b_snatch_ghostball == true&&grabflag==0)
				{
					if(IsBallInPassing(&iGhostBall)==true&&iGhostBall.speed.vx>0)
						Save_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
					else
					{
						Tc_SnatchBall(hHero_2,TC_GHOST_BALL);

					}
				}
				else
					Save_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));

				break;
			}


		}
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if(iAttackDirection==TC_DIRECTION_LEFT)
	{
		///////////////////////////////////////////////////////
		/*if(iEnemy_1.b_is_spelling==true||iEnemy_2.b_is_spelling==true)
		{
			int res=1;
		}//调试用
		*/
		

		switch (stategy)
		{
		case FORCE_ATTACK:
			{

				//////////////////////////////////////////////////////////////////////////
				//优先级最低
				Save_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
				Save_Move(hHeroWithoutBall,GetPosToPosDirection(&iHeroWithoutBall.pos,&iHeroWithBall.pos));

				//拿到球后尽可能往前传，节约时间
				if(iHeroWithBall.pos.x>L_CS_LINE+TC_HERO_WIDTH)
				{
					TC_Position target;
					if(iHeroWithBall.pos.x+TC_HERO_WIDTH/2-TC_PASSBALL_DISTANCE-30>L_FORBIDDEN_LINE)
						target.x=iHeroWithBall.pos.x+TC_HERO_WIDTH/2-TC_PASSBALL_DISTANCE-30;
					else
						target.x=L_FORBIDDEN_LINE+1;
					target.y=iHeroWithBall.pos.y;
					if(iHeroWithBall.abnormal_type!=TC_SPELLED_BY_MALFOY&&iHeroWithBall.abnormal_type!=TC_SPELLED_BY_FREEBALL)
					{
						bool test;
						test=Tc_PassBall(hHeroWithBall,target);
						int a=1;
					}
				}
				//如果在前场禁区左侧进攻时，形成混战，吹走一个人，局部二打一
				if(iHero_2.b_ghostball==false&&iHero_2.b_can_spell==true&&iHero_2.curr_blue>2000&&GetLineDis(&iHero_1.pos,&iHero_2.pos)<TC_HERO_SPELL_DISTANCE&&iHero_1.pos.x<(L_CS_LINE-128)&&GetLineDis(&iHero_1.pos,&iEnemy_1.pos)<TC_HERO_WIDTH*2&&GetLineDis(&iHero_1.pos,&iEnemy_2.pos)<TC_HERO_WIDTH*2)
				{

					if(GetLineDis(&iHero_1.pos,&iEnemy_1.pos)<=GetLineDis(&iHero_1.pos,&iEnemy_2.pos))
						Tc_Spell(hHero_2,hEnemy_1);
					else
						Tc_Spell(hHero_2,hEnemy_2);

				}

				//在禁区内遭遇吹球或吹人，如果不能交给队友则传到禁区下侧
				if(iHeroWithBall.pos.x<L_CS_LINE+TC_HERO_WIDTH   &&   iEnemy_1.b_is_spelling==true  &&  (iEnemy_1.type==TC_HERO_GINNY||iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_MALFOY)   &&   (iEnemyGate.x-iHeroWithBall.pos.x)<0)
				{
					if( (iEnemy_1.type==TC_HERO_GINNY||iEnemy_1.type==TC_HERO_RON||iEnemy_1.type==TC_HERO_MALFOY)&&GetLineDis(&iHeroWithBall.pos,&iHeroWithoutBall.pos)>(TC_HERO_WIDTH-64)  )
					{

						TC_Position t;
						t.x=L_FORBIDDEN_LINE-128-64;
						t.y=640;
						Tc_PassBall(hHeroWithBall,t);
					}


				}

				else if(iHeroWithBall.pos.x<L_CS_LINE+TC_HERO_WIDTH&&iEnemy_2.b_is_spelling==true&&(iEnemy_2.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_MALFOY)&&iEnemyGate.x-iHeroWithBall.pos.x<0)
				{
					if( (iEnemy_2.type==TC_HERO_GINNY||iEnemy_2.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_MALFOY)&&GetLineDis(&iHeroWithBall.pos,&iHeroWithoutBall.pos)>(TC_HERO_WIDTH-64)  )
					{
						TC_Position t;
						t.x=L_FORBIDDEN_LINE-128-64;
						t.y=640;
						Tc_PassBall(hHeroWithBall,t);
					}
				}
				//如果持球队员被晕了或是被乌龟了，另一个队员夺走球继续进攻
				if( ( ( warn==1&&iHeroBeingSpelled.b_ghostball==true)||(iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE||iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_RON) ) &&GetLineDis(&iHeroWithBall.pos,&iHeroWithoutBall.pos)<=(TC_HERO_WIDTH-32))
				{

					if(iHeroWithoutBall.b_snatch_ghostball==true)
					{
						Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
					}
					else
						Save_Move(hHeroWithoutBall,GetPosToPosDirection(&iHeroWithoutBall.pos,&iHeroWithBall.pos));
				}
				break;
			}
		case FORCE_DEFEND:
			{

				//////////////////////////////////////////////////////////////////////////
				//暴抢，双人抢，优先级最低
				Save_Move(hHero_1,GetEnemyDirection(&iHero_1,&iEnemyWithBall));
				TC_Position centre;
				centre.x=CENTREX-TC_HERO_WIDTH/2;//让球员中心与中点重合
				centre.y=CENTREY-TC_HERO_WIDTH/2;//同上
				if(iHero_1.abnormal_type==TC_SPELLED_BY_NONE&&GetLineDis(&iHero_1.pos,&iEnemyWithBall.pos)<TC_HERO_SPELL_DISTANCE&&iEnemyWithBall.pos.x>R_CS_LINE&&(iHero_1.b_can_spell==true||iHero_1.b_is_spelling==true) )
				{
					Save_Move(hHero_2,GetPosToPosDirection(&iHero_2.pos,&centre));
				}

				else
					Save_Move(hHero_2,GetEnemyDirection(&iHero_2,&iEnemyWithBall));

				//如果在中场混战，吹飞一人，二打一
				//if( iEnemy_1.pos.x<CENTREX+TC_HERO_WIDTH && iEnemy_1.pos.x >CENTREX-TC_HERO_WIDTH && iEnemy_2.pos.x<CENTREX+TC_HERO_WIDTH && iEnemy_2.pos.x >CENTREX-TC_HERO_WIDTH&&iHero_1.pos.x<CENTREX+TC_HERO_WIDTH && iHero_1.pos.x >CENTREX-TC_HERO_WIDTH&&iHero_2.pos.x<CENTREX+TC_HERO_WIDTH && iHero_2.pos.x >CENTREX-TC_HERO_WIDTH&&GetLineDis(&iHero_1.pos,&iEnemy_1.pos)<TC_HERO_WIDTH&&GetLineDis(&iHero_1.pos,&iEnemy_2.pos)<TC_HERO_WIDTH&&GetLineDis(&iHero_2.pos,&iEnemy_1.pos)<TC_HERO_WIDTH&&GetLineDis(&iHero_2.pos,&iEnemy_2.pos)<TC_HERO_WIDTH&&iHero_2.curr_blue>2000&&iHero_2.b_can_spell==true)
				//{

				//	if(GetLineDis(&iEnemy_1.pos,&iHero_2.pos)<=GetLineDis(&iEnemy_2.pos,&iHero_2.pos) )
				//	{
				//		Tc_Spell(hHero_2,hHeroWithBall);
				//	}
				//}


				//珍妮施法将球吹回中场

				if(iHero_1.type==TC_HERO_GINNY&&iHero_1.b_can_spell==true&&abs(iEnemyWithBall.pos.x-iOwnGate.x)<DANGERDIS+TC_HERO_WIDTH&&iHero_2.b_is_spelling==false&&iEnemyWithBall.abnormal_type!=TC_SPELLED_BY_RON)
				{
					Tc_Spell(hHero_1,hEnemyWithBall);
				}

				//如果珍妮不能,RON协助将带球人吹回后场，争取时间,RON的danger值做了微调
				else if(iHero_2.type==TC_HERO_RON&&iHero_2.b_can_spell==true&&abs(iEnemyWithBall.pos.x-iOwnGate.x)<(DANGERDIS+10+TC_HERO_WIDTH)&&GetLineDis(&iHero_2.pos,&iHeroWithBall.pos)<TC_HERO_SPELL_DISTANCE&&iHero_1.b_is_spelling==false)
				{
					Tc_Spell(hHero_2,hEnemyWithBall);
				}


				if(iHero_1.b_snatch_ghostball==true)
					Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				else if(iHero_2.b_snatch_ghostball==true)
					Tc_SnatchBall(hHero_2,TC_GHOST_BALL);


				//抓金色飞贼优先级最高,仅作尝试，小概率事件^_^
				if(iGoldBall.b_visible==true)
				{
					if(iHero_1.b_snatch_goldball)
						Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
					else if(iHero_2.b_snatch_goldball)
						Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
				}
				//////////////////////////////////////////////////////////////////////////
				break;


			}
		case FORCE_GRAB_FREE_BALL:
			{
				//////////////////////////////////////////////////////////////////////////

				int grabflag=0;
				if (iHero_1.b_snatch_ghostball == true)
				{
					if(IsBallInPassing(&iGhostBall)==true&&iGhostBall.speed.vx<0)
						Save_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					else
					{
						Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
						grabflag=1;
					}

				}
				else
					Save_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));


				//////////////////////////////////////////////////////////////////////////
				if (iHero_2.b_snatch_ghostball == true&&grabflag==0)
				{
					if(IsBallInPassing(&iGhostBall)==true&&iGhostBall.speed.vx<0)
						Save_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
					else
					{
						Tc_SnatchBall(hHero_2,TC_GHOST_BALL);

					}
				}
				else
					Save_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));

				break;
			}
		}

		
	}
}

///////////////////////////CREATED_BY_ABILITYTAO_AND_CYACM_2009_NOV///////////////////////////////////////////////
//Finished by imfeiteng,abilitytao and cyacm 
