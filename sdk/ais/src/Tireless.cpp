/*
队名：Tireless
成员：钮鑫涛
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
//基本信息
TC_Position goldBall;
double speedNormal=7;
double speedHaveBall=3;
TC_Handle hHero_1, hHero_2;
TC_Handle hEnemy_1, hEnemy_2;
TC_Handle hFreeBall, hGhostBall, hGoldBall;
TC_Ball iFreeBall, iGhostBall, iGoldBall;
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_Direction iAttackDirection;
TC_Direction DirHash[8];
TC_GameInfo gameInfo;

int  GinnySpell=0;
int  gHero1=0;
int  gHero2=0;
//处理信息的函数
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

	Tc_GetGameInfo(hHero_1,&gameInfo);

	if (iOwnGate.x < iEnemyGate.x)
		iAttackDirection = TC_DIRECTION_RIGHT;
	else
		iAttackDirection = TC_DIRECTION_LEFT;
}
//基本函数群
int getDisdance(TC_Position pos1,TC_Position pos2)
{
    return  abs((int)pos1.x-pos2.x)+abs((int)pos1.y-pos2.y);
}
int GetMoveDirection(TC_Position iHero,TC_Position target)
{
	int direction=-1;
	if(abs(iHero.y-target.y)<speedHaveBall)
	{
		if(target.x>=iHero.x)
			direction=0;
		else
		  if(target.x<iHero.x)
		   	direction=7;
	}
	else if(abs(iHero.x-target.x)<speedHaveBall)
	{
		if(target.y>=iHero.y)
			direction=3;
		else
		  if(target.y<iHero.y)
			direction=7;
	}
	else if(target.y>iHero.y&&target.x>iHero.x)
	{
     if((target.y-iHero.y)<speedNormal)
       direction=0;
	  else 
	  {
	    if((target.x-iHero.x)<speedNormal)
	     direction=3;
	    else
		 direction=1;
	  }
	}
	else if(target.y>iHero.y&&target.x>iHero.x)
	{
     if((target.y-iHero.y)<speedNormal)
       direction=0;
	  else 
	  {
	    if((target.x-iHero.x)<speedNormal)
	     direction=3;
	    else
		 direction=1;
	  }
	}
	else if(target.y<iHero.y&&target.x>iHero.x)
	{
       if((iHero.y-target.y)<speedNormal)
         direction=0;
	   else
	   {
		  if((target.x-iHero.x)<speedNormal)
	        direction=4;
		  else
			direction=2;
	   }
	}
	else if(target.y>iHero.y&&target.x<iHero.x)
	{
      if((target.y-iHero.y)<speedNormal)
       direction=7;
	  else
	   {
		 if((iHero.x-target.x)<speedNormal)
	      direction=3;
	     else
          direction=5;
	   }
	}
	else if(target.y<iHero.y&&target.x<iHero.x)
	{
      if((iHero.y-target.y)<speedNormal)
        direction=7;
	  else
	  {
	   if((iHero.x-target.x)<speedNormal)
	     direction=4;
	   else
        direction=6;
	  }
	}
	return  direction;
}
int PassBallState(TC_Handle hHero)//几种传球状态,相等,也是一种状态,
{
   int state=-1;
   TC_Hero hero;
   if(hHero==hHero_1)
	  hero=iHero_1;
   else
	  hero=iHero_2;
   if(hero.b_ghostball==true)
   {
    if(iAttackDirection==TC_DIRECTION_RIGHT)
    {
	   if(hero.pos.x>=iForbiddenArea.right.left-250)
        state=-1;
	   else
		 state=0;
    }
    else
    {
	   if(hero.pos.x<=iForbiddenArea.left.right+250-TC_HERO_WIDTH)
        state=-1;
	   else
		 state=0;
    }
   }
   return state;
}
TC_Position dealPos(TC_Position pos,int direction)//还有问题
{
	TC_Position target;
	target.x=pos.x;
	target.y=pos.y;
	if(direction==0)//攻击右侧
	{
	 if(pos.x>iForbiddenArea.right.left-512)
     {
		if(pos.y<iEnemyGate.y_upper-128)
		 {
				 target.x+=400;
				 target.y+=240;
		 }
		else  if(pos.y>iEnemyGate.y_lower+128)
		{
               target.y-=240;
			     target.x+=400;

		}
		else 
			target.x+=512;
	  }
	 else
		 target.x+=512;
	}
	else
	{
     if(pos.x<iForbiddenArea.left.right+512)
     {
		 if(pos.y<iEnemyGate.y_upper-128)
		 {
				 target.x-=400;
				 target.y+=240;
		 }
		 else  if(pos.y>iEnemyGate.y_lower+128)
		 {
                 target.y-=240;
				   target.x-=400;
		 }
		 else
			 target.x-=512;
	  }
     else
		 target.x-=512;
	}
	if(target.y>TC_MAP_HEIGHT-TC_BALL_HEIGHT)
		target.y=TC_MAP_HEIGHT-TC_BALL_HEIGHT;
	if(target.y<0)
		target.y=0;
    return target;
}
int getStep(int dis)
{
  int step;
  int a=(int)dis/10;
  step=a*2+13;
  return step;
}
TC_Position GetPassBallPositon(TC_Handle hHero)
{
	 int flag=0;
     TC_Position target;
     target.x=-1;
     target.y=-1;
     TC_Position hero;
	 if(hHero==hHero_1)
		 hero=iHero_1.pos;
	 else
		 hero=iHero_2.pos;
	 int a=512;
     int step1=9;
	 int step2=9;
	 int dis1;
	 int dis2;
	 if(iAttackDirection==TC_DIRECTION_RIGHT)
	 {
		  dis1=iEnemy_1.pos.x-hero.x-abs(iEnemy_1.pos.y-hero.y);
		  dis2=iEnemy_2.pos.x-hero.x-abs(iEnemy_2.pos.y-hero.y);
		  step1=getStep(dis1);
		  step2=getStep(dis2);

	 }
	 else
	 {	 
         dis1=hero.x-iEnemy_1.pos.x-abs(iEnemy_1.pos.y-hero.y);
		  dis2=hero.x-iEnemy_2.pos.x-abs(iEnemy_2.pos.y-hero.y);
		  step1=getStep(dis1);
		  step2=getStep(dis2);
         a=-512;
	 }
	 int ddis1=abs(iEnemy_1.pos.x-hero.x)+abs(iEnemy_1.pos.y-hero.y);
	 int ddis2=abs(iEnemy_2.pos.x-hero.x)+abs(iEnemy_2.pos.y-hero.y);
	 bool resultJu2=(iEnemy_2.steps_before_next_snatch>step2||iEnemy_2.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_2.abnormal_type==TC_SPELLED_BY_FREEBALL||step2<=0);
     bool resultJu1=(iEnemy_1.steps_before_next_snatch>step1||iEnemy_1.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_1.abnormal_type==TC_SPELLED_BY_FREEBALL||step1<=0);
	 if((resultJu1&&resultJu2)||(resultJu1&&ddis2>512)||(resultJu2&&ddis1>512)||(ddis1>512&ddis2>512))
	   {
        	  if(a>0)
				 target=dealPos(iGhostBall.pos,0);
			  else
               target=dealPos(iGhostBall.pos,1);//ballThroughEnemy(hero,iEnemy_1);
			flag=1;
	   }
		if(flag==1)
		{
           if(iAttackDirection==TC_DIRECTION_RIGHT)
           {
	        if(target.x>=iForbiddenArea.right.left-64)
		       target.x=iForbiddenArea.right.left-64;
           }
           else
           {
	        if(target.x<=iForbiddenArea.left.right)
		       target.x=iForbiddenArea.left.right;
           }
		}
  return target;
}
int getOutOfTrap(TC_Position hero,int dir)//当在外面要进入门内时，或在门内要出门外时，一定的轨迹，要改变方向,还是被卡
{
  TC_Gate iLeftGate;
  TC_Gate iRightGate;
  if(iAttackDirection==TC_DIRECTION_RIGHT)
  {
    iLeftGate=iOwnGate;
	iRightGate=iEnemyGate;
  }
  else
  {
	iLeftGate=iEnemyGate;
	iRightGate=iOwnGate;
  }
    if(hero.x>=iRightGate.x-TC_HERO_WIDTH)
    {
       if(hero.y==iRightGate.y_upper)
	   {
        if(dir==2)
          dir=0;
		 if(dir==6)
          dir=7;
		 if(dir==4)
          dir=7;
		}
		if(hero.y==iRightGate.y_lower-TC_HERO_HEIGHT)
		{
        if(dir==1)
          dir=0;
		 if(dir==5)
          dir=7;
        if(dir==3)
          dir=7;
		}
	 }
	 if(hero.x<=iLeftGate.x)
	 {
       if(hero.y==iLeftGate.y_upper)
	   {
        if(dir==2)
          dir=0;
		 if(dir==6)
          dir=7;
		 if(dir==4)
          dir=0;
	   }
	   if(hero.y==iLeftGate.y_lower-TC_HERO_HEIGHT)
	   {
        if(dir==1)
          dir=0;
		 if(dir==5)
          dir=7;
        if(dir==3)
          dir=0;
	   }
	 }
	 if(hero.y<=iRightGate.y_upper)
	 {
		 if(hero.x==iRightGate.x-TC_HERO_WIDTH)
		 {
          if(dir==2)
            dir=4;
		   if(dir==1)
            dir=3;
		   if(dir==0)
            dir=3;
		 }
	 }
	 if(hero.y>=iRightGate.y_lower-TC_HERO_HEIGHT)
	 {
		 if(hero.x==iRightGate.x-TC_HERO_WIDTH)
		 {
          if(dir==2)
           dir=4;
		  if(dir==1)
           dir=3;
		  if(dir==0)
           dir=4;
		 }
	 }
	 if(hero.y<=iLeftGate.y_upper)
	 {
	  if(hero.x==iLeftGate.x)
	  {
        if(dir==6)
         dir=4;
		if(dir==5)
         dir=3;
		if(dir==7)
         dir=3;
	  }
	 }
	 if(hero.y>=iLeftGate.y_lower-TC_HERO_HEIGHT)
	 {
	  if(hero.x==iLeftGate.x)
	  {
       if(dir==6)
         dir=4;
		if(dir==5)
         dir=3;
		if(dir==7)
         dir=4;
	   }
	 }
	 if(hero.y==0)//如果在底线，直走
	 {
        if(dir==6)
         dir=7;
		if(dir==2)
		  dir=0;
	 }
	 if(hero.y==TC_MAP_HEIGHT-TC_HERO_HEIGHT)
	 {
       if(dir==1)
		   dir=0;
	   if(dir==5)
		   dir=7;
	 }
	 return dir;
}
//判断函数群
bool canCatchGhostBall(TC_Handle hHero)
{
	bool result=false;
   TC_Hero hero;
	TC_Hero freindHero;
	if(hHero==hHero_1)
	{
		hero=iHero_1;
		freindHero=iHero_2;
	}
	else
	{
       hero=iHero_2;
	   freindHero=iHero_1;
	}
	if(hero.b_ghostball!=true&&freindHero.b_ghostball!=true)
	{
		if(hero.b_snatch_ghostball==true)
			result=true;
	}
	if(freindHero.abnormal_type==TC_SPELLED_BY_MALFOY||freindHero.abnormal_type==TC_SPELLED_BY_FREEBALL||freindHero.abnormal_type==TC_SPELLED_BY_HERMIONE)
	{
	   if(hero.b_snatch_ghostball==true)
			result=true;
	}
	if(hHero==hHero_1)
	{ 
       if(gHero1==1)
		   result=false;
	}
	if(hHero==hHero_2)
	{ 
       if(gHero2==1)
		   result=false;
	}
	if(iEnemy_1.b_is_spelling==true&&iEnemy_1.b_ghostball==true)//好像又不是了
         result=false;
	if(iEnemy_2.b_is_spelling==true&&iEnemy_2.b_ghostball==true)
         result=false;
  return result;
}
//执行函数群
void Goal()//进球，固定x，y和人相同
{
	int direction=-1;
    TC_Position Target;
	
	 if(iAttackDirection==TC_DIRECTION_RIGHT)
		 Target.x=iForbiddenArea.right.right+500;
	 else
        Target.x=0;
	 if(iHero_1.b_ghostball==true)
	 {		
		if(iHero_1.pos.y<iEnemyGate.y_upper)
			Target.y=iEnemyGate.y_upper-1;
		else
			if(iHero_1.pos.y+TC_HERO_HEIGHT>iEnemyGate.y_lower)
				Target.y=iEnemyGate.y_lower-TC_HERO_HEIGHT+1;
		   else
		       Target.y=iHero_1.pos.y;
		direction=GetMoveDirection(iHero_1.pos,Target);
		direction=getOutOfTrap(iHero_1.pos,direction);
       Tc_Move(hHero_1,DirHash[direction]);
	 }
	 if(iHero_2.b_ghostball==true)
	 {
		 if(iHero_2.pos.y<iEnemyGate.y_upper)
			Target.y=iEnemyGate.y_upper-1;
		else
			if(iHero_2.pos.y+TC_HERO_HEIGHT>iEnemyGate.y_lower)
				Target.y=iEnemyGate.y_lower-TC_HERO_HEIGHT+1;
		   else
		       Target.y=iHero_2.pos.y;
		 direction=GetMoveDirection(iHero_2.pos,Target);
		 direction=getOutOfTrap(iHero_2.pos,direction);
		 Tc_Move(hHero_2,DirHash[direction]);
	 }
}
void catchGoldBall()//还是按方位来判断
{
	 if(iGoldBall.b_visible==true)
	 {    
		 int disSmall=64;
		 int left,right,up,down;
		 up=0;
		 down=TC_MAP_HEIGHT-(int)0.5*TC_GOLDBALL_WIDTH;
		 if(iAttackDirection==TC_DIRECTION_RIGHT)
		 {
			 left=iOwnGate.x;
			 right=iEnemyGate.x-128;
		 }
		 else
		 { 
			 left=iEnemyGate.x;
			 right=iOwnGate.x-128;
		 }
		 int disL=abs(int(iGoldBall.pos.x-left));
		 int disR=abs(int(right-iGoldBall.pos.x));
		 int disU=abs(int(iGoldBall.pos.y-up));
		 int disD=abs(int(down-iGoldBall.pos.y));
        TC_Position pos2;
		if(disL<=disSmall&&disU<=disSmall)
		{
			if(goldBall.x==-1||goldBall.y==-1||getDisdance(goldBall,iGoldBall.pos)<100)
			{
			  pos2.x=right-disL;
			  pos2.y=down-disU;
			  goldBall.x=iGoldBall.pos.x;
			  goldBall.y=iGoldBall.pos.y;
			}
			else
			{
				pos2.x=goldBall.x;
				pos2.y=goldBall.y;
			   goldBall.x=iGoldBall.pos.x;
			   goldBall.y=iGoldBall.pos.y;
			}
		}
		else if(disR<=disSmall&&disD<=disSmall)
		{
			if(goldBall.x==-1||goldBall.y==-1||getDisdance(goldBall,iGoldBall.pos)<100)
			{
			  pos2.x=left+disR;
			  pos2.y=up+disD;
			  goldBall.x=iGoldBall.pos.x;
			  goldBall.y=iGoldBall.pos.y;
			}
			else
			{
				pos2.x=goldBall.x;
				pos2.y=goldBall.y;
				goldBall.x=iGoldBall.pos.x;
			   goldBall.y=iGoldBall.pos.y;
			}
		}
		else if(disL<=disSmall&&disD<=disSmall)
		{
			if(goldBall.x==-1||goldBall.y==-1||getDisdance(goldBall,iGoldBall.pos)<100)
			{
				pos2.x=right-disL;
			   pos2.y=up+disD;
			   goldBall.x=iGoldBall.pos.x;
			   goldBall.y=iGoldBall.pos.y;
			}
			else
			{
				pos2.x=goldBall.x;
				pos2.y=goldBall.y;
							  goldBall.x=iGoldBall.pos.x;
			  goldBall.y=iGoldBall.pos.y;
			}
		}
		else if(disR<=disSmall&&disU<=disSmall)
		{
			if(goldBall.x==-1||goldBall.y==-1||getDisdance(goldBall,iGoldBall.pos)<100)
			{
				pos2.x=left+disR;
		       pos2.y=down-disU;
			   goldBall.x=iGoldBall.pos.x;
			   goldBall.y=iGoldBall.pos.y;
			}
			else
			{
				pos2.x=goldBall.x;
				pos2.y=goldBall.y;
							  goldBall.x=iGoldBall.pos.x;
			  goldBall.y=iGoldBall.pos.y;
			}
		}  
		else if(disL<=disSmall)
		{
			if(goldBall.x==-1||goldBall.y==-1||getDisdance(goldBall,iGoldBall.pos)<100)
			{
			pos2.x=right-disL;
			pos2.y=iGoldBall.pos.y;
			}
			else
			{
				pos2.x=goldBall.x;
				pos2.y=goldBall.y;
			}

		}
		else if(disR<=disSmall)
		{
		  if(goldBall.x==-1||goldBall.y==-1||getDisdance(goldBall,iGoldBall.pos)<100)
		  {
			  pos2.x=left+disR;
			  pos2.y=iGoldBall.pos.y;
		  }
		  else
		  {
				pos2.x=goldBall.x;
				pos2.y=goldBall.y;
		  }
		}
		else if(disU<=disSmall)
		{
			if(goldBall.x==-1||goldBall.y==-1||getDisdance(goldBall,iGoldBall.pos)<100)
			{
			pos2.y=down-disU;
			pos2.x=iGoldBall.pos.x;
			}
			else
			{
				pos2.x=goldBall.x;
				pos2.y=goldBall.y;
			}
		}
		else if(disD<=disSmall)
		{
         if(goldBall.x==-1||goldBall.y==-1||getDisdance(goldBall,iGoldBall.pos)<100)
		 { 
			pos2.y=up+disD;
			pos2.x=iGoldBall.pos.x;
		 }
		 else
		 {
				pos2.x=goldBall.x;
				pos2.y=goldBall.y;
		 }
		}
		else
		{
			pos2.y=up+disD;
			pos2.x=iGoldBall.pos.x;
		}
		 if(abs((int)gameInfo.score_enemy-(int)gameInfo.score_self)<=2)//是否做出修改
              Goal();
		 else if(gameInfo.score_enemy>gameInfo.score_self)//抢球
		 {
	       	 double d1=getDisdance(iHero_1.pos,iGoldBall.pos);
			 double d2=getDisdance(iHero_2.pos,iGoldBall.pos);
			 if(d1<d2)
			 {
			   int direction1=GetMoveDirection(iHero_1.pos,iGoldBall.pos);
			     direction1=getOutOfTrap(iHero_1.pos,direction1);
			   if(direction1!=-1)
               Tc_Move(hHero_1,DirHash[direction1]);
              
			   int direction2=GetMoveDirection(iHero_2.pos,pos2);
			   direction2=getOutOfTrap(iHero_2.pos,direction2);
			  Tc_Move(hHero_2,DirHash[direction2]);
			 }
			 else
			 {
			   int direction1=GetMoveDirection(iHero_2.pos,iGoldBall.pos);
			     direction1=getOutOfTrap(iHero_2.pos,direction1);
			   if(direction1!=-1)
               Tc_Move(hHero_2,DirHash[direction1]);
              
			   int direction2=GetMoveDirection(iHero_1.pos,pos2);
			   direction2=getOutOfTrap(iHero_1.pos,direction2);
			  Tc_Move(hHero_1,DirHash[direction2]);
			 }
		 }
		 else 
		 {
             int direction1=GetMoveDirection(iHero_1.pos,iEnemy_1.pos);
			     direction1=getOutOfTrap(iHero_1.pos,direction1);
			   if(direction1!=-1)
               Tc_Move(hHero_1,DirHash[direction1]);
              
			   int direction2=GetMoveDirection(iHero_2.pos,iEnemy_2.pos);
			   direction2=getOutOfTrap(iHero_2.pos,direction2);
			  Tc_Move(hHero_2,DirHash[direction2]);
		 }
		  if(iHero_1.b_snatch_goldball==true)
           Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
	      if(iHero_2.b_snatch_goldball==true)    
	        Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
	 }
}
void catchGhostBall(TC_Handle hHero)//保持好两个英雄32距离，一个靠近ghostball，一个在后面一点,敌方英雄时同时拿
{
	TC_Position  BallPos;
	TC_Speed    BallSpeed;
	TC_Hero hero;
	TC_Hero freindHero;
	int direction=-1;
	if(hHero==hHero_1)
	{
		hero=iHero_1;
		freindHero=iHero_2;
	}
	else
	{
       hero=iHero_2;
	   freindHero=iHero_1;
	}
	BallPos=iGhostBall.pos;
	BallSpeed=iGhostBall.speed;   
   
	BallPos.x+=(int)(0.5*TC_BALL_WIDTH); 
	BallPos.y+=(int)(0.5*TC_BALL_HEIGHT);
	TC_Position person=hero.pos;
	TC_Position perFri=freindHero.pos;
	person.x+=(int)(0.5* TC_HERO_WIDTH); 
	person.y+=(int)(0.5*TC_HERO_HEIGHT);
	perFri.x+=(int)(0.5* TC_HERO_WIDTH); 
	perFri.y+=(int)(0.5*TC_HERO_HEIGHT);

	double d1=getDisdance(person,BallPos);
	double d2=getDisdance(perFri,BallPos);
	if(d1>d2)
	{
      if(iAttackDirection==TC_DIRECTION_RIGHT)
			BallPos.x-=(int)(0.51*TC_BALL_WIDTH);
		else
           BallPos.x+=(int)(0.51*TC_BALL_WIDTH);
	}
	direction=GetMoveDirection(person,BallPos);
	direction=getOutOfTrap(hero.pos,direction);
	if(direction!=-1)
	{
       Tc_Move(hHero,DirHash[direction]);
	}
	if(canCatchGhostBall(hHero))
	{
      Tc_SnatchBall(hHero,TC_GHOST_BALL);
	}
}
void passBall()
{
	int flag1=0;
	int flag2=0;
	TC_Position target;
	if(iHero_1.b_ghostball==true)
	{
       flag1=PassBallState(hHero_1);
	   if(flag1==-1)
		  Goal();
	   else
	   {
        target=GetPassBallPositon(hHero_1);
		 if(target.x==-1||target.y==-1)
			 Goal();
		 else
		    Tc_PassBall(hHero_1,target);	  
	   }
	}
	if(iHero_2.b_ghostball==true)
	{
      flag2=PassBallState(hHero_2);
	   if(flag2==-1)
		  Goal();
	   else
	   {
         target=GetPassBallPositon(hHero_2);
		  if(target.x==-1||target.y==-1)
			  Goal();
		  else
	        Tc_PassBall(hHero_2,target);
	   }
	}
}
void attack_1(int direct)
{
 if(iEnemy_1.type==TC_HERO_RON)
 {
   if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
  {
   Tc_Spell(hHero_1,hEnemy_1);
  }
 }
 else if(iEnemy_2.type==TC_HERO_RON)
 {
  if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
  {
   Tc_Spell(hHero_1,hEnemy_2);
  }
 }
 else if(iEnemy_1.type==TC_HERO_GINNY&&direct==0)//进攻时
 {
   if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
   {
    Tc_Spell(hHero_1,hEnemy_1);
   }
 }
 else if(iEnemy_2.type==TC_HERO_GINNY&&direct==0)
 {
  if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
  {
   Tc_Spell(hHero_1,hEnemy_2);
  }
 }
 else if(iEnemy_1.type==TC_HERO_MALFOY) 
 {                       
   if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
   {
    Tc_Spell(hHero_1,hEnemy_1);
   }
 }
 else if(iEnemy_2.type==TC_HERO_MALFOY)
 {
   if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
  {
   Tc_Spell(hHero_1,hEnemy_2);
  }
 }
 else if(iEnemy_1.type==TC_HERO_HERMIONE)
 {
  if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
   {
    Tc_Spell(hHero_1,hEnemy_1);
   }
 }
 else if(iEnemy_2.type==TC_HERO_HERMIONE)
 {
  if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
  {
   Tc_Spell(hHero_1,hEnemy_2);
  }
 }
 else 
 {
   if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
   {
    Tc_Spell(hHero_1,hEnemy_1);
   }
   if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
   {
    Tc_Spell(hHero_1,hEnemy_2);
   }
 }
}
void attack_2(int direct)
{
 if(iEnemy_1.type==TC_HERO_RON&&Tc_CanBeSpelled(hHero_2,hEnemy_1))
 {
   Tc_Spell(hHero_2,hEnemy_1);
 }
 else if(iEnemy_2.type==TC_HERO_RON&&Tc_CanBeSpelled(hHero_2,hEnemy_2))
 {
   Tc_Spell(hHero_2,hEnemy_2);
 }
 else if(iEnemy_1.type==TC_HERO_GINNY&&direct==0&&Tc_CanBeSpelled(hHero_2,hEnemy_1))//进攻时
 {
    Tc_Spell(hHero_2,hEnemy_1);
 }
 else if(iEnemy_2.type==TC_HERO_GINNY&&direct==0&&Tc_CanBeSpelled(hHero_2,hEnemy_2))
 {
   Tc_Spell(hHero_2,hEnemy_2);
 }
 else if(iEnemy_1.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hHero_2,hEnemy_1)) 
 {                       
    Tc_Spell(hHero_2,hEnemy_1);
 }
 else if(iEnemy_2.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hHero_2,hEnemy_2))
 {
   Tc_Spell(hHero_2,hEnemy_2);
 }
 else if(iEnemy_1.type==TC_HERO_HERMIONE&&Tc_CanBeSpelled(hHero_2,hEnemy_1))
 {
    Tc_Spell(hHero_2,hEnemy_1);
 }
 else if(iEnemy_2.type==TC_HERO_HERMIONE&&Tc_CanBeSpelled(hHero_2,hEnemy_2))
 {
   Tc_Spell(hHero_2,hEnemy_2);
 }
 else 
 {
   if(Tc_CanBeSpelled(hHero_2,hEnemy_1))
   {
    Tc_Spell(hHero_2,hEnemy_1);
   }
   if(Tc_CanBeSpelled(hHero_2,hEnemy_2))
   {
    Tc_Spell(hHero_2,hEnemy_2);
   }
 }
}
void attack()
{
	if(gameInfo.game_time_left<8&&gameInfo.score_self-gameInfo.score_enemy>=2)//当小于10秒时
	{
	  if(getDisdance(iEnemy_2.pos,iGoldBall.pos)<85)
	  {
       if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
	   {
        Tc_Spell(hHero_1,hEnemy_2);
	   }
	   if(Tc_CanBeSpelled(hHero_2,hEnemy_2))
	   {
        Tc_Spell(hHero_2,hEnemy_2);
	   }
	  }
	  if(getDisdance(iEnemy_1.pos,iGoldBall.pos)<85)
	  {
	   if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
	   {
        Tc_Spell(hHero_1,hEnemy_1);
	   }
	   	if(Tc_CanBeSpelled(hHero_2,hEnemy_2))
	   {
        Tc_Spell(hHero_2,hEnemy_2);
	   }
	  }
	}
	else
	{
        if(iAttackDirection==TC_DIRECTION_RIGHT)
  	    {	  
				if(iHero_1.b_ghostball==true)
				{
					if(iHero_1.pos.x>iForbiddenArea.right.left-200)
		             attack_1(0);
					if(iHero_1.pos.x>iForbiddenArea.right.left+100)
					  attack_2(0);
				}
				if(iHero_2.b_ghostball==true)
				{
					if(iHero_2.pos.x>iForbiddenArea.right.left-200)
					  attack_1(0);
					if(iHero_2.pos.x>iForbiddenArea.right.left+150)
					  attack_2(0);
				}
		       if(iEnemy_1.b_ghostball==true)
			   {
				   if(iEnemy_1.pos.x<iForbiddenArea.left.right)
			      {       
                     if(Tc_CanBeSpelled(hHero_2,hEnemy_1))
                     {
						  Tc_Spell(hHero_2,hEnemy_1);
					    if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
                       Tc_Spell(hHero_2,hEnemy_2);
						}
					    else
                        if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
                          Tc_Spell(hHero_2,hEnemy_1);
				   }
			   }
		      if(iEnemy_2.b_ghostball==true)
		      {
				 if(iEnemy_2.pos.x<iForbiddenArea.left.right)
			    {     
                    if(Tc_CanBeSpelled(hHero_2,hEnemy_2))
					  { 
						  Tc_Spell(hHero_2,hEnemy_2);
				       if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
                       Tc_Spell(hHero_1,hEnemy_1);
					  }
					  else
                      if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
                          Tc_Spell(hHero_2,hEnemy_2);
				}
			  }
	     }
	     else
	     {	
				if(iHero_1.b_ghostball==true)
				{
					if(iHero_1.pos.x<iForbiddenArea.left.right+200-TC_HERO_WIDTH)
		             attack_1(0);
					if(iHero_1.pos.x<iForbiddenArea.left.right-100-TC_HERO_WIDTH)
					  attack_2(0);
				}
				if(iHero_2.b_ghostball==true)
				{
					if(iHero_2.pos.x<iForbiddenArea.left.right+200-TC_HERO_WIDTH)
						attack_1(0);
					if(iHero_2.pos.x<iForbiddenArea.left.right-150-TC_HERO_WIDTH)
						attack_2(0);
				}
		       if(iEnemy_1.b_ghostball==true)
			   {
				   if(iEnemy_1.pos.x>iForbiddenArea.right.left-TC_HERO_WIDTH)
			      {
						 if(Tc_CanBeSpelled(hHero_2,hEnemy_1))
						 {
							Tc_Spell(hHero_2,hEnemy_1);
						   if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
                         Tc_Spell(hHero_2,hEnemy_2);
						 }
						 else
                        if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
                          Tc_Spell(hHero_2,hEnemy_1);
			      }
			   }
		      if(iEnemy_2.b_ghostball==true)
		      {
				 if(iEnemy_2.pos.x>iForbiddenArea.right.left-TC_HERO_WIDTH)
			    {
					 if(Tc_CanBeSpelled(hHero_2,hEnemy_2))
					 {
						 Tc_Spell(hHero_2,hEnemy_2); 
					   if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
                       Tc_Spell(hHero_1,hEnemy_1);
					 }
					 else
                     if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
                       Tc_Spell(hHero_2,hEnemy_2);
				}
			  }
	   }
	}
}
void actting()
{  
	catchGhostBall(hHero_1);
	catchGhostBall(hHero_2);
	passBall();
	attack();
	catchGoldBall();
}
void hideGinny()//左右区分方向,出来会被卡住？？
{
	if(iEnemy_2.type==TC_HERO_GINNY)
	{
	  if(iEnemy_2.b_is_spelling==true&&GinnySpell==0)
	  {
	    GinnySpell=1;
		if(iHero_1.b_ghostball==true&&Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==false)
		{
		  gHero1=1;
         TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.right.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400; 
				 target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_1,target);
			  }
			  else
			  {
				  { 
					  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.right.top))
					 target.y=iForbiddenArea.right.bottom+64;
					  else
					 target.y=iForbiddenArea.right.top-64;
				    target.x=iGhostBall.pos.x;
				    if(target.x>iForbiddenArea.right.right-TC_BALL_WIDTH)
				      target.x=iForbiddenArea.right.right-TC_BALL_WIDTH-1;
					 Tc_PassBall(hHero_1,target);
				  }
			  }
		  }
		  else
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.left.top)>TC_PASSBALL_DISTANCE)
			  { 
				  target.x=iForbiddenArea.left.right+64;//GhostBall.pos.x-400;
			     target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_1,target);
			  }
			  else
			  {
				  {
					  target.x=iGhostBall.pos.x;
					   if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.left.top))
				     target.y=iForbiddenArea.left.bottom+64;
					   else
                   target.y=iForbiddenArea.right.top-64;
					   if(target.x<iForbiddenArea.left.left)
					   target.x=iForbiddenArea.left.left+1;
					  Tc_PassBall(hHero_1,target);
				  }
			  }
		  }
		}
		if(iHero_2.b_ghostball==true&&Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==false)
		{
			gHero2=1;
            TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.right.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400;
			    target.y=iGhostBall.pos.y; 
				Tc_PassBall(hHero_2,target);
			  }
			  else
			  {
				  { 
                  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.right.top))
					 target.y=iForbiddenArea.right.bottom+64;
		           else
                  target.y=iForbiddenArea.right.top-64;
				    target.x=iGhostBall.pos.x;
					if(target.x>iForbiddenArea.right.right-TC_BALL_WIDTH)
				      target.x=iForbiddenArea.right.right-TC_BALL_WIDTH-1;
					 Tc_PassBall(hHero_2,target);
				  }
			  }
		  }
		  else
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.left.top)>TC_PASSBALL_DISTANCE)
			  { 
				  target.x=iForbiddenArea.left.right+64;//GhostBall.pos.x-400;
			     target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_2,target);
			  }
			  else
			  {
				  {
					  target.x=iGhostBall.pos.x;
                if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.left.top))
				     target.y=iForbiddenArea.left.bottom+64;
                 else
					 target.y=iForbiddenArea.left.top-64;
				   if(target.x<iForbiddenArea.left.left)
					  target.x=iForbiddenArea.left.left+1;
					  Tc_PassBall(hHero_2,target);
				  }
		
			  }
		  }
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
	  if(iEnemy_1.b_is_spelling==true&&GinnySpell==0)
	  {
	    GinnySpell=1;
		if(iHero_1.b_ghostball==true&&Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==false)
		{
		  gHero1=1;
         TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.right.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400;
			    target.y=iGhostBall.pos.y; 
				Tc_PassBall(hHero_1,target);
			  }
			  else
			  {
				  { 
				    if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.right.top))
					 target.y=iForbiddenArea.right.bottom+64;
		           else
                  target.y=iForbiddenArea.right.top-128;
				    target.x=iGhostBall.pos.x;
					if(target.x>iForbiddenArea.right.right-TC_BALL_WIDTH)
				      target.x=iForbiddenArea.right.right-TC_BALL_WIDTH-1;
					 Tc_PassBall(hHero_1,target);
				  }
			  }
		  }
		  else
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.left.top)>TC_PASSBALL_DISTANCE)
			  { 
				  target.x=iForbiddenArea.left.right+64;//GhostBall.pos.x-400;
			     target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_1,target);
			  }
			  else
			  {
				  {
				    if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.left.top))
					 target.y=iForbiddenArea.left.bottom+64;
		           else
                  target.y=iForbiddenArea.left.top-64;
				    target.x=iGhostBall.pos.x;		 
				     if(target.x<iForbiddenArea.left.left)
					  target.x=iForbiddenArea.left.left+1;
					  Tc_PassBall(hHero_1,target);
				  }
			  }
		  }
		}
		if(iHero_2.b_ghostball==true&&Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==false)
		{
			gHero2=1;
            TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.right.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400;
			    target.y=iGhostBall.pos.y; 
				Tc_PassBall(hHero_2,target);
			  }
			  else
			  {
				  { 
				    if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.right.top))
					 target.y=iForbiddenArea.right.bottom+64;
		           else
                  target.y=iForbiddenArea.right.top-64;
				    target.x=iGhostBall.pos.x;
					 if(target.x>iForbiddenArea.right.right-TC_BALL_WIDTH)
				      target.x=iForbiddenArea.right.right-TC_BALL_WIDTH-1;
					 Tc_PassBall(hHero_2,target);
				  }
			  }
		  }
		  else
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.left.top)>TC_PASSBALL_DISTANCE)
			  { 
				  target.x=iForbiddenArea.left.right+64;//GhostBall.pos.x-400;
			     target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_2,target);
			  }
			  else
			  {
				  {
			       if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.left.top))
					 target.y=iForbiddenArea.left.bottom+64;
		           else
                  target.y=iForbiddenArea.left.top-64;
				    target.x=iGhostBall.pos.x;		 
					if(target.x<iForbiddenArea.left.left)
					  target.x=iForbiddenArea.left.left+1;
					  Tc_PassBall(hHero_2,target);
				  }
			  }
		  }
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
void hideRon()//是否和jinny一样,对方来抢球，能抢，就算了，不能就传
{
    if(iEnemy_2.type==TC_HERO_RON)
	{
	  if(iEnemy_2.b_is_spelling==true)
	  {
		if(iHero_1.b_ghostball==true&&Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==false)
		{
         TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.right.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400;
			    target.y=iGhostBall.pos.y; 
				Tc_PassBall(hHero_1,target);
			  }
			  else
			  {
					  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.right.top))
					 target.y=iForbiddenArea.right.bottom+64;
					  else
					 target.y=iForbiddenArea.right.top-64;
				    target.x=iGhostBall.pos.x;
					 if(target.x>iForbiddenArea.right.right-TC_BALL_WIDTH)
				      target.x=iForbiddenArea.right.right-TC_BALL_WIDTH-1;
					 Tc_PassBall(hHero_1,target);
			  }
		  }
		  else
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.left.top)>TC_PASSBALL_DISTANCE)
			  { 
				  target.x=iForbiddenArea.left.right+64;//GhostBall.pos.x-400;
			     target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_1,target);
			  }
			  else
			  {
					  target.x=iGhostBall.pos.x;
					   if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.left.top))
				     target.y=iForbiddenArea.left.bottom+64;
					   else
                   target.y=iForbiddenArea.right.top-64;
					   if(target.x<iForbiddenArea.left.left)
					  target.x=iForbiddenArea.left.left+1;
					  Tc_PassBall(hHero_1,target);
			  }
		  }
		}
		if(iHero_2.b_ghostball==true&&Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==false)
		{
            TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.right.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400;
			    target.y=iGhostBall.pos.y; 
				Tc_PassBall(hHero_2,target);
			  }
			  else
			  {
                  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.right.top))
					 target.y=iForbiddenArea.right.bottom+64;
		           else
                  target.y=iForbiddenArea.right.top-64;
				    target.x=iGhostBall.pos.x;
					if(target.x>iForbiddenArea.right.right-TC_BALL_WIDTH)
				      target.x=iForbiddenArea.right.right-TC_BALL_WIDTH-1;
					 Tc_PassBall(hHero_2,target);
			  }
		  }
		  else
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.left.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.left.right+64;
			     target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_2,target);
			  }
			  else
			  {
				  target.x=iGhostBall.pos.x;
                if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.left.top))
				     target.y=iForbiddenArea.left.bottom+64;
                 else
					 target.y=iForbiddenArea.left.top-64;
				if(target.x<iForbiddenArea.left.left)
					  target.x=iForbiddenArea.left.left+1;
					  Tc_PassBall(hHero_2,target);
			  }
		  }
		}
	  }
	}
	if(iEnemy_1.type==TC_HERO_RON)
	{
	  if(iEnemy_1.b_is_spelling==true)
	  {
		if(iHero_1.b_ghostball==true&&Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==false)
		{
         TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.right.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.right.left-64;//GhostBall.pos.x-400;
			    target.y=iGhostBall.pos.y; 
				Tc_PassBall(hHero_1,target);
			  }
			  else
			  {
				    if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.right.top))
					 target.y=iForbiddenArea.right.bottom+64;
		           else
                  target.y=iForbiddenArea.right.top-128;
				    target.x=iGhostBall.pos.x;
					if(target.x>iForbiddenArea.right.right-TC_BALL_WIDTH)
				      target.x=iForbiddenArea.right.right-TC_BALL_WIDTH-1;
					 Tc_PassBall(hHero_1,target);
			  }
		  }
		  else
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.left.top)>TC_PASSBALL_DISTANCE)
			  { 
				  target.x=iForbiddenArea.left.right+64;//GhostBall.pos.x-400;
			     target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_1,target);
			  }
			  else
			  {
				    if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.left.top))
					 target.y=iForbiddenArea.left.bottom+64;
		           else
                  target.y=iForbiddenArea.left.top-64;
				    target.x=iGhostBall.pos.x;	
					if(target.x<iForbiddenArea.left.left)
					  target.x=iForbiddenArea.left.left+1;
					  Tc_PassBall(hHero_1,target);
			  }
		  }
		}
		if(iHero_2.b_ghostball==true&&Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==false)
		{
            TC_Position target;
		  if(iAttackDirection==TC_DIRECTION_RIGHT)
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.right.top)>TC_PASSBALL_DISTANCE)
			  { 
				 target.x=iForbiddenArea.right.left-64;
			    target.y=iGhostBall.pos.y; 
				Tc_PassBall(hHero_2,target);
			  }
			  else
			  {
				    if(abs((int)iGhostBall.pos.y-iForbiddenArea.right.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.right.top))
					 target.y=iForbiddenArea.right.bottom+64;
		           else
                  target.y=iForbiddenArea.right.top-64;
				    target.x=iGhostBall.pos.x;
					if(target.x>iForbiddenArea.right.right-TC_BALL_WIDTH)
				      target.x=iForbiddenArea.right.right-TC_BALL_WIDTH-1;
					 Tc_PassBall(hHero_2,target);
			  }
		  }
		  else
		  {
			  if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)>TC_PASSBALL_DISTANCE&&abs((int)iGhostBall.pos.y-iForbiddenArea.left.top)>TC_PASSBALL_DISTANCE)
			  { 
				  target.x=iForbiddenArea.left.right+64;//GhostBall.pos.x-400;
			     target.y=iGhostBall.pos.y; 
				 Tc_PassBall(hHero_2,target);
			  }
			  else
			  {
			       if(abs((int)iGhostBall.pos.y-iForbiddenArea.left.bottom)<=abs((int)iGhostBall.pos.y-iForbiddenArea.left.top))
					 target.y=iForbiddenArea.left.bottom+64;
		           else
                  target.y=iForbiddenArea.left.top-64;
				    target.x=iGhostBall.pos.x;
					if(target.x<iForbiddenArea.left.left)
					  target.x=iForbiddenArea.left.left+1;
					  Tc_PassBall(hHero_2,target);
			  }
		  }
		}
	  }
	}
}
void initDirHash()
{
	DirHash[0]=TC_DIRECTION_RIGHT;    
	DirHash[1]=TC_DIRECTION_RIGHTBOTTOM;
	DirHash[2]=TC_DIRECTION_RIGHTTOP;
	DirHash[3]=TC_DIRECTION_BOTTOM;
	DirHash[4]=TC_DIRECTION_TOP; 
	DirHash[5]=TC_DIRECTION_LEFTBOTTOM; 
	DirHash[6]=TC_DIRECTION_LEFTTOP; 
	DirHash[7]=TC_DIRECTION_LEFT;
}
CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(L"Tireless");
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"malfoy");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_RON,L"Ron");
	initDirHash();
	goldBall.x=-1;
	goldBall.y=-1;
}
CLIENT_EXPORT_API void __stdcall AI()
{
	getInfo();
	actting();
	hideGinny();
	hideRon();
}