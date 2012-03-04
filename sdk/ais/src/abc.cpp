//team:abc
//member:杨帆

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
	Tc_SetTeamName(_T("abc"));
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_GINNY,L"Sister");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_RON,L"Brother");
}

#define GHOSTBALL_STOPPED 0
#define GHOSTBALL_BE_PASSEDBYGINNY 1
#define GHOSTBALL_BE_PASSEDBYRON 2
#define GHOSTBALL_BE_PASSEDBYENEMY 3
#define GHOSTBALL_IN_GINNY 4
#define GHOSTBALL_IN_RON 5
#define GHOSTBALL_IN_ENEMY 6

TC_Handle hHero_1, hHero_2;
TC_Handle hEnemy_1, hEnemy_2;
TC_Handle hHeroWithBall, hHeroWithoutBall;
TC_Handle hEnemyWithBall, hEnemyWithoutBall;
TC_Handle hFreeBall, hGhostBall, hGoldBall;
TC_Ball iFreeBall, iGhostBall, iGoldBall;
TC_Hero iHero_1, iHero_2, iEnemy_1, iEnemy_2, iHeroWithBall, iHeroWithoutBall, iEnemyWithBall, iEnemyWithoutBall;
TC_Gate iOwnGate, iEnemyGate;
TC_ForbiddenArea iForbiddenArea;
TC_GameInfo iGameInfo;
TC_Direction iAttackDirection;
TC_Position pMiddle;
TC_Position pMyQuarter;
TC_Position pMyGate;
TC_Position pEnemyGate;
int iOldState=GHOSTBALL_STOPPED;
unsigned iMyScore=0;
unsigned iEnemyScore=0;

int GetState()
{
	if(iHero_1.b_ghostball == false && iHero_2.b_ghostball == false && iEnemy_1.b_ghostball == false && iEnemy_2.b_ghostball == false)
	{
		if((iGhostBall.speed.vx==0 && iGhostBall.speed.vy==0) || iOldState==GHOSTBALL_STOPPED) return GHOSTBALL_STOPPED;
		else
		{
			if(iOldState==GHOSTBALL_IN_GINNY || iOldState==GHOSTBALL_BE_PASSEDBYGINNY) return GHOSTBALL_BE_PASSEDBYGINNY;
			if(iOldState==GHOSTBALL_IN_RON || iOldState==GHOSTBALL_BE_PASSEDBYRON) return GHOSTBALL_BE_PASSEDBYRON;
			if(iOldState==GHOSTBALL_IN_ENEMY || iOldState==GHOSTBALL_BE_PASSEDBYENEMY) return GHOSTBALL_BE_PASSEDBYENEMY;
		}
	}
	if (iHero_1.b_ghostball == true || iHero_2.b_ghostball == true)
	{
		if (iHero_1.b_ghostball == true)
		{
			iHeroWithBall = iHero_1;
			hHeroWithBall = hHero_1;
			iHeroWithoutBall = iHero_2;
			hHeroWithoutBall = hHero_2;
			return GHOSTBALL_IN_GINNY;
		}
		else 
		{
			iHeroWithBall = iHero_2;
			hHeroWithBall = hHero_2;
			iHeroWithoutBall = iHero_1;
			hHeroWithoutBall = hHero_2;
			return GHOSTBALL_IN_RON;
		}
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
		return GHOSTBALL_IN_ENEMY;
	}
	else return -1;
}

TC_Hero* HandleToTag(TC_Handle handle)
{
	if(handle==hHero_1) return &iHero_1;
	if(handle==hHero_2) return &iHero_2;
	if(handle==hEnemy_1) return &iEnemy_1;
	if(handle==hEnemy_2) return &iEnemy_2;
	else return NULL;
}

TC_Direction GetPositionDirection(TC_Hero *pHero,TC_Position *pPos)
{
	if (pHero->pos.x > pPos->x && pHero->pos.y > pPos->y)return TC_DIRECTION_LEFTTOP;
	if (pHero->pos.x == pPos->x && pHero->pos.y > pPos->y)return TC_DIRECTION_TOP;
	if (pHero->pos.x < pPos->x && pHero->pos.y > pPos->y)return TC_DIRECTION_RIGHTTOP;
	if (pHero->pos.x < pPos->x && pHero->pos.y == pPos->y)return TC_DIRECTION_RIGHT;
	if (pHero->pos.x < pPos->x && pHero->pos.y < pPos->y)return TC_DIRECTION_RIGHTBOTTOM;
	if (pHero->pos.x == pPos->x && pHero->pos.y < pPos->y)return TC_DIRECTION_BOTTOM;
	if (pHero->pos.x > pPos->x && pHero->pos.y < pPos->y)return TC_DIRECTION_LEFTBOTTOM;
	if (pHero->pos.x > pPos->x && pHero->pos.y == pPos->y)return TC_DIRECTION_LEFT;
	else return iAttackDirection;
}

TC_Direction GetEnemyGateDirection(TC_Hero *pHero)
{
	if (pHero->pos.y >= iEnemyGate.y_lower-TC_HERO_HEIGHT)
	{
		if (iAttackDirection == TC_DIRECTION_RIGHT)
		{
			if(iEnemyGate.x-pHero->pos.x<64) return TC_DIRECTION_TOP;
			if(iEnemyGate.x-pHero->pos.x<TC_MAP_HEIGHT-(iEnemyGate.y_lower-TC_HERO_HEIGHT)) return TC_DIRECTION_RIGHTTOP;
			else return TC_DIRECTION_RIGHT;
		}
		else
		{
			if(pHero->pos.x-iEnemyGate.x<64) return TC_DIRECTION_TOP;
			if(pHero->pos.x-iEnemyGate.x<TC_MAP_HEIGHT-(iEnemyGate.y_lower-TC_HERO_HEIGHT)) return TC_DIRECTION_LEFTTOP;
			else return TC_DIRECTION_LEFT;
		}
	}
	else
	{
		if (pHero->pos.y <= iEnemyGate.y_upper)
		{
			if (iAttackDirection == TC_DIRECTION_RIGHT)
			{
				if(iEnemyGate.x-pHero->pos.x<64) return TC_DIRECTION_BOTTOM;
				if(iEnemyGate.x-pHero->pos.x<iEnemyGate.y_upper+TC_HERO_HEIGHT) return TC_DIRECTION_RIGHTBOTTOM;
				else return TC_DIRECTION_RIGHT;
			}
			else
			{
				if(pHero->pos.x-iEnemyGate.x<64) return TC_DIRECTION_BOTTOM;
				if(pHero->pos.x-iEnemyGate.x<iEnemyGate.y_upper+TC_HERO_HEIGHT) return TC_DIRECTION_LEFTBOTTOM;
				else return TC_DIRECTION_LEFT;
			}
		}
		else
		{
			return iAttackDirection;
		}
	}
}

int GetDistance(TC_Position *pPos1,TC_Position *pPos2)
{
	return (int)sqrt(double((pPos1->x-pPos2->x)*(pPos1->x-pPos2->x)+(pPos1->y-pPos2->y)*(pPos1->y-pPos2->y)));
}

TC_Handle GetNearerEnemyToHero(TC_Hero *pHero)
{
	if(GetDistance(&(iEnemy_1.pos),&(pHero->pos))<GetDistance(&(iEnemy_2.pos),&(pHero->pos)))
		return hEnemy_1;
	else return hEnemy_2;
}

TC_Handle GetTheOther(TC_Handle handle)
{
	if(handle==hHero_1) return hHero_2;
	if(handle==hHero_2) return hHero_1;
	if(handle==hEnemy_1) return hEnemy_2;
	if(handle==hEnemy_2) return hEnemy_1;
	else return 0;
}

TC_Handle GetNearerHeroToGhostBall()
{
	if(GetDistance(&(iHero_1.pos),&(iGhostBall.pos))<GetDistance(&(iHero_2.pos),&(iGhostBall.pos)))
		return hHero_1;
	else return hHero_2;
}

TC_Handle GetNearerEnemyToGhostBall()
{
	if(GetDistance(&(iEnemy_1.pos),&(iGhostBall.pos))<GetDistance(&(iEnemy_2.pos),&(iGhostBall.pos)))
		return hEnemy_1;
	else return hEnemy_2;
}

TC_Handle GetNearerEnemyToGhostBallTarget()
{
	if(iGhostBall.u.target.x==-1) return GetNearerEnemyToGhostBall();
	if(GetDistance(&(iEnemy_1.pos),&(iGhostBall.u.target))<GetDistance(&(iEnemy_2.pos),&(iGhostBall.u.target)))
		return hEnemy_1;
	else return hEnemy_2;
}

TC_Handle GetNearerEnemyToEnemyGate()
{
	if(GetDistance(&(iEnemy_1.pos),&pEnemyGate)<GetDistance(&(iEnemy_2.pos),&pEnemyGate))
		return hEnemy_1;
	else return hEnemy_2;
}

bool IsHeroNearerToGhostBall()
{
	TC_Handle hhero=GetNearerHeroToGhostBall();
	TC_Handle henemy=GetNearerEnemyToGhostBall();
	TC_Hero *phero=HandleToTag(hhero);
	TC_Hero *penemy=HandleToTag(henemy);
	if(GetDistance(&(phero->pos),&(iGhostBall.pos))+TC_SNATCH_DISTANCE_GHOST<GetDistance(&(penemy->pos),&(iGhostBall.pos)))
		return true;
	else return false;
}

bool IsGhostBallInEnemyQuarter()
{
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if(iGhostBall.pos.x>iForbiddenArea.right.left-TC_HERO_WIDTH-1) return true;
		else return false;
	}
	else
	{
		if(iGhostBall.pos.x<iForbiddenArea.left.right+TC_HERO_WIDTH) return true;
		else return false;
	}
}

bool IsGhostBallInMyHalf()
{
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if(iGhostBall.pos.x<pMiddle.x) return true;
		else return false;
	}
	else
	{
		if(iGhostBall.pos.x>pMiddle.x) return true;
		else return false;
	}
}

bool IsGhostBallInMyQuarter()
{
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if(iGhostBall.pos.x<pMyQuarter.x) return true;
		else return false;
	}
	else
	{
		if(iGhostBall.pos.x>pMyQuarter.x) return true;
		else return false;
	}
}

bool IsHeroInMyHalf(TC_Handle hero)
{
	TC_Hero *pHero=HandleToTag(hero);
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if(pHero->pos.x<pMiddle.x) return true;
		else return false;
	}
	else
	{
		if(pHero->pos.x>pMiddle.x) return true;
		else return false;
	}
}

bool IsHeroInMyQuarter(TC_Handle hero)
{
	TC_Hero *pHero=HandleToTag(hero);
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if(pHero->pos.x<pMyQuarter.x) return true;
		else return false;
	}
	else
	{
		if(pHero->pos.x>pMyQuarter.x) return true;
		else return false;
	}
}

void FollowHero(TC_Handle hToFollow,TC_Handle hBeFollowed)
{
	TC_Hero *pToFollow=HandleToTag(hToFollow);
	TC_Hero *pBeFollowed=HandleToTag(hBeFollowed);
	TC_Position target;
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		target.x=pBeFollowed->pos.x+60;
		target.y=pBeFollowed->pos.y;
	}
	else
	{
		target.x=pBeFollowed->pos.x-30;
		target.y=pBeFollowed->pos.y;
	}
	Tc_Move(hToFollow,GetPositionDirection(pToFollow,&target));
}

void MoveToGhostBall(TC_Handle hHero)
{
	TC_Hero *pHero=HandleToTag(hHero);
	TC_Position point;
	point.y=iGhostBall.pos.y;
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if(iOldState==GHOSTBALL_IN_GINNY || iOldState==GHOSTBALL_IN_RON
			|| iOldState==GHOSTBALL_BE_PASSEDBYGINNY || iOldState==GHOSTBALL_BE_PASSEDBYRON)
			point.x=iGhostBall.pos.x+32;
		else 
		{
			if(iOldState==GHOSTBALL_STOPPED) point.x=iGhostBall.pos.x;
			else point.x=iGhostBall.pos.x-32;
		}
	}
	else
	{
		if(iOldState==GHOSTBALL_IN_GINNY || iOldState==GHOSTBALL_IN_RON || iOldState==GHOSTBALL_STOPPED
			|| iOldState==GHOSTBALL_BE_PASSEDBYGINNY || iOldState==GHOSTBALL_BE_PASSEDBYRON)
			point.x=iGhostBall.pos.x-32;
		else 
		{
			if(iOldState==GHOSTBALL_STOPPED) point.x=iGhostBall.pos.x;
			else point.x=iGhostBall.pos.x+32;
		}
	}
	Tc_Move(hHero,GetPositionDirection(pHero,&point));
}

void MoveToGhostBallTarget(TC_Handle hHero)
{
	TC_Hero *pHero=HandleToTag(hHero);
	if(iGhostBall.u.target.x!=-1) Tc_Move(hHero,GetPositionDirection(pHero,&(iGhostBall.u.target)));
	else MoveToGhostBall(hHero);
}

int GetEnemyNumAhead(TC_Hero *pHero)
{
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if(iEnemy_1.pos.x>pHero->pos.x && iEnemy_2.pos.x>pHero->pos.x) return 2;
		if(iEnemy_1.pos.x<pHero->pos.x && iEnemy_2.pos.x<pHero->pos.x) return 0;
		else return 1;
	}
	else
	{
		if(iEnemy_1.pos.x>pHero->pos.x && iEnemy_2.pos.x>pHero->pos.x) return 0;
		if(iEnemy_1.pos.x<pHero->pos.x && iEnemy_2.pos.x<pHero->pos.x) return 2;
		else return 1;
	}
}

bool PassSelf(TC_Handle hHero)
{
	TC_Hero *pHero=HandleToTag(hHero);
	if(IsGhostBallInEnemyQuarter()==true) return false;
	if(pHero->b_ghostball==false) return false;
	TC_Position target;
	target.y=pHero->pos.y;
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if((iForbiddenArea.right.left-TC_BALL_WIDTH-pHero->pos.x>TC_PASSBALL_DISTANCE && pHero->pos.y>iForbiddenArea.right.top && pHero->pos.y<iForbiddenArea.right.bottom)
			|| pHero->pos.y<iForbiddenArea.right.top || pHero->pos.y>iForbiddenArea.right.bottom) 
			target.x=pHero->pos.x+TC_PASSBALL_DISTANCE;
		else target.x=iForbiddenArea.right.left-TC_BALL_WIDTH-1;
	}
	else
	{
		if((pHero->pos.x-iForbiddenArea.left.right>TC_PASSBALL_DISTANCE && pHero->pos.y>iForbiddenArea.left.top && pHero->pos.y<iForbiddenArea.left.bottom)
			|| pHero->pos.y<iForbiddenArea.left.top || pHero->pos.y>iForbiddenArea.left.bottom) 
			target.x=pHero->pos.x-TC_PASSBALL_DISTANCE;
		else target.x=iForbiddenArea.left.right+1;
	}
	return Tc_PassBall(hHero,target);
}

bool PassForward(TC_Handle hpassfrom)
{
	TC_Handle hpassto=GetTheOther(hpassfrom);
	TC_Hero *ppassfrom=HandleToTag(hpassfrom);
	TC_Hero *ppassto=HandleToTag(hpassto);
	if(ppassfrom->b_ghostball==false) return false;
	if(IsGhostBallInEnemyQuarter()==true) return false;
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if(ppassto->pos.x-ppassfrom->pos.x<60) return false;
	}
	else
	{
		if(ppassfrom->pos.x-ppassto->pos.x<60) return false;
	}
	TC_Position target;
	if(iAttackDirection==TC_DIRECTION_RIGHT)
	{
		if((iForbiddenArea.right.left-TC_BALL_WIDTH-ppassto->pos.x>TC_PASSBALL_DISTANCE && ppassto->pos.y>iForbiddenArea.right.top && ppassto->pos.y<iForbiddenArea.right.bottom) 
			|| ppassto->pos.y<iForbiddenArea.right.top || ppassto->pos.y>iForbiddenArea.right.bottom)
		{
			target.x=ppassto->pos.x+TC_PASSBALL_DISTANCE;
			target.y=ppassto->pos.y;
		}
		else 
		{
			target.x=iForbiddenArea.right.left-TC_BALL_WIDTH-1;
			target.y=ppassto->pos.y;
		}
	}
	else
	{
		if((ppassto->pos.x-iForbiddenArea.left.right>TC_PASSBALL_DISTANCE && ppassto->pos.y>iForbiddenArea.left.top && ppassto->pos.y<iForbiddenArea.left.bottom) 
			|| ppassto->pos.y<iForbiddenArea.left.top || ppassto->pos.y>iForbiddenArea.left.bottom) 
		{
			target.x=ppassto->pos.x-TC_PASSBALL_DISTANCE;
			target.y=ppassto->pos.y;
		}
		else 
		{
			target.x=iForbiddenArea.left.right+1;
			target.y=ppassto->pos.y;
		}
	}
	int dis=GetDistance(&target,&(ppassto->pos));
	if(GetDistance(&target,&(iEnemy_1.pos))>dis && GetDistance(&target,&(iEnemy_2.pos))>dis)
	{
		return Tc_PassBall(hpassfrom,target);
	}
	else return false;
}

bool HasVoldemort()
{
	if(iEnemy_1.type==TC_HERO_VOLDEMORT) return true;
	if(iEnemy_2.type==TC_HERO_VOLDEMORT) return true;
	else return false;
}

TC_Handle GetSpellOrder()
{
	if(iEnemy_1.type==TC_HERO_GINNY) return hEnemy_1;
	if(iEnemy_2.type==TC_HERO_GINNY) return hEnemy_2;
	if(iEnemy_1.type==TC_HERO_MALFOY) return hEnemy_1;
	if(iEnemy_2.type==TC_HERO_MALFOY) return hEnemy_2;
	if(iEnemy_1.type==TC_HERO_HERMIONE) return hEnemy_1;
	if(iEnemy_2.type==TC_HERO_HERMIONE) return hEnemy_2;
	return GetNearerEnemyToGhostBall();
}

double k(TC_Position p1,TC_Position p2)
{
	if(p1.x==p2.x)
	{
		if(iAttackDirection==TC_DIRECTION_RIGHT) return -TC_MAP_HEIGHT;
		else return TC_MAP_HEIGHT;
	}
	return double(p1.y-p2.y)/double(p1.x-p2.x);
}

double b(TC_Position p1,TC_Position p2)
{
	return p1.y-k(p1,p2)*p1.x;
}

TC_Direction GetHeroMovingDirection(TC_Handle hero)
{
	TC_Hero *pHero=HandleToTag(hero);
	if(pHero->speed.vx>0 && pHero->speed.vy==0) return TC_DIRECTION_RIGHT;
	if(pHero->speed.vx>0 && pHero->speed.vx>0) return TC_DIRECTION_RIGHTBOTTOM;
	if(pHero->speed.vx==0 && pHero->speed.vy>0) return TC_DIRECTION_BOTTOM;
	if(pHero->speed.vx<0 && pHero->speed.vy>0) return TC_DIRECTION_LEFTBOTTOM;
	if(pHero->speed.vx<0 && pHero->speed.vy==0) return TC_DIRECTION_LEFT;
	if(pHero->speed.vx<0 && pHero->speed.vy<0) return TC_DIRECTION_LEFTTOP;
	if(pHero->speed.vx==0 && pHero->speed.vy<0) return TC_DIRECTION_TOP;
	if(pHero->speed.vx>0 && pHero->speed.vy<0) return TC_DIRECTION_RIGHTTOP;
	else return iAttackDirection;
}

bool EvadeFreeBall(TC_Handle hero)
{
	Tc_GetBallInfo(hFreeBall,&iFreeBall);
	if(iFreeBall.b_visible==false) return false;
	TC_Hero *pHero=HandleToTag(hero);
	if(pHero->abnormal_type!=TC_SPELLED_BY_NONE) return false;
	TC_Direction direction=GetHeroMovingDirection(hero);
	TC_Position point;
	if(pHero->speed.vx==0 && pHero->speed.vy==0)
	{
		point.x=pHero->pos.x;
		point.y=pHero->pos.y;
	}
	switch(direction)
	{
	case TC_DIRECTION_RIGHT:
		{
			point.x=pHero->pos.x+TC_COLLIDE_WITH_FREEBALL*3;
			point.y=pHero->pos.y;
			break;
		}
	case TC_DIRECTION_RIGHTBOTTOM:
		{
			point.x=pHero->pos.x+(TC_COLLIDE_WITH_FREEBALL*3)/1.414;
			point.y=pHero->pos.y+(TC_COLLIDE_WITH_FREEBALL*3)/1.414;
			break;
		}
	case TC_DIRECTION_BOTTOM:
		{
			point.x=pHero->pos.x;
			point.y=pHero->pos.y+TC_COLLIDE_WITH_FREEBALL*3;
			break;
		}
	case TC_DIRECTION_LEFTBOTTOM:
		{
			point.x=pHero->pos.x-(TC_COLLIDE_WITH_FREEBALL*3)/1.414;
			point.y=pHero->pos.y+(TC_COLLIDE_WITH_FREEBALL*3)/1.414;
			break;
		}
	case TC_DIRECTION_LEFT:
		{
			point.x=pHero->pos.x-TC_COLLIDE_WITH_FREEBALL*3;
			point.y=pHero->pos.y;
			break;
		}
	case TC_DIRECTION_LEFTTOP:
		{
			point.x=pHero->pos.x-(TC_COLLIDE_WITH_FREEBALL*3)/1.414;
			point.y=pHero->pos.y-(TC_COLLIDE_WITH_FREEBALL*3)/1.414;
			break;
		}
	case TC_DIRECTION_TOP:
		{
			point.x=pHero->pos.x;
			point.y=pHero->pos.y-TC_COLLIDE_WITH_FREEBALL*3;
			break;
		}
	case TC_DIRECTION_RIGHTTOP:
		{
			point.x=pHero->pos.x+(TC_COLLIDE_WITH_FREEBALL*3)/1.414;
			point.y=pHero->pos.y-(TC_COLLIDE_WITH_FREEBALL*3)/1.414;
			break;
		}
	}
	TC_Direction purpose;
	if(pHero->speed.vx>0) purpose=TC_DIRECTION_RIGHT;
	else if(pHero->speed.vx<0) purpose=TC_DIRECTION_LEFT;
	else
	{
		if(iOldState==GHOSTBALL_STOPPED || iOldState==GHOSTBALL_IN_GINNY || iOldState==GHOSTBALL_IN_RON || iOldState==GHOSTBALL_BE_PASSEDBYGINNY || iOldState==GHOSTBALL_BE_PASSEDBYRON)
			purpose=iAttackDirection;
		else
		{
			if(iAttackDirection==TC_DIRECTION_RIGHT) purpose=TC_DIRECTION_LEFT;
			else purpose=TC_DIRECTION_RIGHT;
		}
	}
	if(GetDistance(&(pHero->pos),&(iFreeBall.pos))<TC_COLLIDE_WITH_FREEBALL*2.5 ||
		(((pHero->pos.y-pHero->pos.x*k(iFreeBall.u.path.pos_start,iFreeBall.u.path.pos_end)-b(iFreeBall.u.path.pos_start,iFreeBall.u.path.pos_end))
		*(point.y-point.x*k(iFreeBall.u.path.pos_start,iFreeBall.u.path.pos_end)-b(iFreeBall.u.path.pos_start,iFreeBall.u.path.pos_end))<0 
		&& (iFreeBall.u.path.pos_start.y-iFreeBall.u.path.pos_start.x*k(pHero->pos,point)-b(pHero->pos,point))
		*(iFreeBall.u.path.pos_end.y-iFreeBall.u.path.pos_end.x*k(pHero->pos,point)-b(pHero->pos,point))<0) && GetDistance(&(pHero->pos),&(iFreeBall.pos))<TC_COLLIDE_WITH_FREEBALL*3)
		|| GetDistance(&(pHero->pos),&(iFreeBall.u.path.pos_start))<TC_COLLIDE_WITH_FREEBALL*1.5
		|| GetDistance(&(pHero->pos),&(iFreeBall.u.path.pos_end))<TC_COLLIDE_WITH_FREEBALL*1.5)
	{
		double slope=-k(iFreeBall.u.path.pos_start,iFreeBall.u.path.pos_end);
		if(slope>0 && slope<=1)
		{
			if(purpose==TC_DIRECTION_RIGHT) return Tc_Move(hero,TC_DIRECTION_RIGHTTOP);
			else return Tc_Move(hero,TC_DIRECTION_LEFTBOTTOM);
		}
		if(slope>1)
		{
			if(purpose==TC_DIRECTION_RIGHT) return Tc_Move(hero,TC_DIRECTION_TOP);
			else return Tc_Move(hero,TC_DIRECTION_BOTTOM);
		}
		if(slope<-1)
		{
			if(purpose==TC_DIRECTION_RIGHT) return Tc_Move(hero,TC_DIRECTION_BOTTOM);
			else return Tc_Move(hero,TC_DIRECTION_TOP);
		}
		if(slope<0 && slope>=-1)
		{
			if(purpose==TC_DIRECTION_RIGHT) return Tc_Move(hero,TC_DIRECTION_RIGHTBOTTOM);
			else return Tc_Move(hero,TC_DIRECTION_LEFTTOP);
		}
		else return Tc_Move(hero,purpose);
	}
	else return false;
}

void Refresh()
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
	Tc_GetGameInfo(hHero_1,&iGameInfo);
	Tc_GetOwnGate(hHero_1,&iOwnGate);
	Tc_GetEnemyGate(hEnemy_1,&iEnemyGate);
	Tc_GetForbiddenArea(&iForbiddenArea);
	pMiddle.x=TC_MAP_WIDTH/2-TC_BALL_WIDTH/2;
	pMiddle.y=(iOwnGate.y_lower+iOwnGate.y_upper)/2;
	pMyQuarter.x=(pMiddle.x*3+iOwnGate.x)/4;
	pMyQuarter.y=pMiddle.y;
	pMyGate.x=iOwnGate.x-TC_BALL_WIDTH/2;;
	pMyGate.y=(iOwnGate.y_lower+iOwnGate.y_upper)/2-TC_HERO_HEIGHT;
	pEnemyGate.x=iEnemyGate.x-TC_BALL_WIDTH/2;
	pEnemyGate.y=(iEnemyGate.y_lower+iEnemyGate.y_upper)/2-TC_HERO_HEIGHT;
	if (iOwnGate.x < iEnemyGate.x) iAttackDirection = TC_DIRECTION_RIGHT;
	else iAttackDirection = TC_DIRECTION_LEFT;
}

CLIENT_EXPORT_API void __stdcall AI()
{
	Refresh();
	iOldState = GetState();

	if(iGoldBall.b_visible==true)
	{
		if(iHero_1.b_snatch_goldball==true) Tc_SnatchBall(hHero_1,TC_GOLD_BALL);
		if(iHero_2.b_snatch_goldball==true) Tc_SnatchBall(hHero_2,TC_GOLD_BALL);
		if(GetDistance(&(iEnemy_1.pos),&(iGoldBall.pos))<TC_SNATCH_DISTANCE_GOLD*3)
		{
			if(Tc_CanBeSpelled(hHero_2,hEnemy_1)==true) Tc_Spell(hHero_2,hEnemy_1);
			else if(Tc_CanBeSpelled(hHero_1,hEnemy_1)==true) Tc_Spell(hHero_1,hEnemy_1);
		}
		if(GetDistance(&(iEnemy_2.pos),&(iGoldBall.pos))<TC_SNATCH_DISTANCE_GOLD*3)
		{
			if(Tc_CanBeSpelled(hHero_2,hEnemy_2)==true) Tc_Spell(hHero_2,hEnemy_2);
			else if(Tc_CanBeSpelled(hHero_1,hEnemy_2)==true) Tc_Spell(hHero_1,hEnemy_2);
		}
	}
	
	switch (iOldState)
	{
	case GHOSTBALL_IN_GINNY:
		{
			if(PassForward(hHero_1)==false)
			{
				PassSelf(hHero_1);
			}
			if(iHero_1.abnormal_type!=TC_SPELLED_BY_NONE && iHero_2.b_snatch_ghostball==true) Tc_SnatchBall(hHero_2,TC_GHOST_BALL);
			if(EvadeFreeBall(hHero_1)==false) Tc_Move(hHero_1,GetEnemyGateDirection(&iHero_1));
			if(EvadeFreeBall(hHero_2)==false) FollowHero(hHero_2,hHero_1);
			if(((iAttackDirection==TC_DIRECTION_RIGHT && iHero_1.pos.x>pMiddle.x+192) || (iAttackDirection==TC_DIRECTION_LEFT && iHero_1.pos.x<pMiddle.x-192)) && IsGhostBallInEnemyQuarter()==false)
			{
				TC_Handle hEnemyNear=GetNearerEnemyToGhostBall();
				if(GetDistance(&(HandleToTag(hEnemyNear)->pos),&(iHero_1.pos))<TC_SNATCH_DISTANCE_GHOST*2
					&& Tc_CanBeSpelled(hHero_2,hEnemyNear)==true && iHero_1.abnormal_type==TC_SPELLED_BY_NONE)
					Tc_Spell(hHero_2,hEnemyNear);
			}
			break;
		}
	case GHOSTBALL_IN_RON:
		{
			if(PassForward(hHero_2)==false)
			{
				PassSelf(hHero_2);
			}
			if(iHero_2.abnormal_type!=TC_SPELLED_BY_NONE && iHero_1.b_snatch_ghostball==true) Tc_SnatchBall(hHero_1,TC_GHOST_BALL);
			if(EvadeFreeBall(hHero_2)==false) Tc_Move(hHero_2,GetEnemyGateDirection(&iHero_2));
			if(EvadeFreeBall(hHero_1)==false) FollowHero(hHero_1,hHero_2);
			if((iHero_1.curr_blue>iHero_1.spell_cost*2.5 && HasVoldemort()==true) || (iHero_1.curr_blue>iHero_1.spell_cost*1.5 && HasVoldemort()==false))
			{
				if((iAttackDirection==TC_DIRECTION_RIGHT && iHero_2.pos.x>iForbiddenArea.right.left)
					|| (iAttackDirection==TC_DIRECTION_LEFT && iHero_2.pos.x<iForbiddenArea.left.right-TC_HERO_WIDTH))
				{
					TC_Handle hEnemyNearHero=GetSpellOrder();
					if(Tc_CanBeSpelled(hHero_1,hEnemyNearHero)==true) Tc_Spell(hHero_1,hEnemyNearHero);
				}
			}
			break;
		}
	case GHOSTBALL_STOPPED:
		{
			if(EvadeFreeBall(hHero_1)==false) MoveToGhostBallTarget(hHero_1);
			if(EvadeFreeBall(hHero_2)==false) MoveToGhostBallTarget(hHero_2);
			if(iHero_1.b_snatch_ghostball==true && Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) break;
			if(iHero_2.b_snatch_ghostball==true && Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) break;
			break;
		}
	case GHOSTBALL_BE_PASSEDBYGINNY:
		{
			if(EvadeFreeBall(hHero_1)==false) MoveToGhostBallTarget(hHero_1);
			if(EvadeFreeBall(hHero_2)==false) MoveToGhostBallTarget(hHero_2);
			if(iHero_1.b_snatch_ghostball==true && Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) break;
			if(iHero_2.b_snatch_ghostball==true && Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) break;
			break;
		}
	case GHOSTBALL_BE_PASSEDBYRON:
		{
			if(EvadeFreeBall(hHero_2)==false) MoveToGhostBallTarget(hHero_2);
			if(EvadeFreeBall(hHero_1)==false) MoveToGhostBallTarget(hHero_1);
			if(iHero_1.b_snatch_ghostball==true && Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) break;
			if(iHero_2.b_snatch_ghostball==true && Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) break;
			break;
		}
	case GHOSTBALL_BE_PASSEDBYENEMY:
		{
			if(iHero_1.b_snatch_ghostball==true && Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) break;
			if(iHero_2.b_snatch_ghostball==true && Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) break;
			if(EvadeFreeBall(hHero_1)==false) MoveToGhostBallTarget(hHero_1);
			if(EvadeFreeBall(hHero_2)==false) MoveToGhostBallTarget(hHero_2);
			TC_Handle hEnemyNearTarget=GetNearerEnemyToGhostBallTarget();
			if(Tc_CanBeSpelled(hHero_2,hEnemyNearTarget)==true && IsHeroNearerToGhostBall()==true) Tc_Spell(hHero_2,hEnemyNearTarget);
			break;
		}
	case GHOSTBALL_IN_ENEMY:
		{
			if(iHero_1.b_snatch_ghostball==true && Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) break;
			if(iHero_2.b_snatch_ghostball==true && Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) break;
			if(EvadeFreeBall(hHero_1)==false) MoveToGhostBallTarget(hHero_1);
			if(EvadeFreeBall(hHero_2)==false) MoveToGhostBallTarget(hHero_2);
			Tc_GetEnemyInfo(hEnemy_1,&iEnemy_1);
			Tc_GetEnemyInfo(hEnemy_2,&iEnemy_2);
			if(iEnemy_1.b_ghostball==true) 
			{
				hEnemyWithBall=hEnemy_1;
				iEnemyWithBall=iEnemy_1;
			}
			else if(iEnemy_2.b_ghostball==true) 
			{
				hEnemyWithBall=hEnemy_2;
				iEnemyWithBall=iEnemy_2;
			}
			if((iAttackDirection==TC_DIRECTION_RIGHT && iEnemyWithBall.pos.x<iHero_1.pos.x-256 && iEnemyWithBall.pos.x<iHero_2.pos.x-256)
				|| (iAttackDirection==TC_DIRECTION_LEFT && iEnemyWithBall.pos.x>iHero_1.pos.x+256 && iEnemyWithBall.pos.x>iHero_2.pos.x+256))
			{
				if(Tc_CanBeSpelled(hHero_2,hEnemyWithBall)==true && iEnemyWithBall.b_ghostball==true) Tc_Spell(hHero_2,hEnemyWithBall);
			}
			if(IsGhostBallInMyHalf()==false)
			{
				if((iAttackDirection==TC_DIRECTION_RIGHT && iEnemyWithoutBall.pos.x<iEnemyWithBall.pos.x-192)
					|| (iAttackDirection==TC_DIRECTION_LEFT && iEnemyWithoutBall.pos.x>iEnemyWithBall.pos.x+192))
				{
					if(Tc_CanBeSpelled(hHero_2,hEnemyWithBall)==true && iEnemyWithBall.b_ghostball==true) Tc_Spell(hHero_2,hEnemyWithBall);
				}
			}
			else
			{
				if(iHero_1.abnormal_type!=TC_SPELLED_BY_NONE && Tc_CanBeSpelled(hHero_2,hEnemyWithBall) && iEnemyWithBall.b_ghostball==true
					&& iEnemyWithBall.speed.vx*(iEnemyGate.x-iOwnGate.x)<0)
					Tc_Spell(hHero_2,hEnemyWithBall);
				if((iAttackDirection==TC_DIRECTION_RIGHT && iEnemyWithBall.pos.x>iForbiddenArea.left.right+256)
					|| (iAttackDirection==TC_DIRECTION_LEFT && iEnemyWithBall.pos.x<iForbiddenArea.right.left-TC_HERO_WIDTH-256))
				{
					if(iHero_1.curr_blue>iHero_1.spell_cost && iHero_1.b_can_spell==true && iHero_1.abnormal_type==TC_SPELLED_BY_NONE)
						if(EvadeFreeBall(hHero_1)==false) Tc_Move(hHero_1,GetPositionDirection(&iHero_1,&pMiddle));
					else
					{
						if(EvadeFreeBall(hHero_1)==false) MoveToGhostBall(hHero_1);
						if((iHero_1.curr_blue<iHero_1.spell_cost || iHero_1.abnormal_type==TC_SPELLED_BY_VOLDEMORT || iHero_1.abnormal_type==TC_SPELLED_BY_HERMIONE) && Tc_CanBeSpelled(hHero_2,hEnemyWithBall)==true 
							&& iEnemyWithBall.b_ghostball==true	&& iEnemyWithBall.speed.vx*(iEnemyGate.x-iOwnGate.x)<0)
							Tc_Spell(hHero_2,hEnemyWithBall);
					}
				}
				if((iAttackDirection==TC_DIRECTION_RIGHT && iEnemyWithBall.pos.x<iForbiddenArea.left.right)
					|| (iAttackDirection==TC_DIRECTION_LEFT && iEnemyWithBall.pos.x>iForbiddenArea.right.left-TC_HERO_WIDTH))
				{
					if(Tc_CanBeSpelled(hHero_1,hEnemyWithBall)==true && iEnemyWithBall.b_ghostball==true)
					{
						if(Tc_Spell(hHero_1,hEnemyWithBall)==true) break;
					}
					if(iHero_1.b_is_spelling==false && Tc_CanBeSpelled(hHero_2,hEnemyWithBall)==true 
						&& iEnemyWithBall.b_ghostball==true && iEnemyWithBall.speed.vx*(iEnemyGate.x-iOwnGate.x)<0) 
						Tc_Spell(hHero_2,hEnemyWithBall);
				}
			}
			if(iHero_1.b_snatch_ghostball==true && Tc_SnatchBall(hHero_1,TC_GHOST_BALL)==true) break;
			if(iHero_2.b_snatch_ghostball==true && Tc_SnatchBall(hHero_2,TC_GHOST_BALL)==true) break;
			break;
		}
	default:
		{
			break;
		}
	}
}