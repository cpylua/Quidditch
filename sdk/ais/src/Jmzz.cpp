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
	//
	// TODO: Set team name(no Chinese) and choose heros here
	//
	Tc_SetTeamName(L"Jmzz");

	Tc_ChooseHero(TC_MEMBER1, TC_HERO_MALFOY, L"MALFOY");
	Tc_ChooseHero(TC_MEMBER2, TC_HERO_RON, L"RON");


}

//
// TODO: Rename your DLL's filename with your team name
//

#define HEROSPEEDX 5
#define HEROSPEED 7
#define GHOSTBALLSPEEDX 7
#define GHOSTBALLSPEED 10
//#define RON myHero[0]
//#define GINNY myHero[1]
#define MALFOY myHero[0]
#define RON myHero[1]

TC_Ball pGoldBall, pGhostBall, pFreeBall;
TC_Gate pMyGate, pEnemyGate;
int middleLine;
TC_ForbiddenArea forbiddenArea;
TC_Handle hGoldBall, hGhostBall, hFreeBall;
TC_GameInfo gameInfo;
TC_Position pDes1, pDes2, pPassDes={-1,-1};
TC_Direction dEnemyGateDir;
int firsttarget;
bool partnerhasmoved;


class Hero
{
public:
	TC_Hero heromessage;
	TC_Handle handle;
	char side;
	TC_Position center;

public:
	TC_Direction GetDirection(TC_Position from, TC_Position to);
	int GetDistants(TC_Position from, TC_Position to);
	int GetDistants(TC_Ball ball);
	bool IsAt(TC_Position p);

public:
	bool CanSpellTo(Hero & enemy);
	bool CanSnatchGoldBall();
	bool CanSnatchGhostBall();
	bool IsSpelling(Hero & enemy);
	TC_Position CanPassToward();

public:
	bool MoveTo(TC_Position p);
	bool MoveTo(TC_Ball ball);
	bool MoveTo(Hero h);
	bool MoveDir(TC_Direction d);
	bool SnatchGoldBall();
	bool SnatchGhostBall();
	bool PassBall(TC_Position p);
	bool SpellTo(Hero & enemy);

};
Hero myHero[2], enemy[2];
Hero hGhostBallHander, hEnemyWithBallNearGate, hMyHeroNearGoldBall, hEnemyNearGoldBall;


TC_Direction Hero::GetDirection(TC_Position from, TC_Position to)
{
	if (from.x>to.x && abs(from.y-to.y) < HEROSPEED-1)
		return TC_DIRECTION_LEFT;
	if (abs(from.x-to.x) < HEROSPEED-1 && from.y>to.y)
		return TC_DIRECTION_TOP;
	if (from.x<to.x && abs(from.y-to.y) < HEROSPEED-1)
		return TC_DIRECTION_RIGHT;
	if (abs(from.x-to.x) < HEROSPEED-1 && from.y<to.y)
		return TC_DIRECTION_BOTTOM;

	if (from.x>to.x && from.y>to.y)
		return TC_DIRECTION_LEFTTOP;
	if (from.x<to.x && from.y>to.y)
		return TC_DIRECTION_RIGHTTOP;
	if (from.x<to.x && from.y<to.y)
		return TC_DIRECTION_RIGHTBOTTOM;
	if (from.x>to.x && from.y<to.y)
		return TC_DIRECTION_LEFTBOTTOM;

}

int Hero::GetDistants(TC_Position from, TC_Position to)
{
	double dis = sqrt( ( (from.x - to.x)*(from.x - to.x)+(from.y - to.y)*(from.y - to.y) ) *1.0 );
	return int(dis);
}

int Hero::GetDistants(TC_Ball ball)
{
	double dis = sqrt( 
		(heromessage.pos.x+64 - ball.pos.x-32)*(heromessage.pos.x+64 - ball.pos.x-32)
		+
		(heromessage.pos.y+64 - ball.pos.y-32)*(heromessage.pos.y+64 - ball.pos.y-32) *1.0 
		);
	return int(dis);
}

bool Hero::IsAt(TC_Position p)
{
	TC_Position pcenter;
	pcenter.x = p.x + 64;
	pcenter.y = p.y + 64;
	if (GetDistants(pcenter, p) < HEROSPEED)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Hero::CanSpellTo(Hero & enemy)
{
	return (heromessage.b_can_spell && Tc_CanBeSpelled(handle, enemy.handle));
}

bool Hero::CanSnatchGoldBall()
{
	return heromessage.b_snatch_goldball;
}

bool Hero::CanSnatchGhostBall()
{
	return heromessage.b_snatch_ghostball;
}

bool Hero::IsSpelling(Hero &enemy)
{
	return enemy.heromessage.b_is_spelling;
}

TC_Position Hero::CanPassToward()
{
	TC_Position des;
	if (side == 'l')
	{
		des.x = forbiddenArea.right.left - 66;
	}
	else
	{
		des.x = forbiddenArea.left.right + 2;
	}
	if (heromessage.pos.y+64 < pEnemyGate.y_upper)
	{
		des.y = pEnemyGate.y_upper;
	}
	else if (heromessage.pos.y+64 > pEnemyGate.y_lower)
	{
		des.y = pEnemyGate.y_upper;
	}
	else
	{
		des.y = heromessage.pos.y+64;
	}
	if (RON.GetDistants(des, hGhostBallHander.center) > 470)
	{
		des.y = hGhostBallHander.center.y;
		if (side == 'l')
		{
			des.x = hGhostBallHander.center.x + 470;
		}
		else
		{
			des.x = hGhostBallHander.center.x - 470;
		}
	}
	return des;
}

bool Hero::MoveTo(TC_Position p)
{
	TC_Position mycenterpos;
	mycenterpos.x = heromessage.pos.x+64;
	mycenterpos.y = heromessage.pos.y+64;
	if (GetDistants(mycenterpos, p) < HEROSPEED)
	{
		return false;
	}
	return Tc_Move(handle, GetDirection(mycenterpos, p));
}

bool Hero::MoveTo(TC_Ball ball)
{
	TC_Position mycenterpos;
	mycenterpos.x = heromessage.pos.x+64;
	mycenterpos.y = heromessage.pos.y+64;
	TC_Position descenterpos;
	descenterpos.x = ball.pos.x+32;
	descenterpos.y = ball.pos.y+32;
	if (GetDistants(mycenterpos, descenterpos) < 5)
	{
		return false;
	}
	else Tc_Move(handle, GetDirection(mycenterpos, descenterpos));
}

bool Hero::MoveTo(Hero h)
{
	if (GetDistants(heromessage.pos, h.heromessage.pos) < 5)
	{
		return false;
	}
	else Tc_Move(handle, GetDirection(heromessage.pos, h.heromessage.pos));
}

bool Hero::MoveDir(TC_Direction d)
{
	return Tc_Move(handle, d);
}
bool Hero::SnatchGoldBall()
{
	return Tc_SnatchBall(handle, TC_GOLD_BALL);
}

bool Hero::SnatchGhostBall()
{
	return Tc_SnatchBall(handle, TC_GHOST_BALL);
}

bool Hero::PassBall(TC_Position p)
{
	return Tc_PassBall(handle, p);
}

bool Hero::SpellTo(Hero & enemy)
{
	return Tc_Spell(handle, enemy.handle);
}



enum GameStatus
{
	None,
	RPTime,
	Attack,
	Defend,
}gameStatus;


int CountGetTime(TC_Position p1, TC_Position p2);
void UpdateMessage()
{
	//基本信息
	myHero[0].handle=Tc_GetHeroHandle(TC_MEMBER1);
	myHero[1].handle=Tc_GetHeroHandle(TC_MEMBER2);
	enemy[0].handle=Tc_GetEnemyHandle(TC_MEMBER1);
	enemy[1].handle=Tc_GetEnemyHandle(TC_MEMBER2);

	hGoldBall=Tc_GetBallHandle(TC_GOLD_BALL);
	hGhostBall=Tc_GetBallHandle(TC_GHOST_BALL);
	hFreeBall=Tc_GetBallHandle(TC_FREE_BALL);

	Tc_GetOwnGate(myHero[0].handle, &pMyGate);
	Tc_GetEnemyGate(enemy[0].handle, &pEnemyGate);
	Tc_GetForbiddenArea(&forbiddenArea);
	middleLine = (forbiddenArea.left.right + forbiddenArea.right.left)/2;
	if (pEnemyGate.x > middleLine)
	{
		dEnemyGateDir = TC_DIRECTION_RIGHT;
	}
	else
	{
		dEnemyGateDir = TC_DIRECTION_LEFT;
	}

	Tc_GetHeroInfo(myHero[0].handle, &myHero[0].heromessage);
	Tc_GetHeroInfo(myHero[1].handle, &myHero[1].heromessage);
	Tc_GetEnemyInfo(enemy[0].handle, &enemy[0].heromessage);
	Tc_GetEnemyInfo(enemy[1].handle, &enemy[1].heromessage);

	Tc_GetBallInfo(hGhostBall, &pGhostBall);
	Tc_GetBallInfo(hGoldBall, &pGoldBall);
	Tc_GetBallInfo(hFreeBall, &pFreeBall);

	myHero[0].center.x = myHero[0].heromessage.pos.x+64;
	myHero[0].center.y = myHero[0].heromessage.pos.y+64;
	myHero[1].center.x = myHero[1].heromessage.pos.x+64;
	myHero[1].center.y = myHero[1].heromessage.pos.y+64;
	enemy[0].center.x = enemy[0].heromessage.pos.x+64;
	enemy[0].center.y = enemy[0].heromessage.pos.y+64;
	enemy[1].center.x = enemy[1].heromessage.pos.x+64;
	enemy[1].center.y = enemy[1].heromessage.pos.y+64;


	hGhostBallHander.handle = NULL;
	if (myHero[0].heromessage.b_ghostball == true)
	{
		hGhostBallHander = myHero[0];
		pPassDes.x = -1;
	}
	else if (myHero[1].heromessage.b_ghostball == true)
	{
		hGhostBallHander = myHero[1];
		pPassDes.x = -1;
	}
	else if (enemy[0].heromessage.b_ghostball == true)
	{
		hGhostBallHander = enemy[0];
		pPassDes.x = -1;
	}
	else if (enemy[1].heromessage.b_ghostball == true)
	{
		hGhostBallHander = enemy[1];
		pPassDes.x = -1;
	}

	Tc_GetGameInfo(myHero[0].handle, &gameInfo);
	if (pMyGate.x < middleLine)
	{
		myHero[0].side = myHero[1].side = 'l';
		enemy[0].side = enemy[1].side = 'r';
	}
	else
	{
		myHero[0].side = myHero[1].side = 'r';
		enemy[0].side = enemy[1].side = 'l';
	}

	//获取战略状态
	//gameStatus = GameStatus.None;
	hEnemyWithBallNearGate.handle = NULL;
	hEnemyNearGoldBall.handle = NULL;
	hMyHeroNearGoldBall.handle = NULL;

	/*if (pGoldBall.b_visible == true)
	{
	int t1, t2;
	if((t1 = CountGetTime(myHero[0].heromessage.pos, pGoldBall.pos)) < pGoldBall.u.slow_down_steps_left - 64.0/7 + 2)
	{
	hMyHeroNearGoldBall = myHero[0];
	}
	if((t2 = CountGetTime(myHero[1].heromessage.pos, pGoldBall.pos)) < pGoldBall.u.slow_down_steps_left - 64.0/7 + 2
	&&
	t2 < t1)
	{
	hMyHeroNearGoldBall = myHero[1];
	}
	if((t1 = CountGetTime(enemy[0].heromessage.pos, pGoldBall.pos)) < pGoldBall.u.slow_down_steps_left - 64.0/7 + 2)
	{
	hEnemyNearGoldBall = enemy[0];
	}
	if((t2 = CountGetTime(enemy[1].heromessage.pos, pGoldBall.pos)) < pGoldBall.u.slow_down_steps_left - 64.0/7 + 2
	&&
	t2 < t1)
	{
	hEnemyNearGoldBall = enemy[1];
	}
	}*/

	//记录第一目标
	firsttarget = 0;
	if (enemy[0].heromessage.type == TC_HERO_MALFOY)
	{
		firsttarget = 0;
	}
	else if (enemy[1].heromessage.type == TC_HERO_MALFOY)
	{
		firsttarget = 1;
	}
	else if (enemy[0].heromessage.type == TC_HERO_HARRY)
	{
		firsttarget = 0;
	}
	else if (enemy[1].heromessage.type == TC_HERO_HARRY)
	{
		firsttarget = 1;
	}
	else if (enemy[0].heromessage.type == TC_HERO_HERMIONE)
	{
		firsttarget = 0;
	}
	else if (enemy[1].heromessage.type == TC_HERO_HERMIONE)
	{
		firsttarget = 1;
	}

	//对方球员是否持球接近球门
	if(hGhostBallHander.handle == enemy[0].handle || hGhostBallHander.handle == enemy[1].handle)
	{
		if (hGhostBallHander.side == 'l' && pMyGate.x - hGhostBallHander.heromessage.pos.x < 300
			||
			hGhostBallHander.side == 'r' && hGhostBallHander.heromessage.pos.x + TC_HERO_WIDTH - pMyGate.x < 300
			)
		{
			hEnemyWithBallNearGate = hGhostBallHander;
		}
	}

	if (pPassDes.x != pGhostBall.u.target.x && pPassDes.x != -1)
	{
		pPassDes.x = -1;
	}
}

int CountGetTime(TC_Position p1, TC_Position p2)
{
	double disx = fabs(p1.x*1.0 - p2.x);
	double disy = fabs(p1.y*1.0 - p2.y);
	double mindis = disx < disy ? disx : disy;
	return int(mindis/5 + fabs(disx - disy)/7)+1;
}



void Rush(Hero partner)
{

	TC_Position p = hGhostBallHander.CanPassToward() ;
	TC_Position hGhostBallHanderCenter;
	hGhostBallHanderCenter.x = hGhostBallHander.heromessage.pos.x + 64;
	hGhostBallHanderCenter.y = hGhostBallHander.heromessage.pos.y + 64;

	if (p.x != -1 && hGhostBallHander.GetDistants(hGhostBallHanderCenter, p) > 80 
		&&
		abs(hGhostBallHander.heromessage.pos.x - pEnemyGate.x) > abs(p.x - pEnemyGate.x)
		)
	{
		hGhostBallHander.PassBall(p);
		if (partnerhasmoved == false)
		{
			partner.MoveTo(p);
		}
		pPassDes = p;
	}

	else if (hGhostBallHander.heromessage.pos.y+TC_HERO_HEIGHT < pEnemyGate.y_lower && 
		hGhostBallHander.heromessage.pos.y > pEnemyGate.y_upper)
	{
		hGhostBallHander.MoveDir(dEnemyGateDir);
	}
	if (hGhostBallHander.heromessage.pos.y <= pEnemyGate.y_upper)
	{
		TC_Position p;
		if (hGhostBallHander.side = 'l')
		{
			p.x = pEnemyGate.x - TC_HERO_WIDTH;
			p.y = pEnemyGate.y_upper+HEROSPEED+1+64;
		}
		else
		{
			p.x = pEnemyGate.x;
			p.y = pEnemyGate.y_upper+HEROSPEED+1+64;
		}
		hGhostBallHander.MoveTo(p);
	}
	if (hGhostBallHander.heromessage.pos.y >= pEnemyGate.y_lower)
	{
		TC_Position p;
		if (hGhostBallHander.side = 'l')
		{
			p.x = pEnemyGate.x - TC_HERO_WIDTH;
			p.y = pEnemyGate.y_lower - TC_HERO_HEIGHT - HEROSPEED-64-1;
		}
		else
		{
			p.x = pEnemyGate.x;
			p.y = pEnemyGate.y_lower - TC_HERO_HEIGHT - HEROSPEED-64-1;
		}
		hGhostBallHander.MoveTo(p);
	}
	if (partnerhasmoved == false)
	{
		partner.MoveTo(hGhostBallHander);
	}
	if (hGhostBallHander.side == 'l' && hGhostBallHander.center.x > middleLine + 100
		||
		hGhostBallHander.side == 'r' && hGhostBallHander.center.x < middleLine - 100
		)
	{
		if (enemy[0].GetDistants(pGhostBall) < 65 && MALFOY.CanSpellTo(enemy[0]))
		{
			MALFOY.SpellTo(enemy[0]);
		}
		else if (enemy[1].GetDistants(pGhostBall) < 65 && MALFOY.CanSpellTo(enemy[1]))
		{
			MALFOY.SpellTo(enemy[1]);
		}
		if (enemy[0].GetDistants(pGhostBall) < 65 && RON.CanSpellTo(enemy[0]) && !MALFOY.heromessage.b_is_spelling)
		{
			RON.SpellTo(enemy[0]);
		}
		else if (enemy[1].GetDistants(pGhostBall) < 65 && RON.CanSpellTo(enemy[1]) && !MALFOY.heromessage.b_is_spelling)
		{
			RON.SpellTo(enemy[1]);
		}
	}
	
}

CLIENT_EXPORT_API void __stdcall AI()
{
	UpdateMessage();

	//TC_Position tmp;
	//tmp = GetkaweiPos(pGhostBall.pos, myHero[0].heromessage.pos, 64);
	//tmp = GetkaweiPos(pGhostBall.pos, myHero[1].heromessage.pos, 64);
	//tmp = GetkaweiPos(pGhostBall.pos, enemy[0].heromessage.pos, 64);
	//tmp = GetkaweiPos(pGhostBall.pos, enemy[1].heromessage.pos, 64);




#pragma region 金飞贼时间

	if (pGoldBall.b_visible == true)
	{
		if (myHero[0].CanSnatchGoldBall())
		{
			myHero[0].SnatchGoldBall();
			return;
		}
		if (myHero[1].CanSnatchGoldBall())
		{
			myHero[1].SnatchGoldBall();
			return;
		}
	}

#pragma endregion 金飞贼时间


#pragma region 我方进攻
	if (hGhostBallHander.handle == myHero[0].handle || hGhostBallHander.handle == myHero[1].handle)
	{
		partnerhasmoved = false;
		Hero partner;
		if (hGhostBallHander.handle == myHero[0].handle)
		{
			partner = myHero[1];
		}
		else
		{
			partner = myHero[0];
		}

		//针对吹球
		if ((enemy[0].heromessage.type == TC_HERO_GINNY 
			&& 
			enemy[0].heromessage.b_is_spelling )
			||
			(enemy[1].heromessage.type == TC_HERO_GINNY 
			&& 
			enemy[1].heromessage.b_is_spelling )
			||
			(enemy[0].heromessage.type == TC_HERO_RON
			&& 
			enemy[0].heromessage.b_is_spelling )
			||
			(enemy[1].heromessage.type == TC_HERO_RON
			&& 
			enemy[1].heromessage.b_is_spelling )
			)
		{
			TC_Position des;
			if (hGhostBallHander.side == 'l')
			{
				des.x = forbiddenArea.right.left - 66;
			}
			else
			{
				des.x = forbiddenArea.left.right + 2;
			}
			if (hGhostBallHander.heromessage.pos.y+64 < pEnemyGate.y_upper)
			{
				des.y = pEnemyGate.y_upper;
			}
			else if (hGhostBallHander.heromessage.pos.y+64 > pEnemyGate.y_lower)
			{
				des.y = pEnemyGate.y_upper;
			}
			else
			{
				des.y = hGhostBallHander.heromessage.pos.y+64;
			}
		
			hGhostBallHander.PassBall(des);
		}
		if ((enemy[0].heromessage.type == TC_HERO_RON
			&& 
			enemy[0].heromessage.b_is_spelling )
			||
			(enemy[1].heromessage.type == TC_HERO_RON
			&& 
			enemy[1].heromessage.b_is_spelling )
			)
		{
			int x = 1;
		}

		Rush(partner);
	}
#pragma endregion 我方进攻


#pragma region 敌方进攻
	else if(hGhostBallHander.handle == enemy[0].handle || hGhostBallHander.handle == enemy[1].handle)
	{
		Hero enemypartner;
		if (hGhostBallHander.handle == enemy[0].handle)
		{
			enemypartner = enemy[1];
		}
		else
		{
			enemypartner = enemy[0];
		}
		if(myHero[0].CanSnatchGhostBall())
		{
			myHero[0].SnatchGhostBall();
		}
		else if (myHero[1].CanSnatchGhostBall())
		{
			myHero[1].SnatchGhostBall();
		}
		else if (hEnemyWithBallNearGate.handle != NULL 
			&& 
			RON.CanSpellTo(hEnemyWithBallNearGate)
			)
		{
			RON.SpellTo(hEnemyWithBallNearGate);
			MALFOY.MoveTo(pGhostBall);
		}
		else
		{
			RON.MoveTo(hGhostBallHander);
			MALFOY.MoveTo(hGhostBallHander);
		}
	}
#pragma endregion 敌方进攻


#pragma region 抢鬼飞球
	else
	{
		if(myHero[0].CanSnatchGhostBall() && (pPassDes.x == -1 || pGhostBall.speed.vx == 0))
		{
			myHero[0].SnatchGhostBall();
		}
		else if (myHero[1].CanSnatchGhostBall() && (pPassDes.x == -1 || pGhostBall.speed.vx == 0))
		{
			myHero[1].SnatchGhostBall();
		}
		else
		{
			if (pPassDes.x != -1)
			{
				if (!myHero[0].MoveTo(pPassDes))
				{
					myHero[0].MoveTo(pGhostBall.pos);
				}
				if (!myHero[1].MoveTo(pPassDes))
				{
					myHero[1].MoveTo(pGhostBall.pos);
				}
			}
			else
			{
				myHero[0].MoveTo(pGhostBall.pos);
				myHero[1].MoveTo(pGhostBall.pos);
			}
		}
	}
#pragma endregion 抢鬼飞球
	if (gameInfo.game_time_left < 20000)
	{
		if (RON.CanSpellTo(enemy[0]))
		{
			RON.SpellTo(enemy[0]);
		}
		else if (RON.CanSpellTo(enemy[1]))
		{
			RON.SpellTo(enemy[1]);
		}
		if (MALFOY.CanSpellTo(enemy[0]))
		{
			MALFOY.SpellTo(enemy[0]);
		}
		else if (MALFOY.CanSpellTo(enemy[1]))
		{
			MALFOY.SpellTo(enemy[1]);
		}
	}
	//抽魔能放就放
}
