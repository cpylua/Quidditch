/*#include <Windows.h>
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


//How to debug?

//Goto
//Project -. Properties --> Configuration Properties --> Debugging

//Copy "$(ProjectDir)bin\TuringCup9.exe" 
//to the Command field.(Without quotation marks)

CLIENT_EXPORT_API void __stdcall Init()
{
Tc_SetTeamName(_T("Zombie_Three"));

Tc_ChooseHero(TC_MEMBER1,TC_HERO_HARRY,L"h1");
Tc_ChooseHero(TC_MEMBER2,TC_HERO_GINNY,L"h2");
}

TC_Handle h1, h2;

CLIENT_EXPORT_API void __stdcall AI()
{
//
// TODO: add your AI here
//
h1 = Tc_GetHeroHandle(TC_MEMBER1);
h2 = Tc_GetHeroHandle(TC_MEMBER2);

if( Tc_CanBeSpelled(h1, h2) )
{
Tc_Stop(h1);
Tc_Spell(h1, h2);
}
else
Tc_Move(h1, TC_DIRECTION_BOTTOM);
}*/



/*
Team:Zombie_Three
Member:苏英  李斐斐  朱雪娇
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



#define GHOSTBALL_IN_NO_TEAM 0
#define GHOSTBALL_IN_SELF_TEAM 1
#define GHOSTBALL_IN_ENEMY_TEAM 2
#define GOLDBALL_REACHABLE 3
TC_Handle hHero_1, hHero_2;
TC_Handle hEnemy_1, hEnemy_2;
TC_Handle hHeroWithBall, hHeroWithoutBall,exEnemyWithBall;
TC_Handle hEnemyWithBall, hEnemyWithoutBall,hEnemynear,hEnemyfar;
TC_Handle hFreeBall, hGhostBall, hGoldBall;
TC_Ball iFreeBall, iGhostBall, iGoldBall;
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2, iHeroWithBall, iHeroWithoutBall, iEnemyWithBall, iEnemyWithoutBall,iEnemyfar,iEnemynear,exienemyWithBall;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_Direction iAttackDirection;
TC_Position iPassDirection,iWaitingDirection;
TC_Handle hPassDirection ,hWaitingDirection;
CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(_T("Zombie_Three"));
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_MALFOY,L"ADAM");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_GINNY,L"EVE");
}


double getDistance2(TC_Hero *pHero1,TC_Ball *pBall)
{
	return sqrt(1.0*(pHero1->pos.x-pBall->pos.x)*(pHero1->pos.x-pBall->pos.x)+(pHero1->pos.y-pBall->pos.y)*(pHero1->pos.y-pBall->pos.y));
}
/*
TC_Direction GetEnemyDirection(TC_Hero *pHero, TC_Hero *pEnemy)//获得敌人位置
{
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x == pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y > pEnemy->pos.y)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y == pEnemy->pos.y)return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x == pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y < pEnemy->pos.y)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x > pEnemy->pos.x && pHero->pos.y == pEnemy->pos.y)return TC_DIRECTION_LEFT;
}*/
TC_Direction GetBallDirection(TC_Hero *pHero, TC_Ball *pBall)//获得球的位置@@@@@@@
{
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x == pBall->pos.x && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y > pBall->pos.y)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y == pBall->pos.y)return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pBall->pos.x && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x == pBall->pos.x && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y < pBall->pos.y)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x > pBall->pos.x && pHero->pos.y == pBall->pos.y)return TC_DIRECTION_LEFT;
}

int GetState()
{
	/*if(iGoldBall.b_visible)
	{
	//if(getDistance2(&iHero_1,&iGoldBall)<getDistance2(&iHero_2,&iGoldBall))

	//if(sqrt((iHero_1.speed.vx*iHero_1.speed.vx+iHero_1.speed.vy*iHero_1.speed.vy)*1.0)*iGoldBall.u.visible_time_left-20>getDistance2(&iHero_1,&iGoldBall))

	Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGoldBall));
	if(Tc_SnatchBall(hHero_1,TC_GOLD_BALL))
	return  GOLDBALL_REACHABLE;



	//if(sqrt((iHero_2.speed.vx*iHero_2.speed.vx+iHero_2.speed.vy*iHero_2.speed.vy)*1.0)*iGoldBall.u.visible_time_left-20>getDistance2(&iHero_2,&iGoldBall))

	Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGoldBall));
	if(Tc_SnatchBall(hHero_2,TC_GOLD_BALL))
	return  GOLDBALL_REACHABLE;


	}*/
	//if(iGoldBall.b_visible&&)


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


	return 0;
}


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



TC_Direction GetTargetDirection(TC_Hero *pHero, TC_Position *pTarget)
{
	if(pHero->pos.x<pTarget->x&&pHero->pos.y==pTarget->y)
		return TC_DIRECTION_RIGHT;
	if(pHero->pos.x<pTarget->x&&pHero->pos.y>pTarget->y)
		return TC_DIRECTION_RIGHTTOP;
	if(pHero->pos.x<pTarget->x&&pHero->pos.y<pTarget->y)
		return TC_DIRECTION_RIGHTBOTTOM;
	if(pHero->pos.x>pTarget->x&&pHero->pos.y==pTarget->y)
		return TC_DIRECTION_LEFT;
	if(pHero->pos.x>pTarget->x&&pHero->pos.y>pTarget->y)
		return TC_DIRECTION_LEFTTOP;
	if(pHero->pos.x>pTarget->x&&pHero->pos.y<pTarget->y)
		return TC_DIRECTION_LEFTBOTTOM;
	if(pHero->pos.x==pTarget->x&&pHero->pos.y>pTarget->y)
		return TC_DIRECTION_TOP;
	if(pHero->pos.x==pTarget->x&&pHero->pos.y<pTarget->y)
		return TC_DIRECTION_BOTTOM;
}



double getDistance(TC_Hero *pHero1,TC_Hero *pHero2)
{
	return sqrt(1.0*(pHero1->pos.x-pHero2->pos.x)*(pHero1->pos.x-pHero2->pos.x)+(pHero1->pos.y-pHero2->pos.y)*(pHero1->pos.y-pHero2->pos.y));
}


int getWhoisNearer(TC_Hero *pHero1,TC_Hero *pHero2,TC_Hero *pHero3)
{
	if(getDistance(pHero1, pHero2)>getDistance( pHero1,pHero3))
		return 2;
	else 
		return 1;

}
/*TC_Hero getWhoisNearer2(TC_Hero *pHero1,TC_Hero *pHero2,TC_Ball *ball)
{
if(((pHero1->pos.x-ball->pos.x)*(pHero1->pos.x-ball->pos.x)+(pHero1->pos.y-ball->pos.y)*(pHero1->pos.y-ball->pos.y))<((pHero2->pos.x-ball->pos.x)*(pHero2->pos.x-ball->pos.x)+(pHero2->pos.y-ball->pos.y)*(pHero2->pos.y-ball->pos.y)))
return pHero1;
else
return pHero2;
}*/
void SnatchGoldBall(TC_Ball *pBall){
	if(pBall->b_visible/*&&iHero_1.b_snatch_goldball*/)
	{
		Tc_Move(hHero_1,GetBallDirection(&iHero_1,pBall));
		Tc_SnatchBall(hHero_1,TC_GOLD_BALL);

	}
	if(pBall->b_visible/*&&iHero_2.b_snatch_goldball*/)
	{
		Tc_Move(hHero_1,GetBallDirection(&iHero_2,pBall));
		Tc_SnatchBall(hHero_2,TC_GOLD_BALL);

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
		{
			/*


			if(iHero_1.b_ghostball)
			{
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
			if(getWhoisNearer(&iHeroWithBall,&iEnemy_1,&iEnemy_2)==2)
			{

			if(iHeroWithBall.b_can_spell&&iEnemy_2.abnormal_type==0)

			Tc_Spell(hHeroWithBall,hEnemy_2);
			else
			{
			if(iHeroWithBall.abnormal_type==3||iHeroWithBall.abnormal_type==4)
			{
			Tc_Spell(hHero_2,hHero_1);
			Tc_Spell(hHero_1,hEnemy_2);
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithBall));//move 函数的位置待定
			}
			}
			}
			else
			{

			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
			if(getWhoisNearer(&iHeroWithBall,&iEnemy_1,&iEnemy_2)==1)
			{

			if(iHeroWithBall.b_can_spell&&iEnemy_1.abnormal_type==0)

			Tc_Spell(hHeroWithBall,hEnemy_1);
			else
			{
			if(iHeroWithBall.abnormal_type==3||iHeroWithBall.abnormal_type==4)
			{
			Tc_Spell(hHero_2,hHero_1);
			Tc_Spell(hHero_1,hEnemy_1);
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithBall));//move 函数的位置待定
			}
			}
			}
			}

			}
			else
			{
			Tc_Move(hHero_2,GetTargetDirection(&iHero_2,&iPassDirection));
			if (iOwnGate.x < iEnemyGate.x)
			{
			iPassDirection.x=iForbiddenArea.right.left-10;
			iPassDirection.y=(iForbiddenArea.right.bottom+iForbiddenArea.right.top)/2;
			if(iHero_2.pos.x<(iOwnGate.x+iEnemyGate.x)/2-200&&iHero_1.abnormal_type==0)
			{
			Tc_Move(hHero_1,GetTargetDirection(&iHero_1,&iPassDirection));
			//Tc_PassBall(hHero_2, GetTargetDirection(&iHero_1,&iPassDirection));
			Tc_PassBall(hHero_2, iPassDirection);

			if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
			break;
			else
			{
			if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
			break;
			else
			Tc_SnatchBall(hHero_1,TC_GHOST_BALL);

			}

			}
			else
			{
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithoutBall));
			}
			}
			else
			{
			iPassDirection.x=iForbiddenArea.right.left+10;
			iPassDirection.y=(iForbiddenArea.right.bottom+iForbiddenArea.right.top)/2;
			if(iHero_2.pos.x>((iOwnGate.x+iEnemyGate.x)/2+200)&&iHero_1.abnormal_type==0)
			{
			Tc_Move(hHero_1,GetTargetDirection(&iHero_1,&iPassDirection));
			//Tc_PassBall(hHero_2, GetTargetDirection(&iHero_1,&iPassDirection));
			Tc_PassBall(hHero_2, iPassDirection);

			if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
			break;
			else
			{
			if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
			break;
			else
			Tc_SnatchBall(hHero_1,TC_GHOST_BALL);

			}
			}
			else
			{
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			}

			}

			}
			Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
			Tc_Move(hHeroWithoutBall,GetAttackDirection(&iHeroWithBall));
			break;*/
			//SnatchGoldBall(&iGoldBall);
			if (iOwnGate.x < iEnemyGate.x)
			{
				/*if((iEnemy_1.type==5&&iEnemy_1.b_can_spell)||(iEnemy_2.type==5&&iEnemy_2.b_can_spell))
				{
				iWaitingDirection.x=(iForbiddenArea.right.left+iForbiddenArea.left.right)/2;
				iWaitingDirection.y=(iForbiddenArea.right.bottom+iForbiddenArea.right.top)/2;
				Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
				Tc_Move(hHeroWithoutBall,GetTargetDirection(&iHeroWithoutBall,  &iWaitingDirection));
				if(iHeroWithBall.abnormal_type==7)
				{
				if(Tc_SnatchBall(hHeroWithoutBall,TC_GHOST_BALL)||Tc_SnatchBall(hHeroWithBall,TC_GHOST_BALL))
				break;
				else
				break;

				}
				}
				else{*/
				/*if(iEnemy_1.b_is_spelling||iEnemy_2.b_is_spelling)
				{
					if((iHeroWithBall.pos.x+10==iForbiddenArea.right.left&&iOwnGate.x < iEnemyGate.x)||(iHeroWithBall.pos.x-10==iForbiddenArea.right.left&&iOwnGate.x > iEnemyGate.x))
					{
						iPassDirection.y=iForbiddenArea.right.bottom-200;
						Tc_PassBall(hHeroWithBall, iPassDirection);
						if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
							Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
							Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					}

				}*/
				if((iEnemy_1.pos.x<iHeroWithBall.pos.x||iEnemy_2.pos.x<iHeroWithBall.pos.x)&&iHeroWithBall.pos.x<iForbiddenArea.right.left-470)
				{
					iPassDirection.x=iHeroWithBall.pos.x+400;
					iPassDirection.y=(iForbiddenArea.right.bottom+iForbiddenArea.right.top)/2;
					if(Tc_PassBall(hHeroWithBall, iPassDirection))
					{
						if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
						{

							if((getWhoisNearer(&iHeroWithBall,&iEnemy_1,&iEnemy_2)==2)&&iEnemy_2.abnormal_type==0&&iEnemy_2.type!=TC_HERO_HARRY)
							{
								Tc_Spell(hHero_1,hEnemy_2);

								Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
								Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
								//Tc_PassBall(hHeroWithBall, iPassDirection);////////

							}
							else
							{
								/*if(iEnemy_1.b_is_spelling||iEnemy_2.b_is_spelling)
								{
									if(iHeroWithBall.pos.x+10==iForbiddenArea.right.left)
									{
										iPassDirection.y=iForbiddenArea.right.bottom-200;
										Tc_PassBall(hHeroWithBall, iPassDirection);
										if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
											Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
										if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
											Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
									}

								}*/
								if((getWhoisNearer(&iHeroWithBall,&iEnemy_1,&iEnemy_2)==1)&&iEnemy_1.abnormal_type==0&&iEnemy_1.type!=TC_HERO_HARRY)

								{
									Tc_Spell(hHero_1,hEnemy_1);

									Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
									Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
									//Tc_PassBall(hHeroWithBall, iPassDirection);/////you wen ti
								}
							}
						}
					}
					else
					{
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					}


				}
				else
				{
					GetState();
					Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
					Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall, &iGhostBall));

				}
				
			}

			else
			{
				if((iEnemy_1.pos.x>iHeroWithBall.pos.x||iEnemy_2.pos.x>iHeroWithBall.pos.x)&&iHeroWithBall.pos.x>iForbiddenArea.left.right+470)
				{
					iPassDirection.x=iHeroWithBall.pos.x-400;
					iPassDirection.y=(iForbiddenArea.left.bottom+iForbiddenArea.left.top)/2;
					if(Tc_PassBall(hHeroWithBall, iPassDirection))
					{
						if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
						{
							/*if(iEnemy_1.b_is_spelling||iEnemy_2.b_is_spelling)
							{
							if(iHeroWithBall.pos.x-10==iForbiddenArea.left.right)
							{
							iPassDirection.y=iForbiddenArea.left.top-200;
							Tc_PassBall(hHeroWithBall, iPassDirection);
							if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
							Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
							if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
							Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
							}

							}*/
							if((getWhoisNearer(&iHeroWithBall,&iEnemy_1,&iEnemy_2)==2)&&iEnemy_2.abnormal_type==0&&iEnemy_2.type!=TC_HERO_HARRY)
							{

								Tc_Spell(hHero_1,hEnemy_2);

								Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
								Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
								//Tc_PassBall(hHeroWithBall, iPassDirection);////////
							}
							else
							{
								/*if(iEnemy_1.b_is_spelling||iEnemy_2.b_is_spelling)
								{
								if(iHeroWithBall.pos.x-10==iForbiddenArea.left.right)
								{
								iPassDirection.y=iForbiddenArea.left.top-200;
								Tc_PassBall(hHeroWithBall, iPassDirection);
								if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
								Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
								if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
								Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
								}

								}*/
								if((getWhoisNearer(&iHeroWithBall,&iEnemy_1,&iEnemy_2)==1)&&iEnemy_1.abnormal_type==0&&iEnemy_1.type!=TC_HERO_HARRY)

								{
									Tc_Spell(hHero_1,hEnemy_1);

									Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
									Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
									//Tc_PassBall(hHeroWithBall, iPassDirection);/////you wen ti
								}
							}
						}
					}
					else
					{
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					}



				}
				else
				{
					GetState();
					Tc_Move(hHeroWithBall,GetAttackDirection(&iHeroWithBall));
					Tc_Move(hHeroWithoutBall,GetBallDirection(&iHeroWithoutBall, &iGhostBall));


				}
			}
			//SnatchGoldBall(&iGoldBall);
			break;
		}
	case GHOSTBALL_IN_ENEMY_TEAM:
		{
			//GetState();
			//SnatchGoldBall(&iGoldBall);

			if (iOwnGate.x < iEnemyGate.x)
			{
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
				{
					Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
					Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					break;
				}/////待定，可加施法
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
				{
					Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
					break;
				}


				GetState();
				int flag=0;
				if(iEnemyWithBall.pos.x<iForbiddenArea.left.right-10&&iEnemyWithBall.pos.x>iOwnGate.x)
				{
					if(iHero_2.b_can_spell/*&&iEnemyWithBall.abnormal_type==0*/)
					{
						if(Tc_Spell(hHero_2,hEnemyWithBall))
							flag=1;
					//	Tc_SnatchBall(hHero_1,TC_GHOST_BALL);//
					//	Tc_SnatchBall(hHero_2,TC_GHOST_BALL);//
					}
					Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
					Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
					if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					{
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}
					if(iHero_2.b_can_spell&&flag==0/*&&iEnemyWithBall.abnormal_type==0*/)
					{
						GetState();
						if(Tc_Spell(hHero_2,hEnemyWithBall))
						{
							Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
							Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
							if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
							{
								Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
								Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
								break;
							}
						}
					}

					if(iHero_1.b_can_spell/*&&iEnemyWithBall.abnormal_type==0*/)
					{
						GetState();
						Tc_Spell(hHero_1,hEnemyWithBall);
					}
					if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					{
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}

				}
			}
			else
			{

				int flag2=0;
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));

				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
				{
					Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
					Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					break;
				}/////待定，可加施法
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
				{
					Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
					Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
					break;
				}


				GetState();
				if(iEnemyWithBall.pos.x>iForbiddenArea.right.left+10&&iEnemyWithBall.pos.x<iOwnGate.x)
				{
					if(iHero_2.b_can_spell/*&&iEnemyWithBall.abnormal_type==0*/)
					{
						if(Tc_Spell(hHero_2,hEnemyWithBall))
						{

							flag2=1;
						}
					}
					Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
					Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
					if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					{
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}
					if(iHero_2.b_can_spell&&flag2==0/*&&iEnemyWithBall.abnormal_type==0*/)
					{
						GetState();
						if(Tc_Spell(hHero_2,hEnemyWithBall))
						{
							Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
							Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
							if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
							{
								Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
								Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
								break;
							}
						}
					}

					if(iHero_1.b_can_spell/*&&iEnemyWithBall.abnormal_type==0*/)
					{
						GetState();
						Tc_Spell(hHero_1,hEnemyWithBall);
					}
					if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					{
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}

				}
				SnatchGoldBall(&iGoldBall);
				break;
				/*Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
				break;
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
				break;


				GetState();
				if(iEnemyWithBall.pos.x+140>iForbiddenArea.right.left&&iOwnGate.x>iEnemyWithBall.pos.x)
				{
				if(iHero_1.b_can_spell&&iEnemyWithBall.abnormal_type==0)
				// {

				//Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));//待定  与前面的不一致

				Tc_Spell(hHero_1,hEnemyWithBall);
				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
				break;
				// else
				// {
				if(iHero_2.b_can_spell&&iHero_1.abnormal_type!=0&&iHero_1.abnormal_type!=1)

				//{

				//Tc_Move(hHero_2,GetBallDirection(&iHero_1,&iGhostBall));
				Tc_Spell(hHero_2,hEnemyWithBall);
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL)||Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
				break;

				}
				}
				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
				break;
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
				break;
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));


				Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
				if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
				break;
				Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))

				break;
				Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));

				break;*/


			}
			SnatchGoldBall(&iGoldBall);

			break;
		}

	case GHOSTBALL_IN_NO_TEAM:
		{
			GetState();
			SnatchGoldBall(&iGoldBall);
			if(((iEnemy_1.pos.x-iGhostBall.pos.x)*(iEnemy_1.pos.x-iGhostBall.pos.x)+(iEnemy_1.pos.y-iGhostBall.pos.y)*(iEnemy_1.pos.y-iGhostBall.pos.y))<((iEnemy_2.pos.x-iGhostBall.pos.x)*(iEnemy_2.pos.x-iGhostBall.pos.x)+(iEnemy_2.pos.y-iGhostBall.pos.y)*(iEnemy_2.pos.y-iGhostBall.pos.y)))
			{
				hEnemynear=hEnemy_1;
				iEnemynear=iEnemy_1;
				hEnemyfar=hEnemy_2;
				iEnemyfar=iEnemy_2;

			}
			else
			{
				hEnemynear=hEnemy_2;
				iEnemynear=iEnemy_2;
				hEnemyfar=hEnemy_1;
				iEnemyfar=iEnemy_1;

			}
			if(iHero_1.b_can_spell)
			{
				if(iEnemynear.abnormal_type==0&&iEnemynear.curr_blue>iEnemynear.spell_cost)
				{
					Tc_Spell(hHero_1,hEnemynear);
					Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
					Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
					Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
				}
				if (iHero_1.b_snatch_ghostball == false) 
				{
					Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
					Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				}
				else
				{
					if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL)||Tc_SnatchBall(hHero_2,TC_GHOST_BALL)) 
					{
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;

					}
				}
			}
			else
			{
				if (iHero_1.b_snatch_ghostball == false) 
				{
					Tc_Move(hHero_1,GetBallDirection(&iHero_1,&iGhostBall));
				}
				else
				{
					if(Tc_SnatchBall(hHero_1,TC_GHOST_BALL))
					{
						Tc_Move(hHero_1,GetAttackDirection(&iHero_1));
						break;
					}
				}
				if (iHero_2.b_snatch_ghostball == false) 
				{
					Tc_Move(hHero_2,GetBallDirection(&iHero_2,&iGhostBall));
				}
				else
				{
					if(Tc_SnatchBall(hHero_2,TC_GHOST_BALL))
					{
						Tc_Move(hHero_2,GetAttackDirection(&iHero_2));
						break;
					}
				}
			}
			SnatchGoldBall(&iGoldBall);

			break;
		}
	case GOLDBALL_REACHABLE:
		{
			exit(0);
		}
	default:
		{
			break;
		}
	}
}

