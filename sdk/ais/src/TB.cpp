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
	队名：TB
	队员：丁雨华
	联系：15850574873
*/

CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(_T("TB"));
	
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"SuperMan");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_HERMIONE,L"IronLady");
}

#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
#define LITTLE 2
#define DISTANCE 100
#define BACK 60


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
TC_Position goalRight={2000,376};
TC_Position goalLeft={0,376};
TC_Position middle1,middle2;
TC_Position MLT={550,376};
TC_Position MRT={1400,376};
TC_Position BallPos;
TC_GameInfo  iGameInfo;
TC_Position  Up;
TC_Position  GoldPos;
TC_Position  FollowBall1;
TC_Position  FollowBall2;


int GetState()
{

	if (iHero_1.b_ghostball == false && iHero_2.b_ghostball == false &&
		iEnemy_1.b_ghostball == false && iEnemy_2.b_ghostball == false)
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

void MOVE(TC_Handle handle_self,TC_Hero *pHero, TC_Position &pos_target)
{
	if(pHero->pos.x<pos_target.x&&pHero->pos.y>pos_target.y-3&&pHero->pos.y<pos_target.y+3) {Tc_Move(handle_self,TC_DIRECTION_RIGHT);return;}
	if(pHero->pos.x>pos_target.x&&pHero->pos.y>pos_target.y-3&&pHero->pos.y<pos_target.y+3) {Tc_Move(handle_self,TC_DIRECTION_LEFT);return;}
	if(pHero->pos.x>pos_target.x-3&&pHero->pos.x<pos_target.x+3&&pHero->pos.y>pos_target.y) {Tc_Move(handle_self,TC_DIRECTION_TOP);return;}
	if(pHero->pos.x>pos_target.x-3&&pHero->pos.x<pos_target.x+3&&pHero->pos.y<pos_target.y) {Tc_Move(handle_self,TC_DIRECTION_BOTTOM);return;}
	if(pHero->pos.x<pos_target.x&&pHero->pos.y<pos_target.y) {Tc_Move(handle_self,TC_DIRECTION_RIGHTBOTTOM);return;}
	if(pHero->pos.x>pos_target.x&&pHero->pos.y<pos_target.y) {Tc_Move(handle_self,TC_DIRECTION_LEFTBOTTOM);return;}
	if(pHero->pos.x>pos_target.x&&pHero->pos.y>pos_target.y) {Tc_Move(handle_self,TC_DIRECTION_LEFTTOP);return;}
	if(pHero->pos.x<pos_target.x&&pHero->pos.y>pos_target.y) {Tc_Move(handle_self,TC_DIRECTION_RIGHTTOP);return;}
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
    Tc_GetGameInfo(hHero_1,&iGameInfo);

	middle1.y=middle2.y=408;
	BallPos.x=iGhostBall.pos.x-32;
	BallPos.y=iGhostBall.pos.y-32;
	FollowBall1.y=iGhostBall.pos.y-36;
	FollowBall2.y=iGhostBall.pos.y-28;


	if (iOwnGate.x < iEnemyGate.x)
	{
		iAttackDirection = TC_DIRECTION_RIGHT;
	}
	else
	{
		iAttackDirection = TC_DIRECTION_LEFT;
	}

	int state = GetState();


	if(iAttackDirection == TC_DIRECTION_RIGHT)
			{		
				if(iGhostBall.pos.x>1308)		
				{		
					if(iEnemy_1.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_RON)
					{
						if(iEnemy_1.type==TC_HERO_RON)	
						{
							if(iEnemy_1.pos.x>1308&&iEnemy_1.b_can_spell==true)
							{
						         if(Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
							}
							if(iEnemy_1.b_can_spell==false)
							{
								if(iEnemy_2.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);

							}
							if(iEnemy_1.pos.x<800&&iEnemy_1.b_can_spell==true)
							{
						         if(iEnemy_2.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
							}
						}
						else
						{
							if(iEnemy_2.pos.x>1308&&iEnemy_2.b_can_spell==true)
							{
						         if(Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
							}
							if(iEnemy_2.b_can_spell==false)
							{
								if(iEnemy_1.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);

							}
							if(iEnemy_2.pos.x<800&&iEnemy_1.b_can_spell==true)
							{
						         if(iEnemy_1.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
							}
						}

					}
					else
					{
						if(iEnemy_1.type==TC_HERO_MALFOY||iEnemy_2.type==TC_HERO_MALFOY)
						{
							if(iEnemy_1.type==TC_HERO_MALFOY)	
						    {
								if(iEnemy_1.pos.x>1308&&iEnemy_1.b_can_spell==true)
								{		         
									if(Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
								}
								if(iEnemy_1.b_can_spell==false)
								{
									if(iEnemy_2.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
								}
								if(iEnemy_1.pos.x<800&&iEnemy_1.b_can_spell==true)	
								{
									if(iEnemy_2.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
							
								}
							}		
							else						
							{	
								if(iEnemy_2.pos.x>1308&&iEnemy_2.b_can_spell==true)
								{		         
									if(Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
								}
								if(iEnemy_2.b_can_spell==false)
								{
									if(iEnemy_1.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
								}
								if(iEnemy_2.pos.x<800&&iEnemy_2.b_can_spell==true)	
								{
									if(iEnemy_1.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
							
								}
							}				
						}
						else
						{
							if(iEnemy_1.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_1))Tc_Spell(hHero_2,hEnemy_1);
							if(iEnemy_2.pos.x>1308&&Tc_CanBeSpelled(hHero_2, hEnemy_2))Tc_Spell(hHero_2,hEnemy_2);
						}
					}
				}

				if(iGhostBall.pos.x<600)
				{
					if(iEnemy_1.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hHero_2, hEnemy_1)&&iEnemy_1.b_can_spell==true)
					{
						Tc_Spell(hHero_2,hEnemy_1);
					}
					else
					{
						if(iEnemy_2.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hHero_2, hEnemy_2)&&iEnemy_2.b_can_spell==true)
					    {
					           	Tc_Spell(hHero_2,hEnemy_2);
					    }
						else
						{
							if(Tc_CanBeSpelled(hHero_2, hEnemy_1))Tc_Spell(hHero_2,hEnemy_1);
							if(Tc_CanBeSpelled(hHero_2, hEnemy_2))Tc_Spell(hHero_2,hEnemy_2);
						}
					}
				}
			}	
			else	
			{
				if(iGhostBall.pos.x<612)		
				{		

					if(iEnemy_1.type==TC_HERO_RON||iEnemy_2.type==TC_HERO_RON)
					{
						if(iEnemy_1.type==TC_HERO_RON)	
						{
							if(iEnemy_1.pos.x<612&&iEnemy_1.b_can_spell==true)
							{
						         if(Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
							}
							if(iEnemy_1.b_can_spell==false)
							{
								if(iEnemy_2.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);

							}
							if(iEnemy_1.pos.x>1300&&iEnemy_1.b_can_spell==true)
							{
						         if(iEnemy_2.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
							}
			
						}
						else
						{
							if(iEnemy_2.pos.x<612&&iEnemy_2.b_can_spell==true)
							{
						         if(Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
							}
							if(iEnemy_2.b_can_spell==false)
							{
								if(iEnemy_1.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);

							}
							if(iEnemy_2.pos.x>1300&&iEnemy_2.b_can_spell==true)
							{
						         if(iEnemy_1.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
							}
					

						}
					}
					else
					{
						if(iEnemy_1.type==TC_HERO_MALFOY||iEnemy_2.type==TC_HERO_MALFOY)
						{
							if(iEnemy_1.type==TC_HERO_MALFOY)	
						    {
								if(iEnemy_1.pos.x<612&&iEnemy_1.b_can_spell==true)
								{
									if(Tc_CanBeSpelled(hHero_2, hEnemy_1))
							         	Tc_Spell(hHero_2,hEnemy_1);
								}
								if(iEnemy_1.b_can_spell==false)
								{
									if(iEnemy_2.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
								}
							
								if(iEnemy_1.pos.x>1300&&iEnemy_1.b_can_spell==true)
								{
									if(iEnemy_2.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_2))
							             Tc_Spell(hHero_2,hEnemy_2);
								}						
							}		
							else						
							{	
								if(iEnemy_2.pos.x<612&&iEnemy_2.b_can_spell==true)
								{
									if(Tc_CanBeSpelled(hHero_2, hEnemy_2))
							         	Tc_Spell(hHero_2,hEnemy_2);
								}
								if(iEnemy_2.b_can_spell==false)
								{
									if(iEnemy_1.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
								}
							
								if(iEnemy_2.pos.x>1300&&iEnemy_2.b_can_spell==true)
								{
									if(iEnemy_1.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_1))
							             Tc_Spell(hHero_2,hEnemy_1);
								}						
								
							}				
						}
						else
						{
							if(iEnemy_1.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_1))Tc_Spell(hHero_2,hEnemy_1);
							if(iEnemy_2.pos.x<612&&Tc_CanBeSpelled(hHero_2, hEnemy_2))Tc_Spell(hHero_2,hEnemy_2);
						}
					}
				}

				if(iGhostBall.pos.x>1428)
				{
					if(iEnemy_1.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hHero_2, hEnemy_1)&&iEnemy_1.b_can_spell==true)
					{
						Tc_Spell(hHero_2,hEnemy_1);
					}
					else
					{
						if(iEnemy_2.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hHero_2, hEnemy_2)&&iEnemy_2.b_can_spell==true)
					    {
					           	Tc_Spell(hHero_2,hEnemy_2);
					    }
						else
						{
							if(Tc_CanBeSpelled(hHero_2, hEnemy_1))Tc_Spell(hHero_2,hEnemy_1);
							if(Tc_CanBeSpelled(hHero_2, hEnemy_2))Tc_Spell(hHero_2,hEnemy_2);
						}
					}

				}	

			}



if(iGoldBall.b_visible==true&&iGameInfo.score_self+1<iGameInfo.score_enemy)
{
	
     MOVE(hHero_1,&iHero_1,iGoldBall.pos);
		
	 if(Tc_CanBeSpelled(hHero_1, hEnemy_1))Tc_Spell(hHero_1,hEnemy_1);       
	 if(Tc_CanBeSpelled(hHero_1, hEnemy_2))Tc_Spell(hHero_1,hEnemy_2);

	 if(Tc_CanBeSpelled(hHero_2, hEnemy_1)&&hEnemy_1==hEnemyWithBall)Tc_Spell(hHero_2,hEnemy_1);	
	 if(Tc_CanBeSpelled(hHero_2, hEnemy_2)&&hEnemy_2==hEnemyWithBall)Tc_Spell(hHero_2,hEnemy_2);	

	if(iHero_1.pos.x>iGoldBall.pos.x-20&&iHero_1.pos.x<iGoldBall.pos.x+20)
	{
		GoldPos.x=iGoldBall.pos.x;
		if(iHero_2.pos.y<384)       
			GoldPos.y=0;
		else
			GoldPos.y=700;

	    MOVE(hHero_2,&iHero_2,GoldPos);
	   
	            if (iHero_1.b_snatch_goldball== true)
			    {
				    if (Tc_SnatchBall(hHero_1,TC_GOLD_BALL) == true)
				    {
						return;
				    }
			    }
			    if (iHero_2.b_snatch_goldball == true)
			    {
				    if (Tc_SnatchBall(hHero_2,TC_GOLD_BALL) == true)
				    {
						return;
				    }
			    }

	   return;
	}

	if(iHero_1.pos.y>iGoldBall.pos.y-20&&iHero_1.pos.y<iGoldBall.pos.y+20)
	{
		if(iHero_1.pos.x<iHero_2.pos.x&&(iGoldBall.pos.x<300||iGoldBall.pos.x>1600))
		{
			GoldPos.x=1715;
            GoldPos.y=iGoldBall.pos.y;
	        MOVE(hHero_2,&iHero_2,GoldPos);
			
	            if (iHero_1.b_snatch_goldball== true)
			    {
				    if (Tc_SnatchBall(hHero_1,TC_GOLD_BALL) == true)
				    {
						return;
				    }
			    }
			    if (iHero_2.b_snatch_goldball == true)
			    {
				    if (Tc_SnatchBall(hHero_2,TC_GOLD_BALL) == true)
				    {
						return;
				    }
			    }

	        return;
		}
		if(iHero_1.pos.x>iHero_2.pos.x&&(iGoldBall.pos.x<300||iGoldBall.pos.x>1600))
		{
			GoldPos.x=205;
            GoldPos.y=iGoldBall.pos.y;
	        MOVE(hHero_2,&iHero_2,GoldPos);
			
	            if (iHero_1.b_snatch_goldball== true)
			    {
				    if (Tc_SnatchBall(hHero_1,TC_GOLD_BALL) == true)
				    {
						return;
				    }
			    }
			    if (iHero_2.b_snatch_goldball == true)
			    {
				    if (Tc_SnatchBall(hHero_2,TC_GOLD_BALL) == true)
				    {
						return;
				    }
			    }
	        return;
		}

	}


	if(iGoldBall.pos.x>iHero_2.pos.x-700&&iGoldBall.pos.x<iHero_2.pos.x+700&&
		iGoldBall.pos.y>iHero_2.pos.y-700&&iGoldBall.pos.y<iHero_2.pos.y+700)
		MOVE(hHero_2,&iHero_2,iGoldBall.pos);
	else
	{

		if(iHero_2.pos.x<1848&&iHero_2.pos.x>200)
		{
			if(iHero_1.pos.x<iGoldBall.pos.x&&iHero_1.pos.y>iGoldBall.pos.y-5&&iHero_1.pos.y<iGoldBall.pos.y+5) Tc_Move(hHero_2,TC_DIRECTION_LEFT);
			if(iHero_1.pos.x>iGoldBall.pos.x&&iHero_1.pos.y>iGoldBall.pos.y-5&&iHero_1.pos.y<iGoldBall.pos.y+5) Tc_Move(hHero_2,TC_DIRECTION_RIGHT);
			if(iHero_1.pos.x<iGoldBall.pos.x&&iHero_1.pos.y<iGoldBall.pos.y) Tc_Move(hHero_2,TC_DIRECTION_LEFTTOP);
			if(iHero_1.pos.x>iGoldBall.pos.x&&iHero_1.pos.y<iGoldBall.pos.y) Tc_Move(hHero_2,TC_DIRECTION_RIGHTTOP);
			if(iHero_1.pos.x>iGoldBall.pos.x&&iHero_1.pos.y>iGoldBall.pos.y) Tc_Move(hHero_2,TC_DIRECTION_RIGHTBOTTOM);
			if(iHero_1.pos.x<iGoldBall.pos.x&&iHero_1.pos.y>iGoldBall.pos.y) Tc_Move(hHero_2,TC_DIRECTION_LEFTBOTTOM);
		}
		else
		{
			MOVE(hHero_2,&iHero_2,iGoldBall.pos);
		}

	}

	            if (iHero_1.b_snatch_goldball== true)
			    {
				    if (Tc_SnatchBall(hHero_1,TC_GOLD_BALL) == true)
				    {
						return;
				    }
			    }
			    if (iHero_2.b_snatch_goldball == true)
			    {
				    if (Tc_SnatchBall(hHero_2,TC_GOLD_BALL) == true)
				    {
						return;
				    }
			    }

    if(state==GHOSTBALL_IN_SELF_TEAM)
	{	
		Up.x=iGhostBall.pos.x;
		Up.y=iGhostBall.pos.y-500;
		if(Up.y<0)Up.y=0;
        Tc_PassBall(hHeroWithBall, Up);

	}


}
else
{

	if(iGoldBall.b_visible==true)
	{
		if(iGameInfo.score_self>iGameInfo.score_enemy+1)
		{
				if(Tc_CanBeSpelled(hHero_1, hEnemy_1))
					Tc_Spell(hHero_1,hEnemy_1);

				if(Tc_CanBeSpelled(hHero_1, hEnemy_2))
					Tc_Spell(hHero_1,hEnemy_2);	
		}
		else
		{
			if(iGoldBall.pos.x-300<iEnemy_1.pos.x&&iEnemy_1.pos.x<iGoldBall.pos.x+300&&
		        iGoldBall.pos.y-300<iEnemy_1.pos.y&&iEnemy_1.pos.y<iGoldBall.pos.y+300)
				if(Tc_CanBeSpelled(hHero_1, hEnemy_1))
					Tc_Spell(hHero_1,hEnemy_1);

	    
			if(iGoldBall.pos.x-300<iEnemy_2.pos.x&&iEnemy_2.pos.x<iGoldBall.pos.x+300&&
	            iGoldBall.pos.y-300<iEnemy_2.pos.y&&iEnemy_2.pos.y<iGoldBall.pos.y+300)
				if(Tc_CanBeSpelled(hHero_1, hEnemy_2))
					Tc_Spell(hHero_1,hEnemy_2);	
		}

		if (iHero_1.b_snatch_goldball == true) 
	    {
		    Tc_SnatchBall(hHero_1,TC_GOLD_BALL);	
	    }
	
	    if (iHero_2.b_snatch_goldball == true) 
	    {
		    Tc_SnatchBall(hHero_2,TC_GOLD_BALL);	
	    }

	}


	switch (state)
	{
	case GHOSTBALL_IN_SELF_TEAM:
		{
			if(iAttackDirection ==TC_DIRECTION_RIGHT)MOVE(hHeroWithBall,&iHeroWithBall,goalRight);
			else MOVE(hHeroWithBall,&iHeroWithBall,goalLeft);
			MOVE(hHeroWithoutBall,&iHeroWithoutBall,BallPos);


			if(iHeroWithBall.abnormal_type==TC_SPELLED_BY_MALFOY||iHeroWithBall.abnormal_type==TC_SPELLED_BY_FREEBALL)
			{
				if(iHeroWithoutBall.b_snatch_ghostball==true)
					if(Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL)==true)
						break;
			}

			if(iEnemy_1.b_snatch_ghostball==false&&iEnemy_2.b_snatch_ghostball==false)
			{
				if (iAttackDirection == TC_DIRECTION_RIGHT)
				{
					if(iHeroWithBall.pos.x+LITTLE>iEnemy_1.pos.x&&iHeroWithBall.pos.x+LITTLE>iEnemy_2.pos.x&&iHeroWithBall.pos.x<1292)
					{
						middle1.x=sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+iGhostBall.pos.x;
						if(middle1.x>1525)middle1.x=1525;
						if(Tc_PassBall(hHeroWithBall, middle1)==true)
					             break;
					}
				}
				else
				{
					if(iHeroWithBall.pos.x-LITTLE<iEnemy_1.pos.x&&iHeroWithBall.pos.x-LITTLE<iEnemy_2.pos.x&&iHeroWithBall.pos.x>628)
					{
						middle2.x=iGhostBall.pos.x-sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+2;
						if(middle2.x<460)middle2.x=460;
						if(Tc_PassBall(hHeroWithBall, middle2)==true)
					             break;
					}
				}
			}

			if(iEnemy_1.b_snatch_ghostball==false&&(iEnemy_2.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_2.abnormal_type==TC_SPELLED_BY_FREEBALL))
			{
				if (iAttackDirection == TC_DIRECTION_RIGHT)
				{
					if(iHeroWithBall.pos.x+LITTLE>iEnemy_1.pos.x&&iHeroWithBall.pos.x<1292)
					{
						middle1.x=sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+iGhostBall.pos.x;
						if(middle1.x>1525)middle1.x=1525;
						if(Tc_PassBall(hHeroWithBall, middle1)==true)
					             break;
					}
				}
				else
				{
					if(iHeroWithBall.pos.x-LITTLE<iEnemy_1.pos.x&&iHeroWithBall.pos.x>628)
					{
                        middle2.x=iGhostBall.pos.x-sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+2;
						if(middle2.x<460)middle2.x=460;
						if(Tc_PassBall(hHeroWithBall, middle2)==true)
					             break;
					}
				}
			}

			if(iEnemy_2.b_snatch_ghostball==false&&(iEnemy_1.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_1.abnormal_type==TC_SPELLED_BY_FREEBALL))
			{
				if (iAttackDirection == TC_DIRECTION_RIGHT)
				{
					if(iHeroWithBall.pos.x+LITTLE>iEnemy_2.pos.x&&iHeroWithBall.pos.x<1292)
					{
						middle1.x=sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+iGhostBall.pos.x;
						if(middle1.x>1525)middle1.x=1525;
						if(Tc_PassBall(hHeroWithBall, middle1)==true)
					             break;
					}
				}
				else
				{
					if(iHeroWithBall.pos.x-LITTLE<iEnemy_2.pos.x&&iHeroWithBall.pos.x>628)
					{
						 middle2.x=iGhostBall.pos.x-sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+2;
						if(middle2.x<460)middle2.x=460;
						if(Tc_PassBall(hHeroWithBall, middle2)==true)
					             break;
					}
				}
			}

            if(iAttackDirection == TC_DIRECTION_RIGHT)
			{	
				if(iEnemy_1.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_1.abnormal_type==TC_SPELLED_BY_FREEBALL)
				     if(iEnemy_2.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_2.abnormal_type==TC_SPELLED_BY_FREEBALL)
                          if(iHeroWithBall.pos.x<1292)
						  {
							  middle1.x=sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+iGhostBall.pos.x;
						      if(middle1.x>1525)middle1.x=1525;
                              if(Tc_PassBall(hHeroWithBall, middle1)==true)
					             break;
						  }
				if(iEnemy_1.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_1.abnormal_type==TC_SPELLED_BY_FREEBALL)
					if(iHeroWithBall.pos.x>iEnemy_2.pos.x+BACK&&iHeroWithBall.pos.x<1292)
					{
						middle1.x=sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+iGhostBall.pos.x;
						if(middle1.x>1525)middle1.x=1525;
						if(Tc_PassBall(hHeroWithBall, middle1)==true)
					            break;
					}
				if(iEnemy_2.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_2.abnormal_type==TC_SPELLED_BY_FREEBALL)
					if(iHeroWithBall.pos.x>iEnemy_1.pos.x+BACK&&iHeroWithBall.pos.x<1292)
					{
						middle1.x=sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+iGhostBall.pos.x;
						if(middle1.x>1525)middle1.x=1525;
						if(Tc_PassBall(hHeroWithBall, middle1)==true)
					            break;
					}
		         if(iHeroWithBall.pos.x>iEnemy_1.pos.x+BACK&&iHeroWithBall.pos.x>iEnemy_2.pos.x+BACK&&iHeroWithBall.pos.x<1292)
				 {
					 middle1.x=sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+iGhostBall.pos.x;
					 if(middle1.x>1525)middle1.x=1525;
					 if(Tc_PassBall(hHeroWithBall, middle1)==true)
					             break;
				 }	

			}

			if (iAttackDirection == TC_DIRECTION_LEFT)
			{
				if(iEnemy_1.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_1.abnormal_type==TC_SPELLED_BY_FREEBALL)
				     if(iEnemy_2.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_2.abnormal_type==TC_SPELLED_BY_FREEBALL)
						  if(iHeroWithBall.pos.x>628)
						  {
							   middle2.x=iGhostBall.pos.x-sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+2;
						       if(middle2.x<460)middle2.x=460;
                               if(Tc_PassBall(hHeroWithBall, middle2)==true)
					             break;
						  }
                      
				if(iEnemy_1.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_1.abnormal_type==TC_SPELLED_BY_FREEBALL)
					if(iHeroWithBall.pos.x+BACK<iEnemy_2.pos.x&&iHeroWithBall.pos.x>628)
					{
						middle2.x=iGhostBall.pos.x-sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+2;
						if(middle2.x<460)middle2.x=460;
						if(Tc_PassBall(hHeroWithBall, middle2)==true)
					            break;
					}
				if(iEnemy_2.abnormal_type==TC_SPELLED_BY_MALFOY||iEnemy_2.abnormal_type==TC_SPELLED_BY_FREEBALL)
					if(iHeroWithBall.pos.x+BACK<iEnemy_1.pos.x&&iHeroWithBall.pos.x>628)
					{
						middle2.x=iGhostBall.pos.x-sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+2;
						if(middle2.x<460)middle2.x=460;
                        if(Tc_PassBall(hHeroWithBall, middle2)==true)
					            break;
					}

		         if(iHeroWithBall.pos.x+BACK<iEnemy_1.pos.x&&iHeroWithBall.pos.x+BACK<iEnemy_2.pos.x&&iHeroWithBall.pos.x>628)
				 {
					 middle2.x=iGhostBall.pos.x-sqrt(float(512*512-(iGhostBall.pos.y-408)*(iGhostBall.pos.y-408)))+2;
					 if(middle2.x<460)middle2.x=460;
			         if(Tc_PassBall(hHeroWithBall, middle2)==true)
					 break;
				 }
			
			}

			if(iEnemy_1.type==TC_HERO_GINNY&&(iEnemy_1.abnormal_type==TC_SPELLING||iEnemy_1.b_is_spelling==true))
			{
				if(iAttackDirection == TC_DIRECTION_RIGHT)
				{
					if(iGhostBall.pos.x<=1592)
					{
						Up.x=1525;
						Up.y=iGhostBall.pos.y;
					}
					else
					{
						if(iGhostBall.pos.x+64<1848)
						{
							Up.x=iGhostBall.pos.x;
						    Up.y=643;
						}
						else
						{
							Up.x=1525;
						    Up.y=iGhostBall.pos.y;
						}

					}
				}
				else
				{
					if(iGhostBall.pos.x>=392)
					{
						Up.x=460;
						Up.y=iGhostBall.pos.y;
					}
					else
					{
						if(iGhostBall.pos.x>200)
						{
							Up.x=iGhostBall.pos.x;
						    Up.y=643;
						}
						else
						{
							Up.x=460;
						    Up.y=iGhostBall.pos.y;
						}

					}
				}
   
				Tc_PassBall(hHeroWithBall, Up);

			}
			if(iEnemy_2.type==TC_HERO_GINNY&&(iEnemy_2.abnormal_type==TC_SPELLING||iEnemy_2.b_is_spelling==true))
			{
				if(iAttackDirection == TC_DIRECTION_RIGHT)
				{
					if(iGhostBall.pos.x<=1592)
					{
						Up.x=1525;
						Up.y=iGhostBall.pos.y;
					}
					else
					{
						if(iGhostBall.pos.x+64<1848)
						{
							Up.x=iGhostBall.pos.x;
						    Up.y=643;
						}
						else
						{
							Up.x=1525;
						    Up.y=iGhostBall.pos.y;
						}

					}
				}
				else
				{
					if(iGhostBall.pos.x>=392)
					{
						Up.x=460;
						Up.y=iGhostBall.pos.y;
					}
					else
					{
						if(iGhostBall.pos.x>200)
						{
							Up.x=iGhostBall.pos.x;
						    Up.y=643;
						}
						else
						{
							Up.x=460;
						    Up.y=iGhostBall.pos.y;
						}

					}
				}
				
				Tc_PassBall(hHeroWithBall, Up);
			}

			if(iEnemy_2.type==TC_HERO_RON&&(iEnemy_2.abnormal_type==TC_SPELLING||iEnemy_2.b_is_spelling==true))
			{
				if(iAttackDirection == TC_DIRECTION_RIGHT)
				{
					if(iGhostBall.pos.x<=1592)
					{
						Up.x=1525;
						Up.y=iGhostBall.pos.y;
					}
					else
					{
						if(iGhostBall.pos.x+64<1848)
						{
							Up.x=iGhostBall.pos.x;
						    Up.y=643;
						}
						else
						{
							Up.x=1525;
						    Up.y=iGhostBall.pos.y;
						}

					}
				}
				else
				{
					if(iGhostBall.pos.x>=392)
					{
						Up.x=460;
						Up.y=iGhostBall.pos.y;
					}
					else
					{
						if(iGhostBall.pos.x>200)
						{
							Up.x=iGhostBall.pos.x;
						    Up.y=643;
						}
						else
						{
							Up.x=460;
						    Up.y=iGhostBall.pos.y;
						}

					}

				}
					Tc_PassBall(hHeroWithBall, Up);
			}

			if(iEnemy_1.type==TC_HERO_RON&&(iEnemy_1.abnormal_type==TC_SPELLING||iEnemy_1.b_is_spelling==true))
			{
				if(iAttackDirection == TC_DIRECTION_RIGHT)
				{
					if(iGhostBall.pos.x<=1592)
					{
						Up.x=1525;
						Up.y=iGhostBall.pos.y;
					}
					else
					{
						if(iGhostBall.pos.x+64<1848)
						{
							Up.x=iGhostBall.pos.x;
						    Up.y=643;
						}
						else
						{
							Up.x=1525;
						    Up.y=iGhostBall.pos.y;
						}

					}
				}
				else
				{
					if(iGhostBall.pos.x>=392)
					{
						Up.x=460;
						Up.y=iGhostBall.pos.y;
					}
					else
					{
						if(iGhostBall.pos.x>200)
						{
							Up.x=iGhostBall.pos.x;
						    Up.y=643;
						}
						else
						{
							Up.x=460;
						    Up.y=iGhostBall.pos.y;
						}

					}

				}
					Tc_PassBall(hHeroWithBall, Up);
			}

			break;
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
	{
		if(iAttackDirection == TC_DIRECTION_RIGHT)
		{
			FollowBall1.x=iGhostBall.pos.x-64;
			FollowBall2.x=iGhostBall.pos.x-64;
		}
		else
		{
			FollowBall1.x=iGhostBall.pos.x;
			FollowBall2.x=iGhostBall.pos.x;
		}

		 MOVE(hHero_2,&iHero_2,FollowBall2);
         MOVE(hHero_1,&iHero_1,FollowBall1);
	
		if(iAttackDirection == TC_DIRECTION_RIGHT)
		{	  
			if(iEnemyWithBall.pos.x>iForbiddenArea.right.left-128&&iEnemyWithoutBall.pos.x<1000)	  
				if(Tc_CanBeSpelled(hHero_1, hEnemyWithBall))
						Tc_Spell(hHero_1,hEnemyWithBall);
	  
			if(iGhostBall.pos.x>iForbiddenArea.right.left||iGhostBall.pos.x<800)			  
				if(Tc_CanBeSpelled(hHero_1, hEnemyWithBall))
						Tc_Spell(hHero_1,hEnemyWithBall);
		 }
		else
		{			
			if(iEnemyWithBall.pos.x<iForbiddenArea.left.right+128&&iEnemyWithoutBall.pos.x>1000)					
				if(Tc_CanBeSpelled(hHero_1, hEnemyWithBall))
						 Tc_Spell(hHero_1,hEnemyWithBall);		
				
			if(iGhostBall.pos.x<iForbiddenArea.left.right||iGhostBall.pos.x>1200)					
				if(Tc_CanBeSpelled(hHero_1, hEnemyWithBall))
						Tc_Spell(hHero_1,hEnemyWithBall);
		}
			

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
	}
	case GHOSTBALL_IN_NO_TEAM:
	{

		if (iHero_1.b_snatch_ghostball == false) 		   
		{		 
			MOVE(hHero_1,&iHero_1,BallPos);		  
		}	   
		else			   
		{			
			if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))break;			  
		}
          
		if (iHero_1.b_ghostball==false&&iHero_2.b_snatch_ghostball == false)	
		{		
			MOVE(hHero_2,&iHero_2,BallPos);		
		}	
		else 		
		{	
			if(iHero_1.b_ghostball==false)	
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))break;	
			MOVE(hHero_2,&iHero_2,BallPos);			
		}

	break;
	}
	default:
		{
			break;
		}
	}

}

}