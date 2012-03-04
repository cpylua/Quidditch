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
	
	队名：Che
	队长：蒋艳明
	队员：沈肖波
	联系方式：15850576174

*/






/*
	How to debug?

	Goto
	Project -. Properties --> Configuration Properties --> Debugging

	Copy "$(ProjectDir)TuringCup9.exe" 
	to the Command field.(Without quotation marks)
*/


CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(L"Che");
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_HERMIONE,L"jym");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_MALFOY,L"che");
}

#define GHOSTBALL_IN_NO_TEAM 0  //表示球不在任一队
#define GHOSTBALL_IN_SELF_TEAM 1  //表示球在自己队
#define GHOSTBALL_IN_ENEMY_TEAM 2 //表示球在敌队
#define GOLDBALL_REACHABLE 3
TC_Handle  game;
TC_Handle hHero_1, hHero_2;  //队员句柄
TC_Handle hEnemy_1, hEnemy_2; //敌方人员句柄
TC_Handle hHeroWithBall, hHeroWithoutBall;
TC_Handle hEnemyWithBall, hEnemyWithoutBall;
TC_Handle hFreeBall, hGhostBall, hGoldBall;//三种球句柄
TC_Ball iFreeBall, iGhostBall, iGoldBall;  //三种球变量
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2, iHeroWithBall, iHeroWithoutBall, iEnemyWithBall, iEnemyWithoutBall;//英雄变量
TC_Gate iOwnGate, iEnemyGate;  //敌我球门变量
TC_ForbiddenArea iForbiddenArea; //传球区域
TC_Direction iAttackDirection;  //方向变量
TC_GameInfo  igame;
int flag;
int flags;
int  GetDianDistance(TC_Position p1,TC_Position p2)  //两点间的距离
{
      double x=(p1.x-p2.x)*(p1.x-p2.x);
	  double y=(p1.y-p2.y)*(p1.y-p2.y);
	  int z= (int)sqrt(x+y);
	  return  z;
}
int GetDistance(TC_Hero hero,TC_Hero hero1)
{
      double x=(hero.pos.x-hero1.pos.x)*(hero.pos.x-hero1.pos.x);
	  double y=(hero.pos.y-hero1.pos.y)*(hero.pos.y-hero1.pos.y);
	  int z= (int)sqrt(x+y);
	  return  z;
}
int GetDistanceBall(TC_Hero hero,TC_Ball ball)
{
      double x=(hero.pos.x-ball.pos.x)*(ball.pos.x-ball.pos.x);
	  double y=(hero.pos.y-ball.pos.y)*(ball.pos.y-ball.pos.y);
	  int z= (int)sqrt(x+y);
	  return  z;
}
int GetState()
{
	if (iHero_1.b_ghostball == false && iHero_2.b_ghostball == false && iEnemy_1.b_ghostball == false && iEnemy_2.b_ghostball == false)
	{                                  //球不在任一队
		return GHOSTBALL_IN_NO_TEAM;
	}
	if (iHero_1.b_ghostball == true || iHero_2.b_ghostball == true)  //球在队友手上
	{  
		//具体判断在哪个英雄手里
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
	if (iEnemy_1.b_ghostball == true || iEnemy_2.b_ghostball == true)  //球在敌方手上
	{    
		//具体判断在谁手上
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
TC_Direction  GetPointDirection(int x,int y,TC_Hero *pHero)
{
if (pHero->pos.x >= x)
	{
		if(abs(pHero->pos.y - y) <=7)
			return  TC_DIRECTION_LEFT;
		else if(pHero->pos.y - y > 7)
			return	TC_DIRECTION_LEFTTOP;
		else
			return TC_DIRECTION_LEFTBOTTOM;
	}
	else
	{
		if(abs(pHero->pos.y - y) <= 7)
			return  TC_DIRECTION_RIGHT;
		else if(pHero->pos.y - y > 7)
			return	TC_DIRECTION_RIGHTTOP;
		else
			return TC_DIRECTION_RIGHTBOTTOM;
	}
}
TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pHero1)   //为到英雄方向		pHero1为另一个英雄
{
	if (pHero->pos.x >= pHero1->pos.x)
	{
		if(abs(pHero->pos.y - pHero1->pos.y) <=7)
			return  TC_DIRECTION_LEFT;
		else if(pHero->pos.y - pHero1->pos.y > 7)
			return	TC_DIRECTION_LEFTTOP;
		else
			return TC_DIRECTION_LEFTBOTTOM;
	}
	else
	{
		if(abs(pHero->pos.y - pHero1->pos.y) <= 7)
			return  TC_DIRECTION_RIGHT;
		else if(pHero->pos.y - pHero1->pos.y > 7)
			return	TC_DIRECTION_RIGHTTOP;
		else
			return TC_DIRECTION_RIGHTBOTTOM;
	}
}
TC_Direction GetHeroDirection(TC_Hero *pHero, TC_Hero *pHero1)   //为到英雄方向		pHero1为另一个英雄
{
	if (pHero->pos.x >= pHero1->pos.x)
	{
		if(abs(pHero->pos.y - pHero1->pos.y) <=20)
			return  TC_DIRECTION_LEFT;
		else if(pHero->pos.y - pHero1->pos.y > 150)
			return	TC_DIRECTION_LEFTTOP;
		else
			return TC_DIRECTION_LEFTBOTTOM;
	}
	else
	{
		if(abs(pHero->pos.y - pHero1->pos.y) <= 20)
			return  TC_DIRECTION_RIGHT;
		else if(pHero->pos.y - pHero1->pos.y > 150)
			return	TC_DIRECTION_RIGHTTOP;
		else
			return TC_DIRECTION_RIGHTBOTTOM;
	}
}
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)//得到球的方向
{
	if (pHero->pos.x <= pBall->pos.x)
	{
		if(abs(pHero->pos.y - pBall->pos.y+20) <=7)
			return  TC_DIRECTION_RIGHT;
		else if(pHero->pos.y - pBall->pos.y+20 >7)
			return	TC_DIRECTION_RIGHTTOP;
		else
			return TC_DIRECTION_RIGHTBOTTOM;
	}
	else
	{
		if(abs(pHero->pos.y - pBall->pos.y+20) <=7)
			return  TC_DIRECTION_LEFT;
		else if(pHero->pos.y - pBall->pos.y+20 > 7)
			return	TC_DIRECTION_LEFTTOP;
		else
			return TC_DIRECTION_LEFTBOTTOM;
	}
}
TC_Direction  GetIattack(){
if (iOwnGate.x < iEnemyGate.x)//判断自己球门在哪边
	{
		iAttackDirection = TC_DIRECTION_RIGHT;
	}
	else
	{
		iAttackDirection = TC_DIRECTION_LEFT;
	}
	return iAttackDirection;
}
TC_Direction GetAttackDirection(TC_Hero *pHero)//返回球门的位置
{
	if(iAttackDirection == TC_DIRECTION_RIGHT)   
	{
       	if (pHero->pos.y >= iEnemyGate.y_lower-150)
		{
              if(iEnemyGate.x-pHero->pos.x>200) return  TC_DIRECTION_RIGHTTOP;
			 else   return  TC_DIRECTION_TOP;
		}
		if(pHero->pos.y<=iEnemyGate.y_upper+2)
		{
               if(iEnemyGate.x-pHero->pos.x>200) return  TC_DIRECTION_RIGHTBOTTOM;
			   else   return TC_DIRECTION_BOTTOM;
		}
		if((pHero->pos.y>iEnemyGate.y_upper+2)&&(pHero->pos.y<iEnemyGate.y_lower-150))  
		{
                   return TC_DIRECTION_RIGHT;
		}
	}
	else
	{
       	if (pHero->pos.y >= iEnemyGate.y_lower-150)
		{ 
			if(pHero->pos.x-iEnemyGate.x>200) return  TC_DIRECTION_LEFTTOP;
			 else   return  TC_DIRECTION_TOP;
		}
		if(pHero->pos.y<=iEnemyGate.y_upper+2)
		{
              if(pHero->pos.x-iEnemyGate.x>200) return  TC_DIRECTION_LEFTBOTTOM;
			   else   return TC_DIRECTION_BOTTOM;
		}
		if((pHero->pos.y>iEnemyGate.y_upper+2)&&(pHero->pos.y<iEnemyGate.y_lower-150))  
		{
            return  TC_DIRECTION_LEFT;
		}
	}
}
void  Spell(TC_Handle hHerowith,TC_Hero iHerowith,TC_Handle hHeroout,TC_Hero iHeroout)
{       
	                 TC_Direction  dir1=GetHeroDirection(&iHerowith,&iEnemy_1);
                    TC_Direction  dir2=GetHeroDirection(&iHerowith,&iEnemy_2);
                   if(iAttackDirection == TC_DIRECTION_RIGHT) 
				   {
	                   if((dir1==TC_DIRECTION_RIGHT||dir1==TC_DIRECTION_RIGHTBOTTOM||dir1==TC_DIRECTION_RIGHTTOP)&&iEnemy_1.b_snatch_ghostball)
					   {
						   if(Tc_CanBeSpelled(hHerowith,hEnemy_1))   
									{ 
									  Tc_Spell(hHerowith,hEnemy_1);  
									}
						   else
						   {
					      if(Tc_CanBeSpelled(hHerowith,hEnemy_1))   
									{ 
									  Tc_Spell(hHerowith,hEnemy_1);  
									} 
						   }
						  }
					   if((dir2==TC_DIRECTION_RIGHT||dir2==TC_DIRECTION_RIGHTBOTTOM||dir2==TC_DIRECTION_RIGHTTOP)&&iEnemy_2.b_snatch_ghostball)
					   {
						   if(Tc_CanBeSpelled(hHerowith,hEnemy_2))   
									{ 
									  Tc_Spell(hHerowith,hEnemy_2);  
									}
						   else
						   {
                             if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2))   
									{ 
									  Tc_Spell(hHeroWithoutBall,hEnemy_2);  
									}
						   }
						   }
					  }
				   else
				   {
                      if((dir1==TC_DIRECTION_LEFT||dir1==TC_DIRECTION_LEFTBOTTOM||dir1==TC_DIRECTION_LEFTTOP)&&iEnemy_1.b_snatch_ghostball)
					   {
						  if(Tc_CanBeSpelled(hHerowith,hEnemy_1))   
									{ 
									  Tc_Spell(hHerowith,hEnemy_1);  
									}
						  else
						  {
					     if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_1))   
									{ 
									  Tc_Spell(hHeroWithoutBall,hEnemy_1);  
									} 
						  }
					   }
					   if((dir2==TC_DIRECTION_LEFT||dir2==TC_DIRECTION_LEFTBOTTOM||dir2==TC_DIRECTION_LEFTTOP)&&iEnemy_2.b_snatch_ghostball)
					   {
						     if(Tc_CanBeSpelled(hHerowith,hEnemy_2))   
									{ 
									  Tc_Spell(hHerowith,hEnemy_2);  
									} 
                             /*if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2))   
									{ 
									  Tc_Spell(hHeroWithoutBall,hEnemy_2);  
									}*/ 
					   }
				   }
}
bool pass(TC_Handle hwithball,TC_Hero* iwithball,TC_Hero* iwithoutball)
{
   
	bool  flag=false;
    TC_Position  position;
    TC_Position  temp1;
    TC_Position  temp2;
	int           PassDir=0;
	TC_Direction  dir1=GetHeroDirection(iwithball,&iEnemy_1);
    TC_Direction  dir2=GetHeroDirection(iwithball,&iEnemy_2);
if(iAttackDirection == TC_DIRECTION_RIGHT)  
	{
	temp1.x=iForbiddenArea.right.left-65;
	temp1.y=(iForbiddenArea.right.left-65-iGhostBall.pos.x)+iGhostBall.pos.y;
	temp2.x=iForbiddenArea.right.left-65;
	temp2.y=iGhostBall.pos.y-(iForbiddenArea.right.left-65-iGhostBall.pos.x);
if((dir1==TC_DIRECTION_LEFT||dir1==TC_DIRECTION_LEFTTOP||dir1==TC_DIRECTION_LEFTBOTTOM)&&
   (dir2==TC_DIRECTION_LEFT||dir2==TC_DIRECTION_LEFTTOP||dir2==TC_DIRECTION_LEFTBOTTOM))
   {
          PassDir=1;
   }
   if((dir1==TC_DIRECTION_LEFTTOP||dir1==TC_DIRECTION_LEFT||dir1==TC_DIRECTION_LEFTBOTTOM)&&
       (dir2==TC_DIRECTION_RIGHTTOP))
   {
	   PassDir=1;
   }
    if((dir2==TC_DIRECTION_LEFT||dir2==TC_DIRECTION_LEFTTOP||dir2==TC_DIRECTION_LEFTBOTTOM)&&
       (dir1==TC_DIRECTION_RIGHTTOP))
   {
   PassDir=1;
   }
    if((dir1==TC_DIRECTION_LEFTTOP||dir1==TC_DIRECTION_LEFT||dir1==TC_DIRECTION_LEFTTOP||dir1==TC_DIRECTION_LEFTBOTTOM)&&
      (dir2==TC_DIRECTION_RIGHTBOTTOM))
   {
	   PassDir=1;
   }
	if((dir2==TC_DIRECTION_LEFT||dir2==TC_DIRECTION_LEFTTOP||dir2==TC_DIRECTION_LEFTBOTTOM)&&
          (dir1==TC_DIRECTION_RIGHTBOTTOM))
   {
	   PassDir=1;
   }
  if(dir2==TC_DIRECTION_RIGHTTOP&&dir1==TC_DIRECTION_RIGHTTOP)
   {
	   PassDir=1;
   }
if((dir1==TC_DIRECTION_LEFTTOP||dir1==TC_DIRECTION_LEFT||dir1==TC_DIRECTION_LEFTBOTTOM)&&
   (dir2==TC_DIRECTION_RIGHT))
{
 PassDir=2;
}
if((dir2==TC_DIRECTION_LEFTTOP||dir2==TC_DIRECTION_LEFT||dir2==TC_DIRECTION_LEFTBOTTOM)&&
   (dir1==TC_DIRECTION_RIGHT))
{
 PassDir=2;
}
if((dir2==TC_DIRECTION_RIGHT&&dir1==TC_DIRECTION_RIGHTTOP)||(dir1==TC_DIRECTION_RIGHT&&dir2==TC_DIRECTION_RIGHTTOP))
   {
   PassDir=2;
   }
if(dir2==TC_DIRECTION_RIGHTBOTTOM&&dir1==TC_DIRECTION_RIGHTBOTTOM)
   {
   PassDir=1;
   }
if((dir2==TC_DIRECTION_RIGHT&&dir1==TC_DIRECTION_RIGHTBOTTOM)||(dir1==TC_DIRECTION_RIGHT&&dir2==TC_DIRECTION_RIGHTBOTTOM))
   {
   PassDir=3;
   }
if((dir2==TC_DIRECTION_RIGHT&&dir1==TC_DIRECTION_RIGHT))
{
    PassDir=2;
}
if((dir1==TC_DIRECTION_RIGHTBOTTOM&&dir2==TC_DIRECTION_RIGHTTOP)||(dir2==TC_DIRECTION_RIGHTBOTTOM&&dir1==TC_DIRECTION_RIGHTTOP))
{
  PassDir=1;	    
}
switch(PassDir)
{
  case 1:
 if(iForbiddenArea.right.left-iGhostBall.pos.x<576)  //水平传球
		 {
	     position.x=iForbiddenArea.right.left-65;
         position.y=iGhostBall.pos.y;
		}
	   else 
	   {
        position.x=iGhostBall.pos.x+511;
        position.y=iGhostBall.pos.y;
	   }
   if(iwithball->pos.x<iForbiddenArea.right.left-200&&(!iEnemy_1.b_snatch_ghostball&&!iEnemy_2.b_snatch_ghostball))
		 {
        if(Tc_PassBall(hwithball,position))    
		{
			flag=true;
		}
		 }
        break;
  case  2:
if(GetDianDistance(temp1,iGhostBall.pos)<511)//斜向下传球
	{
    position.x=temp1.x;
    position.y=temp1.y;
	}
	else
	{
        position.x=iGhostBall.pos.x+360;
		position.y=iGhostBall.pos.y+256;
	}
     if(iwithball->pos.x<iForbiddenArea.right.left-200&&(!iEnemy_1.b_snatch_ghostball&&!iEnemy_2.b_snatch_ghostball))
		 {
        if(Tc_PassBall(hwithball,position))    
		{
			flag=true;
		}
		 }
   break;
  case 3:
if(GetDianDistance(temp2,iGhostBall.pos)<511)   //斜向上传球 
		{
		position.x=temp2.x;
		position.y=temp2.y;
		}
		else 
		{
			position.x=iGhostBall.pos.x+360;
			position.y=iGhostBall.pos.y-256;
		}
		 if(iwithball->pos.x<iForbiddenArea.right.left-200&&(!iEnemy_1.b_snatch_ghostball&&!iEnemy_2.b_snatch_ghostball))
			 {
			if(Tc_PassBall(hwithball,position))    
			{
			flag=true;
		}
			 }
   break;
 default:
  break;
}
	}
/*       球门在左时       */
	else                       
	{
		 temp1.x=iForbiddenArea.left.right+1;
		temp1.y=(iForbiddenArea.left.right+1-iGhostBall.pos.x)+iGhostBall.pos.y;
		temp2.x=iForbiddenArea.left.right+1;
		temp2.y=iGhostBall.pos.y-(iGhostBall.pos.x-iForbiddenArea.left.right+1);	
if((dir1==TC_DIRECTION_RIGHT||dir1==TC_DIRECTION_RIGHTTOP||dir1==TC_DIRECTION_RIGHTBOTTOM)&&
   (dir2==TC_DIRECTION_RIGHT||dir2==TC_DIRECTION_RIGHTTOP||dir2==TC_DIRECTION_RIGHTBOTTOM))
   {
          PassDir=1;
   }
   if((dir1==TC_DIRECTION_RIGHTTOP||dir1==TC_DIRECTION_RIGHT||dir1==TC_DIRECTION_RIGHTBOTTOM)&&
       (dir2==TC_DIRECTION_LEFTTOP))
   {
         PassDir=1;
   }
    if((dir2==TC_DIRECTION_RIGHT||dir2==TC_DIRECTION_RIGHTTOP||dir2==TC_DIRECTION_RIGHTBOTTOM)&&
       (dir1==TC_DIRECTION_LEFTTOP))
   {
   PassDir=1;
   }
    if((dir1==TC_DIRECTION_RIGHTTOP||dir1==TC_DIRECTION_RIGHT||dir1==TC_DIRECTION_RIGHTTOP||dir1==TC_DIRECTION_RIGHTBOTTOM)&&
      (dir2==TC_DIRECTION_LEFTBOTTOM))
   {
     PassDir=1;
   }
	if((dir2==TC_DIRECTION_RIGHT||dir2==TC_DIRECTION_RIGHTTOP||dir2==TC_DIRECTION_RIGHTBOTTOM)&&
          (dir1==TC_DIRECTION_LEFTBOTTOM))
   {
     PassDir=1;
   }
  if(dir2==TC_DIRECTION_LEFTTOP&&dir1==TC_DIRECTION_LEFTTOP)
   {
    PassDir=2;
   }
if((dir1==TC_DIRECTION_RIGHTTOP||dir1==TC_DIRECTION_RIGHT||dir1==TC_DIRECTION_RIGHTBOTTOM)&&
   (dir2==TC_DIRECTION_LEFT))
{
PassDir=2;
}
if((dir2==TC_DIRECTION_RIGHTTOP||dir2==TC_DIRECTION_RIGHT||dir2==TC_DIRECTION_RIGHTBOTTOM)&&
   (dir1==TC_DIRECTION_LEFT))
{
PassDir=2;
}
if((dir2==TC_DIRECTION_LEFT&&dir1==TC_DIRECTION_LEFTTOP)||(dir1==TC_DIRECTION_LEFT&&dir2==TC_DIRECTION_LEFTTOP))
   {
    PassDir=2;
   }
if(dir2==TC_DIRECTION_LEFTBOTTOM&&dir1==TC_DIRECTION_LEFTBOTTOM)
   {
    PassDir=3;
   }
if((dir2==TC_DIRECTION_LEFT&&dir1==TC_DIRECTION_LEFTBOTTOM)||(dir1==TC_DIRECTION_LEFT&&dir2==TC_DIRECTION_LEFTBOTTOM))
   {
    PassDir=3;
   }
if((dir2==TC_DIRECTION_LEFT&&dir1==TC_DIRECTION_LEFT))
{
    PassDir=2;
}
if((dir1==TC_DIRECTION_LEFTBOTTOM&&dir2==TC_DIRECTION_LEFTTOP)||(dir2==TC_DIRECTION_LEFTBOTTOM&&dir1==TC_DIRECTION_LEFTTOP))
{
  PassDir=1;	    
}
switch(PassDir)
{
  case 1:
if(iGhostBall.pos.x-iForbiddenArea.left.right<512) //水平传球
		 {
	     position.x=iForbiddenArea.left.right;
         position.y=iGhostBall.pos.y;
		 }
		 else  
		 {
            position.y=iGhostBall.pos.y;
            position.x=iGhostBall.pos.x-511;
		 }
        if(iwithball->pos.x>iForbiddenArea.left.right+128&&(!iEnemy_1.b_snatch_ghostball&&!iEnemy_2.b_snatch_ghostball))
		 {
        if(Tc_PassBall(hwithball,position))     
			{
			flag=true;
		}
		 }  
 break;
  case  2:
if(GetDianDistance(temp1,iGhostBall.pos)<511)   //斜向下传球
	   {
			position.x=temp1.x;
			position.y=temp1.y;
	   }
	   else
	   {
        position.x=iGhostBall.pos.x-360;
		position.y=iGhostBall.pos.y+300;
	   }
    if(iwithball->pos.x>iForbiddenArea.left.right+128&&(!iEnemy_1.b_snatch_ghostball&&!iEnemy_2.b_snatch_ghostball))
		 {
        if(Tc_PassBall(hwithball,position))     
			{
			flag=true;
		    }
		 }
   break;
  case 3:
 if(GetDianDistance(temp2,iGhostBall.pos)<511)  //斜向上传球
	{
    position.x=temp2.x;
    position.y=temp2.y;
	}
	else 
	{
      position.x=iGhostBall.pos.x-360;
      position.y=iGhostBall.pos.y-300;
	}
    if(iwithball->pos.x>iForbiddenArea.left.right+128&&(!iEnemy_1.b_snatch_ghostball&&!iEnemy_2.b_snatch_ghostball))
		 {
        if(Tc_PassBall(hwithball,position))     
			{
			flag=true;
		    }
		 }
   break;
 default:
  break;
}
	}
   return flag;
}
void  CrossFire(TC_Handle hero)
{
   if(GetIattack()==TC_DIRECTION_RIGHT)
   {
     if(GetHeroDirection(&iHeroWithBall,&iEnemy_1)==TC_DIRECTION_RIGHT&&GetHeroDirection(&iHeroWithBall,&iEnemy_2)==TC_DIRECTION_RIGHT)
		{
            Tc_Move(hero,TC_DIRECTION_RIGHTTOP);
			Tc_Move(hero,TC_DIRECTION_RIGHTBOTTOM);
		}
   }
   else
   {
     if(GetHeroDirection(&iHeroWithBall,&iEnemy_1)==TC_DIRECTION_LEFT&&GetHeroDirection(&iHeroWithBall,&iEnemy_2)==TC_DIRECTION_LEFT)
		 {
              Tc_Move(hero,TC_DIRECTION_RIGHTTOP);
			  Tc_Move(hero,TC_DIRECTION_RIGHTBOTTOM);
		 }
   }
}
void  Ron()
{
if(flags!=0)  
	 {
             TC_Position  tag;
	          switch(flags)
			    {
			  case 1:
				  if(iAttackDirection == TC_DIRECTION_RIGHT&&iGhostBall.pos.x>1500&&(iHero_1.b_ghostball||iHero_2.b_ghostball))
				  {
				    if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						}
					else
					{
                    if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						}
					}
				  }
				  if(iAttackDirection == TC_DIRECTION_LEFT&&iGhostBall.pos.x<500&&(iHero_1.b_ghostball||iHero_2.b_ghostball))
				  {
                    if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						}
					else
					{
                    if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						}
					}
				  }
	 if(iEnemy_1.b_is_spelling) 
		{
			       if(iHero_1.b_ghostball)
					 {
						 tag.x=iHero_2.pos.x;
						 tag.y=iHero_2.pos.y; 
					     if(Tc_PassBall(hHero_1,tag)) 
						 {
							    if(iHero_2.b_snatch_ghostball)
								   {
									if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) 
									{
				     				//	Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
										break;
									}
								}
								else
								{
								  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								}
					  }
					else 
					{
							if(iAttackDirection == TC_DIRECTION_RIGHT)
							 {
							 if(iHero_1.pos.y<=iForbiddenArea.right.top-128)
							 {
							 tag.x=iHero_1.pos.x-1;
							 tag.y=iForbiddenArea.right.top-65;
							 }
							 else
							 {
                               tag.x=iHero_1.pos.x-1;
							   tag.y=iForbiddenArea.right.bottom+1;
							 }
							 }
							 else  
							 {
                             if(iHero_1.pos.y<=iForbiddenArea.left.top-128)
							 {
                               tag.x=iEnemyGate.x+1;
                               tag.y=iForbiddenArea.left.top-65;
							 }
							 else
							 {
							 tag.x=iEnemyGate.x+1;
							 tag.y=iForbiddenArea.left.bottom+1;
							 }
							 }
						   if(Tc_PassBall(hHero_1,tag))  
						   {
							 Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							 Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						   }
					}
					 }
                  if(iHero_2.b_ghostball)  
					  {
                          tag.x=iHero_1.pos.x;
						  tag.y=iHero_1.pos.y; 
			              if(Tc_PassBall(hHero_2,tag)) 
						  {
						           if(iHero_1.b_snatch_ghostball)
								   {
									if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
									{
										break;
									}
								   }
								else
								{
								  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								}
						  }
					  else
					  {
                       if(iAttackDirection == TC_DIRECTION_RIGHT)
						 {
						    if(iHero_2.pos.x<=iForbiddenArea.right.top-128)
							 {
							 tag.x=iHero_2.pos.x-1;
							 tag.y=iForbiddenArea.right.bottom+1;
							 }
							 else
							 {
                               tag.x=iHero_2.pos.x-1;
							   tag.y=iForbiddenArea.right.top-65;
							 }
						 }
						 else  
						 {
                         if(iHero_2.pos.x<=iForbiddenArea.left.top-128)
							 {
                               tag.x=iEnemyGate.x+1;
                               tag.y=iForbiddenArea.left.top-65;
							 }
							 else
							 {
							 tag.x=iEnemyGate.x+1;
							 tag.y=iForbiddenArea.left.bottom+1;
							 }
						 }
						 if(Tc_PassBall(hHero_2,tag)) 
						 {
                          Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						 }
					  }
				  }
				  }
				  break;
			  case 2:
                   if(iAttackDirection == TC_DIRECTION_RIGHT&&iGhostBall.pos.x>1500&&(iHero_1.b_ghostball||iHero_2.b_ghostball))
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
				  if(iAttackDirection == TC_DIRECTION_LEFT&&iGhostBall.pos.x<500&&(iHero_1.b_ghostball||iHero_2.b_ghostball))
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
				  if(iEnemy_2.b_is_spelling) 
				    {
					   if(iHero_1.b_ghostball)   
					   {
						   tag.x=iHero_2.pos.x;
						  tag.y=iHero_2.pos.y; 
					      if(Tc_PassBall(hHero_1,tag)) 
						  {
						          if(iHero_2.b_snatch_ghostball)
								   {
									if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) 
									{
				     				//	Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
										break;
									}
								  }
								else
								{
								  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								}
						  }
                     else
					  {
                       if(iAttackDirection == TC_DIRECTION_RIGHT)
							 {
							 if(iHero_1.pos.y<=iForbiddenArea.right.top-128)
							 {
							 tag.x=iHero_1.pos.x-1;
							 tag.y=iForbiddenArea.right.top-65;
							 }
							 else
							 {
                               tag.x=iHero_1.pos.x-1;
							   tag.y=iForbiddenArea.right.bottom+1;
							 }
							 }
							 else  
							 {
                             if(iHero_1.pos.y<=iForbiddenArea.left.top-128)
							 {
                               tag.x=iEnemyGate.x+1;
                               tag.y=iForbiddenArea.left.top-65;
							 }
							 else
							 {
							 tag.x=iEnemyGate.x+1;
							 tag.y=iForbiddenArea.left.bottom+1;
							 }
							 }
						 if(Tc_PassBall(hHero_1,tag)) 
						 {
                          Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						 }
					  }
					   }
                  if(iHero_2.b_ghostball)  
					  {
						  tag.x=iHero_1.pos.x;
						 tag.y=iHero_1.pos.y; 
					  if(Tc_PassBall(hHero_2,tag)) 
					  {
						          if(iHero_1.b_snatch_ghostball)
								   {
									if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
									{
				     					Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
										break;
									}
								  }
								else
								{ 
								  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
								  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
								}
					  }
					  else 
						{
                         if(iAttackDirection == TC_DIRECTION_RIGHT)
						 {
						    if(iHero_2.pos.x<=iForbiddenArea.right.top-128)
							 {
							 tag.x=iHero_2.pos.x-1;
							 tag.y=iForbiddenArea.right.bottom+1;
							 }
							 else
							 {
                               tag.x=iHero_2.pos.x-1;
							   tag.y=iForbiddenArea.right.top-65;
							 }
						 }
						 else  
						 {
                         if(iHero_2.pos.x<=iForbiddenArea.left.top-128)
							 {
                               tag.x=iEnemyGate.x+1;
                               tag.y=iForbiddenArea.left.top-65;
							 }
							 else
							 {
							 tag.x=iEnemyGate.x+1;
							 tag.y=iForbiddenArea.left.bottom+1;
							 }
						 }
						   if(Tc_PassBall(hHero_2,tag))  
						   {
							 Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							 Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						   }
					  }
				}
				  }
                  break;
			  default:
				  break;
	          }
}
}
void Ginny()
{
if(flag!=0)  
	 {
             TC_Position  tag;
	          switch(flag)
			    {
			  case 1:
	 if(iEnemy_1.b_is_spelling) 
		{
			       if(iHero_1.b_ghostball)
					 {
							 if(iAttackDirection == TC_DIRECTION_RIGHT)
							 {
							 if(iHero_1.pos.y<=iForbiddenArea.right.top-128)
							 {
							 tag.x=iHero_1.pos.x-1;
							 tag.y=iForbiddenArea.right.top-65;
							 }
							 else
							 {
                               tag.x=iHero_1.pos.x-1;
							   tag.y=iForbiddenArea.right.bottom+1;
							 }
							 }
							 else  
							 {
                             if(iHero_1.pos.y<=iForbiddenArea.left.top-128)
							 {
                               tag.x=iEnemyGate.x+1;
                               tag.y=iForbiddenArea.left.top-65;
							 }
							 else
							 {
							 tag.x=iEnemyGate.x+1;
							 tag.y=iForbiddenArea.left.bottom+1;
							 }
							 }
						   if(Tc_PassBall(hHero_1,tag))  
						   {
							 Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							 Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						   }
					 }
                  if(iHero_2.b_ghostball)  
					  {
                       if(iAttackDirection == TC_DIRECTION_RIGHT)
						 {
						    if(iHero_2.pos.x<=iForbiddenArea.right.top-128)
							 {
							 tag.x=iHero_2.pos.x-1;
							 tag.y=iForbiddenArea.right.bottom+1;
							 }
							 else
							 {
                               tag.x=iHero_2.pos.x-1;
							   tag.y=iForbiddenArea.right.top-65;
							 }
						 }
						 else  
						 {
                         if(iHero_2.pos.x<=iForbiddenArea.left.top-128)
							 {
                               tag.x=iEnemyGate.x+1;
                               tag.y=iForbiddenArea.left.top-65;
							 }
							 else
							 {
							 tag.x=iEnemyGate.x+1;
							 tag.y=iForbiddenArea.left.bottom+1;
							 }
						 }
						 if(Tc_PassBall(hHero_2,tag)) 
						 {
                          Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						 }
					  }
				  }
				  break;
			  case 2:
				  if(iEnemy_2.b_is_spelling) 
				    {
					  if(iHero_1.b_ghostball)
					 {
							 if(iAttackDirection == TC_DIRECTION_RIGHT)
							 {
							 if(iHero_1.pos.y<=iForbiddenArea.right.top-128)
							 {
							 tag.x=iHero_1.pos.x-1;
							 tag.y=iForbiddenArea.right.top-65;
							 }
							 else
							 {
                               tag.x=iHero_1.pos.x-1;
							   tag.y=iForbiddenArea.right.bottom+1;
							 }
							 }
							 else  
							 {
                             if(iHero_1.pos.y<=iForbiddenArea.left.top-128)
							 {
                               tag.x=iEnemyGate.x+1;
                               tag.y=iForbiddenArea.left.top-65;
							 }
							 else
							 {
							 tag.x=iEnemyGate.x+1;
							 tag.y=iForbiddenArea.left.bottom+1;
							 }
							 }
						   if(Tc_PassBall(hHero_1,tag))  
						   {
							 Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							 Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						   }
					 }
                 if(iHero_2.b_ghostball)  
					  {
                       if(iAttackDirection == TC_DIRECTION_RIGHT)
						 {
						    if(iHero_2.pos.x<=iForbiddenArea.right.top-128)
							 {
							 tag.x=iHero_2.pos.x-1;
							 tag.y=iForbiddenArea.right.bottom+1;
							 }
							 else
							 {
                               tag.x=iHero_2.pos.x-1;
							   tag.y=iForbiddenArea.right.top-65;
							 }
						 }
						 else  
						 {
                         if(iHero_2.pos.x<=iForbiddenArea.left.top-128)
							 {
                               tag.x=iEnemyGate.x+1;
                               tag.y=iForbiddenArea.left.top-65;
							 }
							 else
							 {
							 tag.x=iEnemyGate.x+1;
							 tag.y=iForbiddenArea.left.bottom+1;
							 }
						 }
						 if(Tc_PassBall(hHero_2,tag)) 
						 {
                          Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						 }
					  }
				}
                  break;
			  default:
				  break;
	          }
}
}
void  IsDangerInNo()
{
	if(iAttackDirection == TC_DIRECTION_RIGHT)  
	 {
	if(iGhostBall.speed.vx<0&&(iEnemy_1.b_snatch_ghostball||iEnemy_2.b_snatch_ghostball))  
	 {  	 
		 if(GetDistanceBall(iEnemy_1,iGhostBall)<GetDistanceBall(iEnemy_2,iGhostBall))
		 {
			 if(iEnemy_1.b_snatch_ghostball)
			 {
			 if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						} 
			 else
			 {
               if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						} 
			 }
			 }
		 }
		 else 
		 {
            if(iEnemy_2.b_snatch_ghostball) 
			 {
              if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_2);  
						} 
			  else
			  {
              if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_2);  
						} 
			  }
			 }  
			
		 }
	 }
	if(iGhostBall.speed.vx>0&&(iEnemy_1.b_snatch_ghostball||iEnemy_2.b_snatch_ghostball))
	{
		if(GetDistanceBall(iEnemy_1,iGhostBall)<GetDistanceBall(iEnemy_2,iGhostBall))
		 {
			 if(iEnemy_1.b_snatch_ghostball)
			 {
			 if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						} 
               if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						} 
			 }
		 }
		 else 
		 {
            if(iEnemy_2.b_snatch_ghostball) 
			 {
              if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_2);  
						}
			  else
			  {
             if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_2);  
						}
			  }
			 }  
		 }
	}
	 }
	 else 
	 {
		if(iGhostBall.speed.vx>0&&(iEnemy_1.b_snatch_ghostball||iEnemy_2.b_snatch_ghostball))   
	   {
		if(GetDistanceBall(iEnemy_1,iGhostBall)<GetDistanceBall(iEnemy_2,iGhostBall))
		 {
			  if(iEnemy_1.b_snatch_ghostball)
			 {
			  if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						} 
			  else
			  {
               if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						}
			  }
			 }
		 }
		 else 
		 {
            if(iEnemy_2.b_snatch_ghostball) 
			 {
              if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_2);  
						} 
			  else
			  {
             if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_2);  
						} 
			  }
			 }  
			
		 }
	   }
		if(iGhostBall.speed.vx<0&&(iEnemy_1.b_snatch_ghostball||iEnemy_2.b_snatch_ghostball))
		{
         if(GetDistanceBall(iEnemy_1,iGhostBall)<GetDistanceBall(iEnemy_2,iGhostBall))
		 {
			  if(iEnemy_1.b_snatch_ghostball)
			 {
			 if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						} 
			 else
			 {
               if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						} 
			 }
			 }
		 }
		 else 
		 {
            if(iEnemy_2.b_snatch_ghostball) 
			 {
              if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_2);  
						} 
			  else
			  {
               if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_2);  
						} 
			  }
			 }  
		 }
		}
	 }
}
void   IsDangerInEnemy(TC_Hero withball,TC_Hero withoutball,TC_Handle  hwithball,TC_Handle hwithoutball)
{
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		  if(iGhostBall.pos.x<600)
		  {
			  if(GetDistance(iHero_1,withball)<=GetDistance(iHero_2,withball))
					   {
			           if(Tc_Spell(hHero_2,hwithball))    
						  {
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
                          Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  }
						  else  
						  {
						   if(Tc_Spell(hHero_1,hwithball))  
						  {
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  } 
						   else
						   {
							  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						   }
						  } 
							    if(Tc_Spell(hHero_2,hEnemyWithoutBall))    ;  
							    else   if(Tc_Spell(hHero_1,hEnemyWithoutBall)) ;  
					   }
					 else
					   {
                        if(Tc_Spell(hHero_1,hwithball))    
						  {
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
                           Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  }
						else  
						  {
						   if(Tc_Spell(hHero_2,hwithball))  
						  {
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  } 
						   else
						   {
							  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						   }
						  } 
							    if(Tc_Spell(hHero_1,hEnemyWithoutBall))    ;  
							    else   if(Tc_Spell(hHero_2,hEnemyWithoutBall)) ;  
                                    }         
		  }
		  if(iGhostBall.pos.x>1400)   
		  {  
 if(GetDistance(iHero_1,withball)<=GetDistance(iHero_2,withball))
					   {
			           if(Tc_Spell(hHero_2,hwithball))    
						  {
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
                          Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  }
						  else  
						  {
						   if(Tc_Spell(hHero_1,hwithball))  
						  {
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  } 
						   else
						   {
							  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						   }
						  } 
					   }
					 else
					   {
                        if(Tc_Spell(hHero_1,hwithball))    
						  {
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
                          Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  }
						else  
						  {
						   if(Tc_Spell(hHero_2,hwithball))  
						  {
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  } 
						   else
						   {
							  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						   }
						  }                                                                              
 }
		  }
			              
	 }
	else 
	{
          if(iGhostBall.pos.x>1400)   
		  {
               if(GetDistance(iHero_1,withball)<=GetDistance(iHero_2,withball))
					   {
			             if(Tc_Spell(hHero_2,hwithball))                                            
						  {
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
                            Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));                                                                                  
						  }
						  else  
						  {
						   if(Tc_Spell(hHero_1,hwithball))  
						  {
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  } 
						   else
						   {
							  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						   }
						  } 
						    if(Tc_Spell(hHero_2,hEnemyWithoutBall))    ;  
							    else   if(Tc_Spell(hHero_1,hEnemyWithoutBall)) ;  
					   }
					 else
					   {
                           if(Tc_Spell(hHero_1,hwithball))                                                                                    
						  {
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
                            Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));                                                                                  
						  }
						else  
						  {
						   if(Tc_Spell(hHero_2,hwithball))  
						  {
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  } 
						   else
						   {
							  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						   }
						  } 
						 
							    if(Tc_Spell(hHero_2,hEnemyWithoutBall))    ;  
							    else   if(Tc_Spell(hHero_1,hEnemyWithoutBall)) ;  
						    
                       }        	 
			         
		  }
		  if(iGhostBall.pos.x<600)
		  {
			    if(GetDistance(iHero_1,withball)<=GetDistance(iHero_2,withball))
					   {
			              if(Tc_Spell(hHero_2,hwithball))                                           
						  {
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
                           Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));                                                                                   
						  }
						  else  
						  {
						   if(Tc_Spell(hHero_1,hwithball))  
						  {
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  } 
						   else
						   {
							  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						   }
						  } 
					   }
					 else
					   {
                          if(Tc_Spell(hHero_1,hwithball))                                                                                     
						  {
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
                            Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));                                                                                  
						  }
						else  
						  {
						   if(Tc_Spell(hHero_2,hwithball))  
						  {
						  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
						  } 
						   else
						   {
							  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
							  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						   }
						  } 
	
                }
					
		  }
   }
}
void   IsDangerInSelf()
{  
	if(iAttackDirection == TC_DIRECTION_RIGHT)
	{
	if(iGhostBall.pos.x>1400) 
	{	
		   if(iEnemy_2.abnormal_type!=TC_SPELLED_BY_NONE&&iEnemy_1.abnormal_type!=TC_SPELLED_BY_NONE)  ;
			else  
			{
					 if(flag!=0)
					 {
						 switch(flag)
						 {
						 case 1:
                              Tc_Spell(hHeroWithoutBall,hEnemy_1);
							 break;
						 case 2:
                             Tc_Spell(hHeroWithoutBall,hEnemy_2);
							 break;
						 default:
							 break;
						 }
					 }
					 if(iEnemy_2.b_can_spell||iEnemy_1.b_can_spell) 
					 {
				       if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_1))   
									{ 
									  Tc_Spell(hHeroWithBall,hEnemy_1);  
									} 
					   if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2))   
									{ 
									  Tc_Spell(hHeroWithoutBall,hEnemy_2);  
									} 
					  if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_1))   
									{ 
									  Tc_Spell(hHeroWithoutBall,hEnemy_1);  
									} 
					  if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_2))   
									{ 
									  Tc_Spell(hHeroWithBall,hEnemy_2);  
									} 
					 }
			}
          if(iEnemy_2.b_can_spell||iEnemy_1.b_can_spell) 
		  {
			if(GetDistance(iEnemy_1,iHeroWithBall)<400)   
			 {  
				if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_1))   
								{ 
								  Tc_Spell(hHeroWithBall,hEnemy_1);  
								} 
				if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_1))   
								{ 
								  Tc_Spell(hHeroWithoutBall,hEnemy_1);  
								} 
			}
			if(GetDistance(iEnemy_2,iHeroWithBall)<400)
			{
				if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_2))   
								{ 
								  Tc_Spell(hHeroWithBall,hEnemy_2);  
								} 
				if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2))   
								{ 
								  Tc_Spell(hHeroWithoutBall,hEnemy_2);  
								} 
				
			}
			if(GetDistance(iEnemy_1,iHeroWithBall)<GetDistance(iEnemy_2,iHeroWithBall)&&iEnemy_1.b_snatch_ghostball)
			 {
					 if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
								{ 
								  Tc_Spell(hHero_2,hEnemy_1);  
								} 
					 else
					 {
					 if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
								{ 
								  Tc_Spell(hHero_1,hEnemy_1);  
								} 
					 }
			}
			else 
			{
					 if(iEnemy_2.b_snatch_ghostball)
					 {
				        if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
								{ 
								  Tc_Spell(hHero_2,hEnemy_2);  
								}
						else
						{
					    if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
								{ 
								  Tc_Spell(hHero_1,hEnemy_2);  
								} 
						}
					 }

			}
		  }
	}
	}
	else  
	{
             if(iGhostBall.pos.x<600)
			 {
			if(iEnemy_2.abnormal_type!=TC_SPELLED_BY_NONE&&iEnemy_1.abnormal_type!=TC_SPELLED_BY_NONE)  ;
			else  
			{
					 if(flag!=0)
					 {
						 switch(flag)
						 {
						 case 1:
                              Tc_Spell(hHeroWithoutBall,hEnemy_1);
							 break;
						 case 2:
                             Tc_Spell(hHeroWithoutBall,hEnemy_2);
							 break;
						 default:
							 break;
						 }
					 }
					 if(iEnemy_2.b_can_spell||iEnemy_1.b_can_spell) 
					 {
				       if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_1))   
									{ 
									  Tc_Spell(hHeroWithBall,hEnemy_1);  
									} 
					   if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2))   
									{ 
									  Tc_Spell(hHeroWithoutBall,hEnemy_2);  
									} 
					  if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_1))   
									{ 
									  Tc_Spell(hHeroWithoutBall,hEnemy_1);  
									} 
					  if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_2))   
									{ 
									  Tc_Spell(hHeroWithBall,hEnemy_2);  
									} 
					 }
			}
          if(iEnemy_2.b_can_spell||iEnemy_1.b_can_spell) 
		  {
			if(GetDistance(iEnemy_1,iHeroWithBall)<400)   
			 {  
				if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_1))   
								{ 
								  Tc_Spell(hHeroWithBall,hEnemy_1);  
								} 
				else
				{

				if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_1))   
								{ 
								  Tc_Spell(hHeroWithoutBall,hEnemy_1);  
								} 
				}
			}
			if(GetDistance(iEnemy_2,iHeroWithBall)<400)
			{
				if(Tc_CanBeSpelled(hHeroWithBall,hEnemy_2))   
								{ 
								  Tc_Spell(hHeroWithBall,hEnemy_2);  
								} 
				else
				{
				if(Tc_CanBeSpelled(hHeroWithoutBall,hEnemy_2))   
								{ 
								  Tc_Spell(hHeroWithoutBall,hEnemy_2);  
								} 
				}
				
			}
			if(GetDistance(iEnemy_1,iHeroWithBall)<GetDistance(iEnemy_2,iHeroWithBall)&&iEnemy_1.b_snatch_ghostball)
			 {
					 if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
								{ 
								  Tc_Spell(hHero_2,hEnemy_1);  
								} 
					 else
					 {
					 if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
								{ 
								  Tc_Spell(hHero_1,hEnemy_1);  
								} 
					 }
			}
			else 
			{
					 if(iEnemy_2.b_snatch_ghostball)
					 {
				        if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
								{ 
								  Tc_Spell(hHero_2,hEnemy_2);  
								}
						else
						{
					    if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
								{ 
								  Tc_Spell(hHero_1,hEnemy_2);  
								} 
						}
					 }

			}
		  }
	}
		}
}
void  GetGhostBall()
{ 	 
int state=GetState();
switch(state)
{
	case GHOSTBALL_IN_SELF_TEAM:
		if(iAttackDirection == TC_DIRECTION_RIGHT)
		{
            IsDangerInSelf();
        if(iHeroWithBall.abnormal_type==TC_SPELLED_BY_GINNY)  
		{
			break;
		}
		CrossFire(hHeroWithBall);
        Spell(hHeroWithBall,iHeroWithBall,hHeroWithoutBall,iHeroWithoutBall);
		if(pass(hHeroWithBall,&iHeroWithBall,&iHeroWithoutBall))  
		{ 
			
			break;
		}
		else  Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
		if(iHeroWithBall.abnormal_type!=TC_SPELLED_BY_NONE||iEnemy_1.b_is_spelling||iEnemy_2.b_is_spelling)
			{
				if( iHeroWithBall.b_ghostball)
				{
				  if(iHeroWithoutBall.b_snatch_ghostball)
					{
						if(Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL)==true) 
						{
				     	    Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
							break;
						}
					 }
                   else  
					 {
					   Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHeroWithBall));
					 }
				}
				else   
				{
					break;
				}
			}
			else
			{
	          if(GetDistance(iHeroWithBall,iHeroWithoutBall)<128&&iHeroWithBall.pos.x-iHeroWithoutBall.pos.x>0)  Tc_Stop(hHeroWithoutBall);
		      else  Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
			}
			break;
		}
		else
		{ 
			IsDangerInSelf();
		 if(iHeroWithBall.abnormal_type==TC_SPELLED_BY_GINNY)
		 {
			 break;
		 }
			CrossFire(hHeroWithBall);
Spell(hHeroWithBall,iHeroWithBall,hHeroWithoutBall,iHeroWithoutBall);
		    if(pass(hHeroWithBall,&iHeroWithBall,&iHeroWithoutBall))    
			{
	           

				break;
			}
			else Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			if(iHeroWithBall.abnormal_type!=TC_SPELLED_BY_NONE||iEnemy_1.b_is_spelling||iEnemy_2.b_is_spelling)
			{
             if(iHeroWithBall.b_ghostball)  
			 {
			if (iHeroWithoutBall.b_snatch_ghostball)
			{
				if(Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL)==true) 
				{
					Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
					break;
				}
			}
			 else  
			 {
	           Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall,&iHeroWithBall));
			 }
			}
			 else    
			 {
                Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall)); 
                Tc_Move(hHeroWithBall,GetBallDirection(&iHeroWithBall,&iGhostBall)); 
				break;
			 }
		   }
		   
		   else
		   {
		   if(GetDistance(iHeroWithBall,iHeroWithoutBall)<128&&iHeroWithoutBall.pos.x-iHeroWithBall.pos.x>0)  Tc_Stop(hHeroWithoutBall);
		   else  Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall,&iGhostBall));
		   }
			break;
			}
	case  GHOSTBALL_IN_NO_TEAM:	
IsDangerInNo();
if(GetDistanceBall(iHero_1,iGhostBall)<=GetDistanceBall(iHero_2,iGhostBall))
	{
	     if(iHero_1.abnormal_type==TC_SPELLED_BY_NONE)
			{
				if(iHero_1.b_snatch_ghostball)
		          {
				       if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
				             {
								  CrossFire(hHero_1);
								  Spell(hHero_1,iHero_1,hHero_2,iHero_2);
			                      if(pass(hHero_1,&iHero_1,&iHero_2))  
					              { 
						           break;
					               }
					      break;
			                }
				}
					if (iHero_2.b_snatch_ghostball)
					{
				
							if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true)
							{   
								CrossFire(hHero_2);
								Spell(hHero_2,iHero_2,hHero_1,iHero_1);
								if(pass(hHero_2,&iHero_2,&iHero_1))  
								{ 
									
									break;
								}
								break;
							}	
					}
			            Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
		}
			   else
			   {
                           if (iHero_2.b_snatch_ghostball)
			                    {
				               if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true)
				                      {
										CrossFire(hHero_2);
										Spell(hHero_2,iHero_2,hHero_1,iHero_1);
										  if(pass(hHero_2,&iHero_2,&iHero_1))  
										{ 	
											
											break;
										}
								break;
							          }
						         }
						   if(iHero_1.b_snatch_ghostball)
		                      {
				                   if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
				                         {
											CrossFire(hHero_1);
												Spell(hHero_1,iHero_1,hHero_2,iHero_2);							 
											 if(pass(hHero_1,&iHero_1,&iHero_2))  
											  { 	
											   	
											   break;
											   }
					                         break;
			                             }
				             }
                        Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			   }
     }
else
	{
					if(iHero_2.abnormal_type==TC_SPELLED_BY_NONE) 
							{
							if (iHero_2.b_snatch_ghostball)
							    {
									   if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true)
										{
											CrossFire(hHero_2);
											Spell(hHero_2,iHero_2,hHero_1,iHero_1);
											if(pass(hHero_2,&iHero_2,&iHero_1))  
												{ 
													
													break;
												}
										       break;
										}
							    } 
						  if(iHero_1.b_snatch_ghostball)
					        {
								if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
								{
									CrossFire(hHero_1);
									Spell(hHero_1,iHero_1,hHero_2,iHero_2);
									if(pass(hHero_1,&iHero_1,&iHero_2))  
									{ 
										
										break;
									}
									break;
								}
						   }
                        Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
					    Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							}
					else  
					{
                      if(iHero_1.b_snatch_ghostball)
					        {
								if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
								{
									CrossFire(hHero_1);
									Spell(hHero_1,iHero_1,hHero_2,iHero_2);
									if(pass(hHero_1,&iHero_1,&iHero_2))  
									{ 
										
										break;
									}
									break;
								}
						   }
					  if(iHero_2.b_snatch_ghostball)
					        {
								if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) 
								{
									CrossFire(hHero_2);
									Spell(hHero_2,iHero_2,hHero_1,iHero_1);
									if(pass(hHero_2,&iHero_2,&iHero_1))  
									{ 
										
										break;
									}
									break;
								}
						   }
					   Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					   Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));  
					}
			}
        IsDangerInNo();
			break;
	case GHOSTBALL_IN_ENEMY_TEAM:
		  IsDangerInEnemy(iEnemyWithBall,iEnemyWithoutBall,hEnemyWithBall,hEnemyWithoutBall);
		  if(flags!=0)
		  {
            switch(flags)
			    {
			  case 1:
				  if(iAttackDirection == TC_DIRECTION_RIGHT&&iGhostBall.pos.x<500)
				  {
				    if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						}
					else
					{
                    if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						}
					}
				  }
				  if(iAttackDirection == TC_DIRECTION_LEFT&&iGhostBall.pos.x>1500)
				  {
                    if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						}
					else
					{
                    if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						}
					}
				  } 
				  break;
				  case 2:
                   if(iAttackDirection == TC_DIRECTION_RIGHT&&iGhostBall.pos.x<500)
				  {
				    if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_2);  
						}
					else
					{
                    if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_2);  
						}
					}
				  }
				  if(iAttackDirection == TC_DIRECTION_LEFT&&iGhostBall.pos.x>1500)
				  {
                    if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_2);  
						}
					else
					{
                    if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_2);  
						}
					}
				  }
				  break;
				  default:
					  break;
			}
		  }
if(GetDistanceBall(iHero_1,iGhostBall)<=GetDistanceBall(iHero_2,iGhostBall))
{     
	   if(iHero_1.abnormal_type==TC_SPELLED_BY_NONE)
			{
				if(iHero_1.b_snatch_ghostball)
		          {
				       if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
				             {
			                     Spell(hHero_1,iHero_1,hHero_2,iHero_2);
								 if(pass(hHero_1,&iHero_1,&iHero_2))  
					              { 
						           break;
					               }
			                }
				}
					if (iHero_2.b_snatch_ghostball)
					{
				
							if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true)
							{
								Spell(hHero_2,iHero_2,hHero_1,iHero_1);
								if(pass(hHero_2,&iHero_2,&iHero_1))  
								{ 
									break;
								}
                      // Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
							//	break;
							}	
					}
			            Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
		}
			   else
			   {
                           if (iHero_2.b_snatch_ghostball)
			                    {
				               if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true)
				                      {
                                        Spell(hHero_2,iHero_2,hHero_1,iHero_1);
										if(pass(hHero_2,&iHero_2,&iHero_1))  
										{ 
											break;
										}
                                   // Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
								        // break;
							          }
						         }
						   if(iHero_1.b_snatch_ghostball)
		                      {
				                   if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
				                         {
											 Spell(hHero_1,iHero_1,hHero_2,iHero_2);
											 if(pass(hHero_1,&iHero_1,&iHero_2))  
											  { 
											   break;
											   }
											  //Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
					                         //break;
			                             }
				             }
                        Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
						Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
			   }
     }
else
	{
					if(iHero_2.abnormal_type==TC_SPELLED_BY_NONE) 
							{
							if (iHero_2.b_snatch_ghostball)
							    {
									   if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true)
										{
											Spell(hHero_2,iHero_2,hHero_1,iHero_1);
												
											if(pass(hHero_2,&iHero_2,&iHero_1))  
												{ 
													break;
												}
											//	Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
										       //break;
										}
							    } 
						  if(iHero_1.b_snatch_ghostball)
					        {
								if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
								{
									Spell(hHero_1,iHero_1,hHero_2,iHero_2);
									if(pass(hHero_1,&iHero_1,&iHero_2))  
									{ 
										break;
									}
								//	Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
									//break;
								}
						   }
                        Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
					    Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
							}
					else  
					{
                      if(iHero_1.b_snatch_ghostball)
					        {
								if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) 
								{
									Spell(hHero_1,iHero_1,hHero_2,iHero_2);
									if(pass(hHero_1,&iHero_1,&iHero_2))  
									{ 
										break;
									}
								//	Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
									//break;
								}
						   }
					  if(iHero_2.b_snatch_ghostball)
					        {
								if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) 
								{
									Spell(hHero_2,iHero_2,hHero_1,iHero_1);
									if(pass(hHero_2,&iHero_2,&iHero_1))  
									{ 
										break;
									}
								//	Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
								//break;
								}
						   }
					   Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					   Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));  
					}
			}
			IsDangerInEnemy(iEnemyWithBall,iEnemyWithoutBall,hEnemyWithBall,hEnemyWithoutBall);
		  break;
	default: 
		break;
}
}
void  GetGoldBallB()
{
      int state=GetState();
	  if(state==GHOSTBALL_IN_ENEMY_TEAM)  
	  {
               if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						} 
		       if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_2);  
						}
			   if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						}
			   if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_2);  
						}
	  }
	if(iEnemy_1.b_snatch_goldball==true||iEnemy_2.b_snatch_goldball==true)  
		 {
                if(Tc_CanBeSpelled(hHero_2,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_1);  
						} 
		       if(Tc_CanBeSpelled(hHero_1,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_2);  
						}
			   if(Tc_CanBeSpelled(hHero_1,hEnemy_1))   
						{ 
						  Tc_Spell(hHero_1,hEnemy_1);  
						}
			   if(Tc_CanBeSpelled(hHero_2,hEnemy_2))   
						{ 
						  Tc_Spell(hHero_2,hEnemy_2);  
						}
		 }  
		 if(state==GHOSTBALL_IN_SELF_TEAM) 
		 {
           TC_Position  tag;
		   tag.x=iHeroWithBall.pos.x;
		   tag.y=iHeroWithBall.pos.y;
           Tc_PassBall(hHeroWithBall,tag);
		 }
	      if(iHero_1.b_snatch_goldball)
			 {
              Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
			 } 
		   if(iHero_2.b_snatch_goldball)
			 {
              Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
			 }
			if(GetBallDirection(&iHero_1,&iGoldBall)<=GetBallDirection(&iHero_2,&iGoldBall)+20)  
			   {
				  TC_Direction  dir=GetBallDirection(&iHero_1,&iGoldBall);
				  if(iHero_1.b_snatch_goldball)
			 {
                 Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
			 }	
		if(iHero_2.b_snatch_goldball)
			 {
              Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
			 }
				  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
				  if(iGoldBall.pos.x>200&&iGoldBall.pos.x<1748&&iGoldBall.pos.y==0)
				  {
                     
					  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
					  if(dir==TC_DIRECTION_LEFT)   Tc_Move(hHero_2,GetPointDirection(200,0,&iHero_2));
					  else   if(dir==TC_DIRECTION_RIGHT)  Tc_Move(hHero_2,GetPointDirection(1748,0,&iHero_2));
					  else  Tc_Move(hHero_2,GetPointDirection(iGoldBall.pos.x,704,&iHero_2));
				  }
				  if(iGoldBall.pos.x>200&&iGoldBall.pos.x<1748&&iGoldBall.pos.y==704) 
				  {
                     Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
					 if(dir==TC_DIRECTION_LEFT)   Tc_Move(hHero_2,GetPointDirection(200,704,&iHero_2));
					  else   if(dir==TC_DIRECTION_RIGHT)  Tc_Move(hHero_2,GetPointDirection(1748,704,&iHero_2));
					  else   Tc_Move(hHero_2,GetPointDirection(iGoldBall.pos.x,0,&iHero_2));
				  }
				  if(iGoldBall.pos.x==200&&iGoldBall.pos.y>0&&iGoldBall.pos.y<704)
				  {
                      Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
                      if(dir==TC_DIRECTION_RIGHTTOP)   Tc_Move(hHero_2,GetPointDirection(200,704,&iHero_2));
					   else  if(dir==TC_DIRECTION_RIGHTBOTTOM)   Tc_Move(hHero_2,GetPointDirection(200,0,&iHero_2));
                       else   Tc_Move(hHero_2,GetPointDirection(1748,iGoldBall.pos.y,&iHero_2));
				  }
				  if(iGoldBall.pos.x==1748&&iGoldBall.pos.y>0&&iGoldBall.pos.y<704)
				  {
                       Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
                       if(dir==TC_DIRECTION_RIGHTTOP)   Tc_Move(hHero_2,GetPointDirection(1748,704,&iHero_2));
					   else  if(dir==TC_DIRECTION_RIGHTBOTTOM)   Tc_Move(hHero_2,GetPointDirection(1748,0,&iHero_2));
                       else  Tc_Move(hHero_2,GetPointDirection(200,iGoldBall.pos.y,&iHero_2));
				  }
				  if(iGoldBall.pos.x==200&&iGoldBall.pos.y==0)
				  {
                     if(dir==TC_DIRECTION_LEFT)  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));Tc_Move(hHero_2,GetPointDirection(1748,0,&iHero_2));
                     if(dir==TC_DIRECTION_RIGHTTOP||dir==TC_DIRECTION_LEFTTOP)  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall)); Tc_Move(hHero_2,GetPointDirection(200,704,&iHero_2));
				  }
				  if(iGoldBall.pos.x==200&&iGoldBall.pos.y==704) 
				  {
                      if(dir==TC_DIRECTION_LEFT)  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));Tc_Move(hHero_2,GetPointDirection(1748,704,&iHero_2));
                      if(dir==TC_DIRECTION_RIGHTBOTTOM||dir==TC_DIRECTION_LEFTBOTTOM)  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall)); Tc_Move(hHero_2,GetPointDirection(200,704,&iHero_2));
				  }
				  if(iGoldBall.pos.x==1748&&iGoldBall.pos.y==704)
				  {
                      if(dir==TC_DIRECTION_RIGHT)  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));Tc_Move(hHero_2,GetPointDirection(200,704,&iHero_2));
                      if(dir==TC_DIRECTION_RIGHTBOTTOM||dir==TC_DIRECTION_LEFTBOTTOM)  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall)); Tc_Move(hHero_2,GetPointDirection(200,704,&iHero_2));
				  }
				  if(iGoldBall.pos.x==1748&&iGoldBall.pos.y==0)
				  {
                        if(dir==TC_DIRECTION_RIGHT)  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));Tc_Move(hHero_2,GetPointDirection(200,0,&iHero_2));
                      if(dir==TC_DIRECTION_RIGHTTOP||dir==TC_DIRECTION_TOP)  Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall)); Tc_Move(hHero_2,GetPointDirection(200,0,&iHero_2));
				  }
		if(iHero_1.b_snatch_goldball)
			 {
                 Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
			 }	
		if(iHero_2.b_snatch_goldball)
			 {
              Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
			 }
		 }
		else  
			{
				if(iHero_2.b_snatch_goldball)
			 {
              Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
			 }
			if(iHero_1.b_snatch_goldball)
			 {
                 Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
			 }
				 TC_Direction  dir=GetBallDirection(&iHero_2,&iGoldBall);
				 Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
				  if(iGoldBall.pos.x>200&&iGoldBall.pos.x<1748&&iGoldBall.pos.y==0)
				  {
                     
					  Tc_Move(hHero_1,GetBallDirection(&iHero_2,&iGoldBall));
					  if(dir==TC_DIRECTION_LEFT)   Tc_Move(hHero_1,GetPointDirection(200,0,&iHero_1));
					  else   if(dir==TC_DIRECTION_RIGHT)  Tc_Move(hHero_1,GetPointDirection(1748,0,&iHero_1));
					  else  Tc_Move(hHero_1,GetPointDirection(iGoldBall.pos.x,704,&iHero_1));
				  }
				  if(iGoldBall.pos.x>200&&iGoldBall.pos.x<1748&&iGoldBall.pos.y==704) 
				  {
                     Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
					 if(dir==TC_DIRECTION_LEFT)   Tc_Move(hHero_1,GetPointDirection(200,704,&iHero_1));
					  else   if(dir==TC_DIRECTION_RIGHT)  Tc_Move(hHero_1,GetPointDirection(1748,704,&iHero_1));
					  else   Tc_Move(hHero_1,GetPointDirection(iGoldBall.pos.x,0,&iHero_1));
				  }
				  if(iGoldBall.pos.x==200&&iGoldBall.pos.y>0&&iGoldBall.pos.y<704)
				  {
                      Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
                      if(dir==TC_DIRECTION_RIGHTTOP)   Tc_Move(hHero_1,GetPointDirection(200,704,&iHero_1));
					   else  if(dir==TC_DIRECTION_RIGHTBOTTOM)   Tc_Move(hHero_1,GetPointDirection(200,0,&iHero_1));
                       else   Tc_Move(hHero_1,GetPointDirection(1748,iGoldBall.pos.y,&iHero_1));
				  }
				  if(iGoldBall.pos.x==1748&&iGoldBall.pos.y>0&&iGoldBall.pos.y<704)
				  {
                       Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
                       if(dir==TC_DIRECTION_RIGHTTOP)   Tc_Move(hHero_1,GetPointDirection(1748,704,&iHero_1));
					   else  if(dir==TC_DIRECTION_RIGHTBOTTOM)   Tc_Move(hHero_1,GetPointDirection(1748,0,&iHero_1));
                       else  Tc_Move(hHero_1,GetPointDirection(200,iGoldBall.pos.y,&iHero_1));
				  }
				  if(iGoldBall.pos.x==200&&iGoldBall.pos.y==0)
				  {
                     if(dir==TC_DIRECTION_LEFT)  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));Tc_Move(hHero_1,GetPointDirection(1748,0,&iHero_1));
                     if(dir==TC_DIRECTION_RIGHTTOP||dir==TC_DIRECTION_LEFTTOP)  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall)); Tc_Move(hHero_1,GetPointDirection(200,704,&iHero_1));
				  }
				  if(iGoldBall.pos.x==200&&iGoldBall.pos.y==704) 
				  {
                      if(dir==TC_DIRECTION_LEFT)  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));Tc_Move(hHero_1,GetPointDirection(1748,704,&iHero_1));
                      if(dir==TC_DIRECTION_RIGHTBOTTOM||dir==TC_DIRECTION_LEFTBOTTOM)  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall)); Tc_Move(hHero_1,GetPointDirection(200,704,&iHero_1));
				  }
				  if(iGoldBall.pos.x==1748&&iGoldBall.pos.y==704)
				  {
                      if(dir==TC_DIRECTION_RIGHT)  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));Tc_Move(hHero_1,GetPointDirection(200,704,&iHero_1));
                      if(dir==TC_DIRECTION_RIGHTBOTTOM||dir==TC_DIRECTION_LEFTBOTTOM)  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall)); Tc_Move(hHero_1,GetPointDirection(200,704,&iHero_1));
				  }
				  if(iGoldBall.pos.x==1748&&iGoldBall.pos.y==0)
				  {
                        if(dir==TC_DIRECTION_RIGHT)  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));Tc_Move(hHero_1,GetPointDirection(200,0,&iHero_1));
                      if(dir==TC_DIRECTION_RIGHTTOP||dir==TC_DIRECTION_TOP)  Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall)); Tc_Move(hHero_1,GetPointDirection(200,0,&iHero_1));
				  }
			if(iHero_2.b_snatch_goldball)
			 {
              Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
			 }
			if(iHero_1.b_snatch_goldball)
			 {
                 Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
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
	if(iEnemy_1.type==TC_HERO_GINNY)   flag=1;
	else 
	{
      if(iEnemy_2.type==TC_HERO_GINNY)  flag=2;
	  else  flag=0;
	}
	if(iEnemy_1.type==TC_HERO_RON)   flags=1;
	else 
	{
      if(iEnemy_2.type==TC_HERO_RON)  flags=2;
	  else  flags=0;
	}
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
	iAttackDirection=GetIattack();
	Tc_GetGameInfo(hHero_1,&igame);
    if(iGoldBall.b_visible&&igame.score_enemy>igame.score_self&&igame.score_enemy-igame.score_self>=2)
	{
	  GetGoldBallB();	
	}
	else 
	{  
		  GetGhostBall();
		  Ginny();
		  Ron();  
	}
}