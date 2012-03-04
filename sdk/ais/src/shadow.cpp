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
	Tc_SetTeamName(_T("shadow"));
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"820");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_GINNY,L"357");
}

#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
#define GOLDBALL_REACHABLE 3
#define PASSBALLDISTANCE 150
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
void Attack()
{
        if(iAttackDirection==TC_DIRECTION_LEFT)
  	    {	  
			if(iHero_1.b_can_spell==true)//可能考虑罗恩
			{
				if(iHero_1.b_ghostball==true&&iHero_1.pos.x<iForbiddenArea.left.right+PASSBALLDISTANCE||(iHero_2.b_ghostball==true&&(iHero_2.pos.x<iForbiddenArea.left.right+PASSBALLDISTANCE)))
				{
					if(iEnemy_1.type==TC_HERO_GINNY&&Tc_CanBeSpelled(hHero_1,hEnemy_1))
					{
                     Tc_Spell(hHero_1,hEnemy_1);
					}
					else if(iEnemy_2.type==TC_HERO_GINNY&&Tc_CanBeSpelled(hHero_1,hEnemy_2))
					{
                     Tc_Spell(hHero_1,hEnemy_2);
					}
					else 
					{
						if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
                       Tc_Spell(hHero_1,hEnemy_1);
						if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
                      Tc_Spell(hHero_1,hEnemy_2);
					}
		    	}
			}
			
		 if(iEnemy_1.b_ghostball==true)
		 {
			  if(iEnemy_1.pos.x>iForbiddenArea.right.left+PASSBALLDISTANCE)
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
		 if(iEnemy_2.b_ghostball==true)
		 {
			  if(iEnemy_2.pos.x>iForbiddenArea.right.left+PASSBALLDISTANCE)
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
	    else
	    {	
			if(iHero_1.b_can_spell==true)
			{
				if((iHero_1.b_ghostball==true&&(iHero_1.pos.x>iForbiddenArea.right.left-PASSBALLDISTANCE))||(iHero_2.b_ghostball==true&&(iHero_2.pos.x>iForbiddenArea.right.left-PASSBALLDISTANCE)))
				{
					if(iEnemy_1.type==TC_HERO_GINNY&&Tc_CanBeSpelled(hHero_1,hEnemy_1))
					{
                     Tc_Spell(hHero_1,hEnemy_1);
					}
					else if(iEnemy_2.type==TC_HERO_GINNY&&Tc_CanBeSpelled(hHero_1,hEnemy_2))
					{
                     Tc_Spell(hHero_1,hEnemy_2);
					}
					else 
					{
						if(Tc_CanBeSpelled(hHero_1,hEnemy_1))
                       Tc_Spell(hHero_1,hEnemy_1);
						if(Tc_CanBeSpelled(hHero_1,hEnemy_2))
                      Tc_Spell(hHero_1,hEnemy_2);
					}
				}
			}
		  if(iEnemy_1.b_ghostball==true)
		  {
			 if(iEnemy_1.pos.x<iForbiddenArea.left.right-PASSBALLDISTANCE)
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
		  if(iEnemy_2.b_ghostball==true)
		  {
            if(iEnemy_2.pos.x<iForbiddenArea.left.right-PASSBALLDISTANCE)
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
void PassBall()
{
	if(iHero_1.b_ghostball==true)
	{
		if(iAttackDirection==TC_DIRECTION_RIGHT)
		{
			if((iEnemy_1.pos.x<iHero_1.pos.x+20||iEnemy_1.steps_before_next_snatch>12)
				&&(iEnemy_2.pos.x<iHero_1.pos.x+20||iEnemy_2.steps_before_next_snatch>12)
				&&iHero_1.pos.x+128<iForbiddenArea.right.left)
			{
				TC_Position pos;
				pos.x=iGhostBall.pos.x+(iForbiddenArea.right.left-iGhostBall.pos.x-TC_BALL_WIDTH<512?iForbiddenArea.right.left-iGhostBall.pos.x-TC_BALL_WIDTH:512);
				pos.y=iGhostBall.pos.y;
				Tc_PassBall(hHero_1,pos);
			}
		}
		else
		{
			if((iEnemy_1.pos.x<iHero_1.pos.x+20||iEnemy_1.steps_before_next_snatch>12)
				&&(iEnemy_2.pos.x<iHero_1.pos.x+20||iEnemy_2.steps_before_next_snatch>12)
				&&iHero_1.pos.x>iForbiddenArea.left.right)
			{
				TC_Position pos;
				pos.x=iGhostBall.pos.x-(iGhostBall.pos.x-iForbiddenArea.left.right<512?iGhostBall.pos.x-iForbiddenArea.left.right:512);
				pos.y=iGhostBall.pos.y;
				Tc_PassBall(hHero_1,pos);
			}
		}
	}
	if(iHero_2.b_ghostball==true)
	{
       if(iAttackDirection==TC_DIRECTION_RIGHT)
		{
			if((iEnemy_1.pos.x<iHero_2.pos.x+20||iEnemy_1.steps_before_next_snatch>12)
				&&(iEnemy_2.pos.x<iHero_2.pos.x+20||iEnemy_2.steps_before_next_snatch>12)
				&&iHero_2.pos.x+128<iForbiddenArea.right.left)
			{
				TC_Position pos;
				pos.x=iGhostBall.pos.x+(iForbiddenArea.right.left-iGhostBall.pos.x-TC_BALL_WIDTH<512?iForbiddenArea.right.left-iGhostBall.pos.x-TC_BALL_WIDTH:512);
				pos.y=iGhostBall.pos.y;
				Tc_PassBall(hHero_2,pos);
			}
		}
		else
		{
			if((iEnemy_1.pos.x<iHero_2.pos.x+20||iEnemy_1.steps_before_next_snatch>12)
				&&(iEnemy_2.pos.x<iHero_2.pos.x+20||iEnemy_2.steps_before_next_snatch>12)
				&&iHero_1.pos.x>iForbiddenArea.left.right)
			{
				TC_Position pos;
				pos.x=iGhostBall.pos.x-(iGhostBall.pos.x-iForbiddenArea.left.right<512?iGhostBall.pos.x-iForbiddenArea.left.right:512);
				pos.y=iGhostBall.pos.y;
				Tc_PassBall(hHero_2,pos);
			}
		}

	}
}
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

TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pEnemy)
{
	if (pHero->pos.x > pEnemy->pos.x+5 && pHero->pos.y > pEnemy->pos.y+5)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x < pEnemy->pos.x+5 && pHero->pos.x >pEnemy->pos.x-5 && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pEnemy->pos.x-5 && pHero->pos.y > pEnemy->pos.y+5)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y<pEnemy->pos.y+5&& pHero->pos.y>pEnemy->pos.y-5)return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pEnemy->pos.x-5 && pHero->pos.y < pEnemy->pos.y-5)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x < pEnemy->pos.x +5&& pHero->pos.x > pEnemy->pos.x -5&&pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pEnemy->pos.x +5&& pHero->pos.y < pEnemy->pos.y-5)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y+5&&pHero->pos.y >pEnemy->pos.y-5)return TC_DIRECTION_LEFT;
}
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)
{
	int length=35;
	if(pHero->pos.x+TC_HERO_WIDTH/2>pBall->pos.x+TC_BALL_WIDTH/2+length)
		if(pHero->pos.y+TC_HERO_HEIGHT/2>pBall->pos.y+TC_BALL_HEIGHT/2+length)
			return TC_DIRECTION_LEFTTOP;
		else
			if(pHero->pos.y+TC_HERO_HEIGHT/2<pBall->pos.y+TC_BALL_HEIGHT/2-length)
				return TC_DIRECTION_LEFTBOTTOM;
			else
				return TC_DIRECTION_LEFT;
	else
		if(pHero->pos.x+TC_HERO_WIDTH/2<pBall->pos.x+TC_BALL_WIDTH/2-length)
			if(pHero->pos.y+TC_HERO_HEIGHT/2>pBall->pos.y+TC_BALL_HEIGHT/2+length)
		    	return TC_DIRECTION_RIGHTTOP;
	       	else
	    		 if(pHero->pos.y+TC_HERO_HEIGHT/2<pBall->pos.y+TC_BALL_HEIGHT/2-length)
		     		return TC_DIRECTION_RIGHTBOTTOM;
		    	else
		     		return TC_DIRECTION_RIGHT;
		else
			if(pHero->pos.y+TC_HERO_HEIGHT/2>pBall->pos.y+TC_BALL_HEIGHT/2+length)
				return TC_DIRECTION_TOP;
			else
				return TC_DIRECTION_BOTTOM;
}
TC_Direction GetAttackDirection(TC_Hero *pHero)
{
    if(iOwnGate.x<iEnemyGate.x)
	{
		if(iHeroWithBall.pos.y<iEnemyGate.y_upper+7)
			return TC_DIRECTION_RIGHTBOTTOM;
		else
			if(iHeroWithBall.pos.y+128>iEnemyGate.y_lower-7)
				return TC_DIRECTION_RIGHTTOP;
			else
				return TC_DIRECTION_RIGHT;

	}
	else
	{
		if(iHeroWithBall.pos.y<iEnemyGate.y_upper+7)
			return TC_DIRECTION_LEFTBOTTOM;
		else
			if(iHeroWithBall.pos.y+128>iEnemyGate.y_lower-7)
				return TC_DIRECTION_LEFTTOP;
			else
				return TC_DIRECTION_LEFT;
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
	switch (state)
	{
	case GHOSTBALL_IN_SELF_TEAM:
		
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetEnemyDirection(&iHeroWithoutBall, &iHeroWithBall));
			if(iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL||iHeroWithBall.abnormal_type==TC_SPELLED_BY_HERMIONE)
			{
				if (iHeroWithoutBall.b_snatch_ghostball==true)
               Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL);
			}
			break;
	
	case GHOSTBALL_IN_ENEMY_TEAM:
		
			TC_Handle hHeroToBall, hHeroNotToBall;
			TC_Hero iHeroToBall, iHeroNotToBall;
			Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
			Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
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
			break;
		
	case GHOSTBALL_IN_NO_TEAM:
		
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
		
	case GOLDBALL_REACHABLE:
		
			break;
		
	default:
		
			break;
		
	}
	Attack();
	PassBall();

	if (iHero_1.b_goldball==true)
		Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
	if (iHero_2.b_goldball==true)
		Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
	
	if(iEnemy_2.type==TC_HERO_GINNY)//躲jinny
	{
	  if(iEnemy_2.b_is_spelling==true&&iHero_1.b_ghostball==true)
	  {
          TC_Position target;
		   if(iAttackDirection==TC_DIRECTION_RIGHT)
			   target.x=iForbiddenArea.right.left-TC_BALL_WIDTH;
		   else
			   target.x=iForbiddenArea.left.right+TC_BALL_WIDTH;
			target.y=iGhostBall.pos.y;
			Tc_PassBall(hHero_1,target);
		}
		if(iEnemy_2.b_is_spelling==true&&iHero_2.b_ghostball==true)
		{
             TC_Position target;
			  if(iAttackDirection==TC_DIRECTION_RIGHT)
				 target.x=iForbiddenArea.right.left-TC_BALL_WIDTH;
			  else
				 target.x=iForbiddenArea.left.right+TC_BALL_WIDTH;
			  target.y=iGhostBall.pos.y;
			  Tc_PassBall(hHero_2,target);
		}
	}
	if(iEnemy_1.type==TC_HERO_GINNY)
	{
      if(iEnemy_1.b_is_spelling==true&&iHero_1.b_ghostball==true)
	  {
          TC_Position target;
		   if(iAttackDirection==TC_DIRECTION_RIGHT)
			target.x=iForbiddenArea.right.left-TC_BALL_WIDTH;
		   else
			target.x=iForbiddenArea.left.right+TC_BALL_WIDTH;
			target.y=iGhostBall.pos.y;
			 Tc_PassBall(hHero_1,target);
		}
		if(iEnemy_1.b_is_spelling==true&&iHero_2.b_ghostball==true)
		{
          TC_Position target;
			  if(iAttackDirection==TC_DIRECTION_RIGHT)
			   target.x=iForbiddenArea.right.left-TC_BALL_WIDTH;
			  else
				target.x=iForbiddenArea.left.right+TC_BALL_WIDTH;
			  target.y=iGhostBall.pos.y;
			  Tc_PassBall(hHero_2,target);
		}
	}
}