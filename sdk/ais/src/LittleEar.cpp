#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <crtdbg.h>

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

TC_Handle hHurmin, hMarfu, hEnemy1, hEnemy2, hGhostball, hFreeball, hGoldball;
TC_Ball ghostball, freeball, goldball;
TC_Hero hurmin, marfu, enemy1, enemy2;
TC_Gate enemygate, owngate;
TC_ForbiddenArea forbidden_area;
bool LEFTGATE, RIGHTGATE; // which gate we should go
int GATE_CENER_Y;

const int DELTA = 4;	// the size of the error
const double SQRT2 = 1.414;
const double INFI = 1e18;
const double NOR_SPEED = 7; // the normal speed
const int COOPERATE_DELTA_X = 200;	// the distance the other hero far from the hero which has the ghostball 
const int COOPERATE_DELTA_Y = 200;
const double PASSBALL_DIST = 300; // when smaller than PASSBALL_DIST, need pass ball
const double SPELL_DIST = 500;	// when smaller than SPELL_DIST, need spell
const int PASSBALL_FORWARD = 600; // pass ball forward 
const double TIME_COLLISION = 30; // next TIME_COLLISTON time may collide
const int STEPS_BEFORE_NEXT_SNATCH = 0;
const int NEAR_TO_ENEMYGATE_DIST = 300; 

void MoveTo(TC_Handle hHero, TC_Position dest);
bool reach(TC_Handle hHero, TC_Position dest);
void spell();
double calSpeed(TC_Ball ball);
double calSpeed(TC_Hero hero);
bool canPassball();
bool needPassball();
bool passball();
bool canSnatchGoldball(TC_Handle hHero);
double dist(TC_Position p1, TC_Position p2);
void getCenter(int& midx, int& midy);
void getGateInfo();
void initAI();
double MinTimeTo(TC_Handle hHero, TC_Position dest);
TC_Handle SelectHero(TC_Position dest, TC_Handle hHero1, TC_Handle hHero2);
void snatchGhostball();
void snatchGoldball();
double sqr(double x);
void toEnemyGate();


void __stdcall Init()
{
	Tc_SetTeamName(TEXT("LittleEar"));

	Tc_ChooseHero(TC_MEMBER1, TC_HERO_HERMIONE, TEXT("hurmin"));
	Tc_ChooseHero(TC_MEMBER2, TC_HERO_MALFOY, TEXT("marfu"));
}

void __stdcall AI()
{
	initAI();


	// snatch the goldball
	if(goldball.b_visible)
	{
		snatchGoldball();
	}

	if(
		enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling ||  // enemy is spelling for snatching the ghostball
		enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling || 
		enemy1.type==TC_HERO_RON && enemy1.b_is_spelling ||
		enemy2.type==TC_HERO_RON && enemy2.b_is_spelling
		)
	{
		passball();
	}

	if( ( (hurmin.b_ghostball && TC_SPELLED_BY_NONE==hurmin.abnormal_type) || 
		  (marfu.b_ghostball && TC_SPELLED_BY_NONE==marfu.abnormal_type) ) // my heroes have the ghostball
		//&& 
		//!( (
		//			enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling ||  // enemy is spelling for snatching the ghostball
		//			 enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling || 
		//			 enemy1.type==TC_HERO_RON && enemy1.b_is_spelling ||
		//			 enemy2.type==TC_HERO_RON && enemy2.b_is_spelling) &&
		//			dist(hurmin.pos, marfu.pos) < 170
		// )
	  )
	{	
		// if can not pass ball, just go to the enemies' gate directly
		if(!(canPassball() && needPassball() && passball())) 
		{
			toEnemyGate();
		}
	}
	else
	{
		snatchGhostball();
	}

}

double sqr(double x)
{
	return x*x;
}
double calSpeed(TC_Hero hero)
{
	return sqrt(sqr(hero.speed.vx)+sqr(hero.speed.vy));
}
double calSpeed(TC_Ball ball)
{
	return sqrt(sqr(ball.speed.vx)+sqr(ball.speed.vy));
}

double dist(TC_Position p1, TC_Position p2)
{
	return sqrt((p1.x-p2.x)*(p1.x-p2.x)*1.0+(p1.y-p2.y)*(p1.y-p2.y));
}

void getCenter(int& midx, int& midy)
{
	midx = (forbidden_area.left.right+forbidden_area.right.left)/2;
	midy = (owngate.y_lower+owngate.y_upper)/2;
}

void toEnemyGate()
{
	TC_Position gatePos, dest;

	spell();

	if(RIGHTGATE)
		gatePos.x = enemygate.x-TC_HERO_HEIGHT/2;
	else
		gatePos.x = enemygate.x+TC_HERO_HEIGHT/2;

	/*

	the ghostball is handled by hurmin

	*/
	if(hurmin.b_ghostball)
	{
		//
		// cooperate
		//
		if((enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling)||
			(enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling))
		{
			/*if( dist(marfu.pos, hurmin.pos)>184 )*/
				getCenter(dest.x, dest.y);
		}
		else
		{
			if(RIGHTGATE)
			{
				dest.x = hurmin.pos.x+COOPERATE_DELTA_X;
				
				//approaching the gate
				if(hurmin.pos.x+TC_HERO_WIDTH>forbidden_area.right.left)
				{
					dest.x = hurmin.pos.x/*-COOPERATE_DELTA_X/2*/;
				}

				if(hurmin.pos.y<enemygate.y_upper)
				{
					dest.y = hurmin.pos.y+COOPERATE_DELTA_Y;

					// not near to the gate, consider passing ball 
					if(hurmin.pos.x<forbidden_area.right.left)
					{
						dest.y = dest.y>GATE_CENER_Y?GATE_CENER_Y:dest.y;
					}
					else
					{
						dest.y = hurmin.pos.y/*-= COOPERATE_DELTA_Y/2*/;
					}
				}
				else
				{
					dest.y = hurmin.pos.y-COOPERATE_DELTA_Y;

					if(hurmin.pos.x<forbidden_area.right.left)
					{
						dest.y = dest.y<GATE_CENER_Y?GATE_CENER_Y:dest.y;
					}
					else
					{
						dest.y = hurmin.pos.y;
					}
				}
				
			}
			else
			{	
				dest.x = hurmin.pos.x-COOPERATE_DELTA_X;
				if(hurmin.pos.x <forbidden_area.left.right)
					dest.x = hurmin.pos.x/*+COOPERATE_DELTA_X/2*/;

				if(hurmin.pos.y<enemygate.y_upper)
				{
					dest.y = hurmin.pos.y+COOPERATE_DELTA_Y;

					// not near to the gate, consider passing ball 
					if(hurmin.pos.x>forbidden_area.left.right)
					{
						dest.y = dest.y>GATE_CENER_Y?GATE_CENER_Y:dest.y;
					}
					else
					{
						dest.y = hurmin.pos.y;
					}
				}
				else
				{
					dest.y = hurmin.pos.y-COOPERATE_DELTA_Y;
					if(hurmin.pos.x>forbidden_area.left.right)
					{
						dest.y = dest.y<GATE_CENER_Y?GATE_CENER_Y:dest.y;
					}
					else
					{
						dest.y = hurmin.pos.y;
					}
				}
			}
		}

		MoveTo(hMarfu, dest);
		//
		// end cooperating
		//


		//
		// to the enemies' gate
		//
		if(hurmin.pos.y>enemygate.y_upper+DELTA && hurmin.pos.y<enemygate.y_lower-TC_HERO_HEIGHT-DELTA)
		{
			if(RIGHTGATE)
				Tc_Move(hHurmin, TC_DIRECTION_RIGHT);
			else
				Tc_Move(hHurmin, TC_DIRECTION_LEFT);
		}
		else if(hurmin.pos.y<=enemygate.y_upper)
		{
			gatePos.y = enemygate.y_upper+TC_HERO_HEIGHT/2;
			MoveTo(hHurmin, gatePos);

			if(reach(hHurmin, gatePos))
			{
				if(RIGHTGATE)
					Tc_Move(hHurmin, TC_DIRECTION_RIGHT);
				else
					Tc_Move(hHurmin, TC_DIRECTION_LEFT);
			}
		}
		else
		{
			gatePos.y = enemygate.y_lower-int(TC_HERO_HEIGHT*1.5);
			MoveTo(hHurmin, gatePos);
			if(reach(hHurmin, gatePos))
			{
				if(RIGHTGATE)
					Tc_Move(hHurmin, TC_DIRECTION_RIGHT);
				else
					Tc_Move(hHurmin, TC_DIRECTION_LEFT);
			}
		}
	}

	/*

	the ghostball is handled by marfu

	*/
	else
	{
		//
		// cooperate
		//
		if((enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling)||
			(enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling))
		{
			getCenter(dest.x, dest.y);
		}
		else
		{
			if(RIGHTGATE)
			{
				dest.x = marfu.pos.x+COOPERATE_DELTA_X;

				//approaching the gate
				if(marfu.pos.x+TC_HERO_WIDTH>forbidden_area.right.left)
				{
					dest.x = marfu.pos.x/*-COOPERATE_DELTA_X/2*/;
				}

				if(marfu.pos.y<enemygate.y_upper)
				{
					dest.y = marfu.pos.y+COOPERATE_DELTA_Y;

					// not near to the gate, consider passing ball 
					if(marfu.pos.x<forbidden_area.right.left)
					{
						dest.y = dest.y>GATE_CENER_Y?GATE_CENER_Y:dest.y;
					}
					else
					{
						dest.y = marfu.pos.y;
					}
				}
				else
				{
					dest.y = marfu.pos.y-COOPERATE_DELTA_Y;
					if(marfu.pos.x<forbidden_area.right.left)
					{
						dest.y = dest.y<GATE_CENER_Y?GATE_CENER_Y:dest.y;
					}
					else
					{
						dest.y = marfu.pos.y;
					}
				}

			}
			else
			{	
				dest.x = marfu.pos.x-COOPERATE_DELTA_X;
				if(marfu.pos.x <forbidden_area.left.right)
					dest.x = marfu.pos.x/*+COOPERATE_DELTA_X/2*/;

				if(marfu.pos.y<enemygate.y_upper)
				{
					dest.y = marfu.pos.y+COOPERATE_DELTA_Y;

					// not near to the gate, consider passing ball 
					if(marfu.pos.x>forbidden_area.left.right)
					{
						dest.y = dest.y>GATE_CENER_Y?GATE_CENER_Y:dest.y;
					}
					else
					{
						dest.y = marfu.pos.y;
					}
				}
				else
				{
					dest.y = marfu.pos.y-COOPERATE_DELTA_Y;
					if(marfu.pos.x>forbidden_area.left.right)
					{
						dest.y = dest.y<GATE_CENER_Y?GATE_CENER_Y:dest.y;
					}
					else
					{
						dest.y = marfu.pos.y;
					}
				}
			}
		}
		MoveTo(hHurmin, dest);
		//
		// end cooperate
		//

		// to the enemies' gate
		if(marfu.pos.y>enemygate.y_upper && marfu.pos.y<enemygate.y_lower-TC_HERO_HEIGHT)
		{
			if(RIGHTGATE)
				Tc_Move(hMarfu, TC_DIRECTION_RIGHT);
			else
				Tc_Move(hMarfu, TC_DIRECTION_LEFT);
		}
		else if(marfu.pos.y<=enemygate.y_upper)
		{
			gatePos.y = enemygate.y_upper+TC_HERO_HEIGHT/2;
			MoveTo(hMarfu, gatePos);
			if(reach(hMarfu, gatePos))
			{
				if(RIGHTGATE)
					Tc_Move(hMarfu, TC_DIRECTION_RIGHT);
				else
					Tc_Move(hMarfu, TC_DIRECTION_LEFT);
			}
		}
		else
		{
			gatePos.y = enemygate.y_lower-int(TC_HERO_HEIGHT*1.5);
			MoveTo(hMarfu, gatePos);
			if(reach(hMarfu, gatePos))
			{
				if(RIGHTGATE)
					Tc_Move(hMarfu, TC_DIRECTION_RIGHT);
				else
					Tc_Move(hMarfu, TC_DIRECTION_LEFT);
			}
		}
	}
}

double MinTimeTo(TC_Handle hHero, TC_Position dest)
{
	TC_Hero hero;
	double t_hero;
	double speedV = NOR_SPEED;
	int dx, dy;

	Tc_GetHeroInfo(hHero, &hero);

	if(TC_SPELLED_BY_MALFOY==hero.abnormal_type)
		speedV = 0;
	if(TC_SPELLED_BY_HERMIONE==hero.abnormal_type)
		speedV = NOR_SPEED/2;
	if(TC_SPELLED_BY_FREEBALL==hero.abnormal_type)
		speedV = 0;

	if(speedV==0)
		return INFI;

	dx = abs(hero.pos.x-dest.x),dy = abs(hero.pos.y-dest.y);
	if(dy<dx)
	{
		int tmp = dx;
		dx = dy;
		dy = tmp;
	}

	t_hero = (dy+(SQRT2-1)*dx)/speedV;

	return t_hero;

}

bool canSnatchGoldball(TC_Handle hHero)
{
	TC_Hero hero;
	TC_Position pos1, pos2;

	Tc_GetHeroInfo(hHero, &hero);

	pos1.x = hero.pos.x+TC_HERO_WIDTH/2;
	pos1.y = hero.pos.y+TC_HERO_WIDTH/2;
	pos2.x = goldball.pos.x+TC_GOLDBALL_WIDTH/2;
	pos2.y = goldball.pos.y+TC_GOLDBALL_WIDTH/2;

	if(goldball.b_visible)
		return dist(pos1, pos2)<TC_SNATCH_DISTANCE_GOLD+DELTA;
	return false;
}

bool reach(TC_Handle hHero, TC_Position dest)
{
	TC_Hero hero;
	Tc_GetHeroInfo(hHero, &hero);
	return (abs(hero.pos.x-dest.x)+abs(hero.pos.y-dest.y)<DELTA);
}

void MoveTo(TC_Handle hHero, TC_Position dest)
{
	int dx, dy;
	TC_Hero hero;
	TC_Position start;


	Tc_GetHeroInfo(hHero, &hero);

	start = hero.pos;
	dx = start.x-dest.x, dy = start.y-dest.y;

	if(abs(dy)-abs(dx)>DELTA)
	{
		if(dy<0)
		{
			Tc_Move(hHero,TC_DIRECTION_BOTTOM);
			if(abs(dest.y-hero.pos.y-abs(dx))<DELTA)
			{
				if(dx<=0) /*   down \  */
				{
					Tc_Move(hHero, TC_DIRECTION_RIGHTBOTTOM);
				}
				else /* down / */
				{
					Tc_Move(hHero, TC_DIRECTION_LEFTBOTTOM);
				}
			}
		}
		if(dy>0)
		{
			Tc_Move(hHero, TC_DIRECTION_TOP);
			if(abs(hero.pos.y-dest.y-abs(dx))<DELTA)
			{	
				if(dx>=0)  /*  up \  */
				{
					Tc_Move(hHero, TC_DIRECTION_LEFTTOP);
				}
				else	/*  up /   */
				{
					Tc_Move(hHero, TC_DIRECTION_RIGHTTOP);
				}
			}
		}
	}
	else if(abs(dx)-abs(dy)>DELTA)
	{
		if(dx>0)
		{
			Tc_Move(hHero, TC_DIRECTION_LEFT);
			if(abs(hero.pos.x-dest.x-abs(dy))<DELTA)
			{
				if(dy<=0) /* left / */
				{
					Tc_Move(hHero, TC_DIRECTION_LEFTBOTTOM);
				}
				else  /* left \ */
				{
					Tc_Move(hHero, TC_DIRECTION_LEFTTOP);
				}
			}
		}
		if(dx<0)
		{
			Tc_Move(hHero, TC_DIRECTION_RIGHT);
			if(abs(dest.x-hero.pos.x-abs(dy))<DELTA)
			{
				if(dy>=0) /* right / */
				{
					Tc_Move(hHero, TC_DIRECTION_RIGHTTOP);
				}
				else	/* right \ */
				{
					Tc_Move(hHero, TC_DIRECTION_RIGHTBOTTOM);
				}
			}
		}
	}
	else
	{
		if( dx>=0 && dy>0 )
			Tc_Move(hHero, TC_DIRECTION_LEFTTOP);
		if( dx>=0 && dy<=0 )
			Tc_Move(hHero, TC_DIRECTION_LEFTBOTTOM);
		if( dx<0 && dy>0 )
			Tc_Move(hHero, TC_DIRECTION_RIGHTTOP);
		if( dx<0 && dy<=0 )
			Tc_Move(hHero, TC_DIRECTION_RIGHTBOTTOM);
	}
}

//
//determine which hero to move or to attack
//
TC_Handle SelectHero(TC_Position dest, TC_Handle hHero1, TC_Handle hHero2)
{
	//TC_Hero hero1, hero2;

	//Tc_GetHeroInfo(hHero1, &hero1);
	//Tc_GetHeroInfo(hHero2, &hero2);
	if(enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling ||
		enemy1.type==TC_HERO_RON && enemy1.b_is_spelling ||
		enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling ||
		enemy2.type==TC_HERO_RON && enemy2.b_is_spelling)
	{
		if (marfu.b_ghostball)
		{
			return hHurmin;
		}
		else
		{
			return hMarfu;
		}
	}

	return MinTimeTo(hHero1, dest)<MinTimeTo(hHero2,dest)?hHero1:hHero2;
}

void spell()
{
	bool CanBeSpelled_Hurmin_Enemy1, CanBeSpelled_Hurmin_Enemy2, CanBeSpelled_Marfu_Enemy1, CanBeSpelled_Marfu_Enemy2;
	CanBeSpelled_Marfu_Enemy1=Tc_CanBeSpelled(hMarfu, hEnemy1);
	CanBeSpelled_Marfu_Enemy2=Tc_CanBeSpelled(hMarfu, hEnemy2);
	CanBeSpelled_Hurmin_Enemy1=Tc_CanBeSpelled(hHurmin, hEnemy1);
	CanBeSpelled_Hurmin_Enemy2=Tc_CanBeSpelled(hHurmin, hEnemy2);

	if( (hurmin.b_can_spell || marfu.b_can_spell)	// can spell

		&& (!(enemy1.speed.vx==0&&enemy1.speed.vy==0) || !(enemy2.speed.vx==0 && enemy2.speed.vy==0)) // need spell

		&& ( CanBeSpelled_Marfu_Enemy1 || CanBeSpelled_Marfu_Enemy2
		||CanBeSpelled_Hurmin_Enemy1 || CanBeSpelled_Hurmin_Enemy2 ) // can be spelled

		) // if
	{
		/*

		my team handle the ghostball

		*/

		// marfu has ghostball
		if(marfu.b_ghostball)
		{
			// close to enemies' gate
			if(( RIGHTGATE && marfu.pos.x>enemygate.x-2*TC_HERO_HEIGHT) 
				|| ( LEFTGATE && marfu.pos.x<enemygate.x+2*TC_HERO_HEIGHT))
			{
				if(enemy1.type==TC_HERO_GINNY)
				{
					Tc_Spell(hMarfu, hEnemy1);
					Tc_Spell(hHurmin, hEnemy1);
				}
				if(enemy2.type==TC_HERO_GINNY)
				{
					Tc_Spell(hMarfu, hEnemy2);
					Tc_Spell(hHurmin, hEnemy2);
				}
			}

			if(MinTimeTo(hEnemy1, marfu.pos)<MinTimeTo(hEnemy2, marfu.pos))
			{
				if(dist(enemy1.pos, marfu.pos)<SPELL_DIST && fabs(calSpeed(enemy1)-NOR_SPEED)<1)
				{	
					Tc_Spell(hMarfu, hEnemy1);
					Tc_Spell(hHurmin, hEnemy1);
					Tc_Spell(hMarfu, hEnemy2);
					Tc_Spell(hHurmin, hEnemy2);
				}
			}
			else
			{
				if(dist(enemy2.pos, marfu.pos)<SPELL_DIST && fabs(calSpeed(enemy2)-NOR_SPEED)<1)
				{
					Tc_Spell(hMarfu, hEnemy2); 
					Tc_Spell(hHurmin, hEnemy2);
					Tc_Spell(hMarfu, hEnemy1);
					Tc_Spell(hHurmin, hEnemy1);
				}
			}
			return;
		}

		// hurmin has ghostball
		if(hurmin.b_ghostball)
		{
			if(( RIGHTGATE && hurmin.pos.x>enemygate.x-2*TC_HERO_HEIGHT) 
				|| ( LEFTGATE && hurmin.pos.x<enemygate.x+2*TC_HERO_HEIGHT))
			{
				if(enemy1.type==TC_HERO_GINNY)
				{
					Tc_Spell(hMarfu, hEnemy1);
					Tc_Spell(hHurmin, hEnemy1);
				}
				if(enemy2.type==TC_HERO_GINNY)
				{
					Tc_Spell(hMarfu, hEnemy2);
					Tc_Spell(hHurmin, hEnemy2);
				}
			}

			if(MinTimeTo(hEnemy1, hurmin.pos)<MinTimeTo(hEnemy2, hurmin.pos))
			{
				if(dist(enemy1.pos, hurmin.pos)<SPELL_DIST && fabs(calSpeed(enemy1)-NOR_SPEED)<1)
				{	
					Tc_Spell(hMarfu, hEnemy1);
					Tc_Spell(hHurmin, hEnemy1);
					Tc_Spell(hMarfu, hEnemy2);
					Tc_Spell(hHurmin, hEnemy2);
				}
			}
			else
			{
				if(dist(enemy2.pos, hurmin.pos)<SPELL_DIST && fabs(calSpeed(enemy2)-NOR_SPEED)<1)
				{
					Tc_Spell(hMarfu, hEnemy2); 
					Tc_Spell(hHurmin, hEnemy2);
					Tc_Spell(hMarfu, hEnemy1);
					Tc_Spell(hHurmin, hEnemy1);
				}
			}
			return;
		}

		/*

		the ghostball is in enemies' hand

		*/

		// enemy1 has the ghostball
		if(enemy1.b_ghostball)
		{
			Tc_Spell(hMarfu, hEnemy1);
			Tc_Spell(hHurmin, hEnemy1);
			Tc_Spell(hMarfu, hEnemy2);
			Tc_Spell(hHurmin, hEnemy2);
			return;
		}

		// enemy2 has the ghostball
		if (enemy2.b_ghostball)
		{
			Tc_Spell(hMarfu, hEnemy2); 
			Tc_Spell(hHurmin, hEnemy2);
			Tc_Spell(hMarfu, hEnemy1);
			Tc_Spell(hHurmin, hEnemy1);
			return;
		}

		/*

		the ghostball has no owner 

		*/

		if(MinTimeTo(hEnemy1, ghostball.pos)<MinTimeTo(hEnemy2, ghostball.pos))
		{
			Tc_Spell(hMarfu, hEnemy1);
			Tc_Spell(hHurmin, hEnemy1);
			Tc_Spell(hMarfu, hEnemy2);
			Tc_Spell(hHurmin, hEnemy2);
		}
		else
		{
			Tc_Spell(hMarfu, hEnemy2); 
			Tc_Spell(hHurmin, hEnemy2);
			Tc_Spell(hMarfu, hEnemy1);
			Tc_Spell(hHurmin, hEnemy1);
		}
	}
}

void snatchGoldball()
{	
	if(hurmin.b_snatch_goldball)
		Tc_SnatchBall(hHurmin, TC_GOLD_BALL);
	if(marfu.b_snatch_goldball)
		Tc_SnatchBall(hMarfu, TC_GOLD_BALL);

	// spell for snatching the goldball
	if(canSnatchGoldball(hEnemy1) || canSnatchGoldball(hEnemy2))
	{
		if(MinTimeTo(hEnemy1, goldball.pos)<MinTimeTo(hEnemy2, goldball.pos))
		{
			Tc_Spell(hMarfu, hEnemy1); // spell the enemy1 first
			Tc_Spell(hHurmin, hEnemy1);
			Tc_Spell(hMarfu, hEnemy2);
			Tc_Spell(hHurmin, hEnemy2);
		}
		else
		{
			Tc_Spell(hMarfu, hEnemy2); // spell the enemy2 first
			Tc_Spell(hHurmin, hEnemy2);
			Tc_Spell(hMarfu, hEnemy1);
			Tc_Spell(hHurmin, hEnemy1);
		}
	}
}

void snatchGhostball()
{
	TC_Handle hHero;
	TC_Position dest;

	spell();
	if(!hurmin.b_ghostball && !marfu.b_ghostball)
	{
		MoveTo(hHurmin, ghostball.pos);
		MoveTo(hMarfu, ghostball.pos);
	}
	else
	{
		hHero = SelectHero(ghostball.pos, hHurmin, hMarfu);

		MoveTo(hHero, ghostball.pos);

		if(ghostball.pos.y<enemygate.y_upper)
		{
			dest.y = ghostball.pos.y+COOPERATE_DELTA_Y;
			if(dest.y>GATE_CENER_Y)
			{
				dest.y = GATE_CENER_Y;
			}
		}
		else
		{
			dest.y = ghostball.pos.y-COOPERATE_DELTA_Y;
			if(dest.y<GATE_CENER_Y)
			{
				dest.y = GATE_CENER_Y;
			}
		}

		if(RIGHTGATE)
		{
			dest.x = ghostball.pos.x+COOPERATE_DELTA_X>forbidden_area.right.left?
				(ghostball.pos.x-COOPERATE_DELTA_X):(ghostball.pos.x+COOPERATE_DELTA_X);
		}
		else
		{
			dest.x = ghostball.pos.x-COOPERATE_DELTA_X<forbidden_area.left.right?
				forbidden_area.left.right:ghostball.pos.x-COOPERATE_DELTA_X;
		}

		if(hHero==hHurmin)
		{
			MoveTo(hMarfu, dest);
		}
		else
		{
			MoveTo(hHurmin, dest);
		}
	}

	if(hurmin.b_snatch_ghostball)
		Tc_SnatchBall(hHurmin, TC_GHOST_BALL);
	if(marfu.b_snatch_ghostball)
		Tc_SnatchBall(hMarfu, TC_GHOST_BALL);
}


bool needPassball()
{
	if((enemy1.type==TC_HERO_RON && enemy1.b_is_spelling) 
		||(enemy2.type==TC_HERO_RON && enemy2.b_is_spelling) 
		||(enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling) 
		||(enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling)
		)
	{
		return true;
	}

	// approach marfu
	if(marfu.b_ghostball)
	{
		if((dist(marfu.pos, enemy1.pos)<PASSBALL_DIST && fabs(calSpeed(enemy1)-NOR_SPEED)<1) || 
			(dist(marfu.pos, enemy2.pos)<PASSBALL_DIST && fabs(calSpeed(enemy2)-NOR_SPEED)<1)
			)
			return true;
	}

	// approach hurmin
	if(hurmin.b_ghostball)
	{
		if((dist(hurmin.pos, enemy1.pos)<PASSBALL_DIST && fabs(calSpeed(enemy1)-NOR_SPEED)<1) ||
			(dist(hurmin.pos, enemy2.pos)<PASSBALL_DIST) && fabs(calSpeed(enemy2)-NOR_SPEED)<1)
			return true;
	}
	return false;
}

bool passball()
{
	/*

	enemies' gate is to the right

	*/
	TC_Position passball_target;

	if(RIGHTGATE)
	{
		if(marfu.b_ghostball)
		{
			if((enemy1.type==TC_HERO_RON && enemy1.b_is_spelling) 
				||(enemy2.type==TC_HERO_RON && enemy2.b_is_spelling) 
				||(enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling) 
				||(enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling)
				)
			{
				//passball_target.x = enemygate.x-TC_BALL_WIDTH-DELTA;
				//if(hurmin.pos.y<GATE_CENER_Y)
				//	passball_target.y = forbidden_area.right.top-TC_BALL_WIDTH-DELTA;
				//else
				//	passball_target.y = forbidden_area.right.bottom+DELTA;
				if(Tc_PassBall(hMarfu, hurmin.pos))
					return true;
			}
			if(hurmin.pos.x>marfu.pos.x)
			{
				passball_target.x = hurmin.pos.x+400;
				passball_target.y = hurmin.pos.y;
			}

			else
			{
				passball_target.x = marfu.pos.x + 510;
				passball_target.y = marfu.pos.y;
			}
			if(marfu.pos.x<forbidden_area.right.left && passball_target.x+TC_BALL_WIDTH>forbidden_area.right.left)
				passball_target.x = forbidden_area.right.left-TC_BALL_WIDTH-DELTA;
			if(dist(marfu.pos, passball_target)>170 && marfu.pos.x<forbidden_area.right.left)
			{
				if(Tc_PassBall(hMarfu, passball_target))
					return true;
			}
		}
		else
		{
			if((enemy1.type==TC_HERO_RON && enemy1.b_is_spelling) 
				||(enemy2.type==TC_HERO_RON && enemy2.b_is_spelling) 
				||(enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling) 
				||(enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling)
				)
			{
				//passball_target.x = enemygate.x-TC_BALL_WIDTH-DELTA;
				//if(marfu.pos.y<GATE_CENER_Y)
				//	passball_target.y = forbidden_area.right.top-TC_BALL_WIDTH-DELTA;
				//else
				//	passball_target.y = forbidden_area.right.bottom+DELTA;
				if(Tc_PassBall(hHurmin, marfu.pos))
					return true;
			}
			if(marfu.pos.x>hurmin.pos.x)
			{
				passball_target.x = marfu.pos.x + 400;
				passball_target.y = marfu.pos.y;
			}

			else
			{
				passball_target.x = hurmin.pos.x + 510;
				passball_target.y = hurmin.pos.y;
			}
			if(hurmin.pos.x<forbidden_area.right.left && passball_target.x+TC_BALL_WIDTH>forbidden_area.right.left)
				passball_target.x = forbidden_area.right.left-TC_BALL_WIDTH-DELTA;
			if(dist(hurmin.pos, passball_target)>170 && hurmin.pos.x<forbidden_area.right.left)
			{
				if(Tc_PassBall(hHurmin, passball_target))
					return true;
			}
		}
	}

	/*

	enemies' gate is to the left

	*/
	else
	{
		if(marfu.b_ghostball)
		{
			if((enemy1.type==TC_HERO_RON && enemy1.b_is_spelling) 
				||(enemy2.type==TC_HERO_RON && enemy2.b_is_spelling) 
				||(enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling) 
				||(enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling)
				)
			{
				//passball_target.x = enemygate.x+DELTA;
				//if(hurmin.pos.y<GATE_CENER_Y)
				//	passball_target.y = forbidden_area.left.top-TC_BALL_WIDTH-DELTA;
				//else
				//	passball_target.y = forbidden_area.left.bottom+DELTA;
				if(Tc_PassBall(hMarfu, hurmin.pos))
					return true;
			}
			if(hurmin.pos.x<marfu.pos.x)
			{
				passball_target.x = hurmin.pos.x - 400;
				passball_target.y = hurmin.pos.y;
			}

			else
			{
				passball_target.x = marfu.pos.x - 510;
				passball_target.y = marfu.pos.y;
			}
			if(marfu.pos.x>forbidden_area.left.right && passball_target.x<forbidden_area.left.right)
				passball_target.x = forbidden_area.left.right+DELTA;
			if(dist(marfu.pos, passball_target)>170 && marfu.pos.x>forbidden_area.left.right)
			{
				if(Tc_PassBall(hMarfu, passball_target))
					return true;
			}
		}
		else
		{
			if((enemy1.type==TC_HERO_RON && enemy1.b_is_spelling) 
				||(enemy2.type==TC_HERO_RON && enemy2.b_is_spelling) 
				||(enemy1.type==TC_HERO_GINNY && enemy1.b_is_spelling) 
				||(enemy2.type==TC_HERO_GINNY && enemy2.b_is_spelling)
				)
			{
				//passball_target.x = enemygate.x+DELTA;
				//if(marfu.pos.y<GATE_CENER_Y)
				//	passball_target.y = forbidden_area.left.top-TC_BALL_WIDTH-DELTA;
				//else
				//	passball_target.y = forbidden_area.left.bottom+DELTA;
				if(Tc_PassBall(hHurmin, marfu.pos))
					return true;
			}
			if(marfu.pos.x<hurmin.pos.x)
			{
				passball_target.x = marfu.pos.x - 400;
				passball_target.y = marfu.pos.y;
			}

			else
			{
				passball_target.x = hurmin.pos.x - 510;
				passball_target.y = hurmin.pos.y;
			}
			if(hurmin.pos.x>forbidden_area.left.right && passball_target.x<forbidden_area.left.right)
				passball_target.x = forbidden_area.left.right+DELTA;
			if(dist(hurmin.pos, passball_target)>170 && hurmin.pos.x>forbidden_area.left.right)
			{
				if(Tc_PassBall(hHurmin, passball_target))
					return true;
			}
		}
	}
	return false;
}

bool canPassball()
{
	// enemy can snatch ghostball
	//if(enemy1.b_snatch_ghostball||enemy2.b_snatch_ghostball)
	//	return false;

	if(marfu.b_ghostball)
	{
		if(abs(enemygate.x-marfu.pos.x)>NEAR_TO_ENEMYGATE_DIST && hurmin.steps_before_next_snatch>STEPS_BEFORE_NEXT_SNATCH)
			return false;
	}
	if(hurmin.b_ghostball)
	{
		if(abs(enemygate.x-hurmin.pos.x)>NEAR_TO_ENEMYGATE_DIST && marfu.steps_before_next_snatch>STEPS_BEFORE_NEXT_SNATCH)
			return false;
	}
	return true;
}

void getGateInfo()
{
	GATE_CENER_Y = (enemygate.y_lower+enemygate.y_upper)/2;
	if(enemygate.x>owngate.x)
	{
		RIGHTGATE = true;
		LEFTGATE = false;
	}
	else
	{
		LEFTGATE = true;
		RIGHTGATE = false;
	}
}

void initAI()
{
	hHurmin = Tc_GetHeroHandle(TC_MEMBER1);
	hMarfu = Tc_GetHeroHandle(TC_MEMBER2);
	hEnemy1 = Tc_GetEnemyHandle(TC_MEMBER1);
	hEnemy2 = Tc_GetEnemyHandle(TC_MEMBER2);
	hGhostball = Tc_GetBallHandle(TC_GHOST_BALL);
	hFreeball = Tc_GetBallHandle(TC_FREE_BALL);
	hGoldball = Tc_GetBallHandle(TC_GOLD_BALL);

	Tc_GetHeroInfo(hHurmin, &hurmin);
	Tc_GetHeroInfo(hMarfu, &marfu);
	Tc_GetEnemyInfo(hEnemy1, &enemy1);
	Tc_GetEnemyInfo(hEnemy2, &enemy2);
	Tc_GetBallInfo(hGhostball, &ghostball);
	Tc_GetBallInfo(hFreeball, &freeball);
	Tc_GetBallInfo(hGoldball, &goldball);
	Tc_GetOwnGate(hHurmin, &owngate);
	Tc_GetEnemyGate(hEnemy1, &enemygate);
	Tc_GetForbiddenArea(&forbidden_area);

	getGateInfo();
}