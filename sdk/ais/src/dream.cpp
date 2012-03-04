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


enum Mode {
	DEFEND_MODE=-1,
	GRAB_BALL_MODE=0,
	ATTACK_MODE=1
};

struct RelativeLocation{
	int relative_x;
	int relative_y;
};
struct Direction{
	int dir_x;
	int dir_y;
};


void initialize();

Mode getMode();

Direction getDirectionData(TC_Speed speed);

TC_Direction dataToDiretion(Direction dir);

Direction whereIsTheBall(RelativeLocation rl,Direction enemyDir);

RelativeLocation getRelativeLocation(TC_Position pos1,TC_Position pos2);

TC_Direction getBallDirection(TC_Position pos1,TC_Position pos2);

void grabGhostBall();

void attack();

void defend();

void GoldBall();

int getCriticality(TC_Hero hero,int lastSpell);


TC_Handle hMyHero1,hMyHero2,hEnemyHero1,hEnemyHero2,hFreeBall,hGhostBall,hGoldBall;
TC_Hero myHero1,myHero2,enemyHero1,enemyHero2;
TC_Hero *heroWithGhostBall=NULL,*heroWithGoldBall=NULL;
TC_Ball freeBall,ghostBall,goldBall;
TC_Gate myGate,enemyGate;
TC_ForbiddenArea forbidenArea;
TC_GameInfo gameInfo;
int myLastSpell1=0,myLastSpell2=0,enemyLastSpell1=0,enemyLastSpell2=0;
bool myPassBall=false,enemyPassBall=false;
const int heroSpeed=7,heroSpeedWithBall=5,ghostBallSpeed=12,freeBallSpeed=12;

CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(_T("dream"));
	
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_RON,L"Fly");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_MALFOY,L"Infi");
}


CLIENT_EXPORT_API void __stdcall AI()
{
	initialize();
	
	switch(getMode())
	{
	case 	DEFEND_MODE:
		defend();

		break;
	case GRAB_BALL_MODE:

		grabGhostBall();
		break;

	case ATTACK_MODE:
		attack();

		break;
	}
	
	GoldBall();


}


Direction getDirectionData(TC_Speed speed)
{

	Direction dir;
	if(speed.vx>0)
		dir.dir_x=1;
	else
		if(speed.vx<0)
			dir.dir_x=-1;
		else
			dir.dir_x=0;
	if(speed.vy>0)
		dir.dir_y=1;
	else
		if(speed.vy<0)
			dir.dir_y=-1;
		else
			dir.dir_y=0;
	return dir;
}

TC_Direction dataToDiretion(Direction dir)
{
	if(dir.dir_x>0)
		if(dir.dir_y>0)
			return TC_DIRECTION_RIGHTBOTTOM;
		else
			if(dir.dir_y<0)
				return TC_DIRECTION_RIGHTTOP;
			else
				return TC_DIRECTION_RIGHT;
	else
		if(dir.dir_x<0)
			if(dir.dir_y>0)
			return TC_DIRECTION_LEFTBOTTOM;
		else
			if(dir.dir_y<0)
				return TC_DIRECTION_LEFTTOP;
			else
				return TC_DIRECTION_LEFT;
		else
			if(dir.dir_y>0)
				return TC_DIRECTION_BOTTOM;
			else
				return TC_DIRECTION_TOP;
			
}
Direction whereIsTheBall(RelativeLocation rl,Direction enemyDir)
{
	bool exchanged;
	int x_coe,y_coe;
	if(abs(rl.relative_x)<abs(rl.relative_y))
		exchanged=true;
	else
		exchanged=false;
	if(rl.relative_x<0)
		x_coe=-1;
	else
		x_coe=1;
	if(rl.relative_y<0)
		y_coe=-1;
	else
		y_coe=1;
	Direction changedEnemyDir=enemyDir;
	changedEnemyDir.dir_x*=x_coe;
	changedEnemyDir.dir_y*=y_coe;
	if(exchanged)
	{
		int temp=changedEnemyDir.dir_x;
		changedEnemyDir.dir_x=changedEnemyDir.dir_y;
		changedEnemyDir.dir_y=temp;
	}

	Direction myDir;
	if(changedEnemyDir.dir_x==0&&changedEnemyDir.dir_y==-1)     {		myDir.dir_x=1;		myDir.dir_y=0;	}
	if(changedEnemyDir.dir_x==0&&changedEnemyDir.dir_y==1)		{		myDir.dir_x=1;		myDir.dir_y=1;	}
	if(changedEnemyDir.dir_x==-1&&changedEnemyDir.dir_y==0)     {		myDir.dir_x=1;		myDir.dir_y=1;	}
	if(changedEnemyDir.dir_x==1&&changedEnemyDir.dir_y==0)	    {		myDir.dir_x=1;		myDir.dir_y=1;	}
	if(changedEnemyDir.dir_x==-1&&changedEnemyDir.dir_y==-1)	{		myDir.dir_x=1;		myDir.dir_y=0;	}
	if(changedEnemyDir.dir_x==1&&changedEnemyDir.dir_y==-1)		{		myDir.dir_x=1;		myDir.dir_y=0;	}
	if(changedEnemyDir.dir_x==1&&changedEnemyDir.dir_y==1)	    {		myDir.dir_x=1;		myDir.dir_y=1;	}
	if(changedEnemyDir.dir_x==-1&&changedEnemyDir.dir_y==1)	    {		myDir.dir_x=1;		myDir.dir_y=1;	}
	if(changedEnemyDir.dir_x==0&&changedEnemyDir.dir_y==0)	    {		myDir.dir_x=1;		myDir.dir_y=1;	}
	Direction changedMyDir=myDir;
	if(exchanged)
	{
		int temp=changedMyDir.dir_x;
		changedMyDir.dir_x=changedMyDir.dir_y;
		changedMyDir.dir_y=temp;
	}
	changedMyDir.dir_x*=x_coe;
	changedMyDir.dir_y*=y_coe;
	return changedMyDir;
}
RelativeLocation getRelativeLocation(TC_Position pos1,TC_Position pos2)
{
	RelativeLocation location;
	if(pos2.x>pos1.x)
		if(pos2.y>pos1.y)
			if(pos2.y-pos1.y>pos2.x-pos1.x)
			{
				location.relative_x=1;
				location.relative_y=2;
			}
			else
			{
				location.relative_x=2;
				location.relative_y=1;
			}
		else
			if(pos1.y-pos2.y>pos2.x-pos1.x)
			{
				location.relative_x=1;
				location.relative_y=-2;
			}
			else
			{
				location.relative_x=2;
				location.relative_y=-1;
			}

	else
		if(pos2.y>pos1.y)
			if(pos2.y-pos1.y>pos1.x-pos2.x)
			{
				location.relative_x=-1;
				location.relative_y=2;
			}
			else
			{
				location.relative_x=-2;
				location.relative_y=1;
			}
		else
			if(pos1.y-pos2.y>pos1.x-pos2.x)
			{
				location.relative_x=-1;
				location.relative_y=-2;
			}
			else
			{
				location.relative_x=-2;
				location.relative_y=-1;
			}
			return location;

}
void initialize()
{
	hMyHero1=Tc_GetHeroHandle(TC_MEMBER1);
	hMyHero2=Tc_GetHeroHandle(TC_MEMBER2);
	hEnemyHero1=Tc_GetEnemyHandle(TC_MEMBER1);
	hEnemyHero2=Tc_GetEnemyHandle(TC_MEMBER2);
	hFreeBall=Tc_GetBallHandle(TC_FREE_BALL);
	hGhostBall=Tc_GetBallHandle(TC_GHOST_BALL);
	hGoldBall=Tc_GetBallHandle(TC_GOLD_BALL);

	Tc_GetHeroInfo(hMyHero1,&myHero1);
	Tc_GetHeroInfo(hMyHero2,&myHero2);
	Tc_GetEnemyInfo(hEnemyHero1,&enemyHero1);
	Tc_GetEnemyInfo(hEnemyHero2,&enemyHero2);
	Tc_GetBallInfo(hFreeBall,&freeBall);
	Tc_GetBallInfo(hGhostBall,&ghostBall);
	Tc_GetBallInfo(hGoldBall,&goldBall);
	Tc_GetOwnGate(hMyHero1,&myGate);
	Tc_GetEnemyGate(hEnemyHero1,&enemyGate);
	Tc_GetForbiddenArea(&forbidenArea);
	if(myHero1.b_ghostball==true)
		heroWithGhostBall=&myHero1;
	else
	    if(myHero2.b_ghostball==true)
	    	heroWithGhostBall=&myHero2;
	     else 
			 if(enemyHero1.b_ghostball==true)
	            	heroWithGhostBall=&enemyHero1;
	          else 
				  if(enemyHero2.b_ghostball==true)
	                	heroWithGhostBall=&enemyHero2;
				  else heroWithGhostBall=NULL;

		  if(myHero1.b_is_spelling)
			  myLastSpell1=myHero1.spell_interval;
			  else
				  myLastSpell1--;
		  if(myHero2.b_is_spelling)
				  myLastSpell2=myHero2.spell_interval;
			  else
				  myLastSpell2--;
		  if(enemyHero1.b_is_spelling)
				 enemyLastSpell1=enemyHero1.spell_interval;
			  else
				 enemyLastSpell1--;
		  if(enemyHero2.b_is_spelling)
				  enemyLastSpell2=enemyHero2.spell_interval;
			  else
				  enemyLastSpell2--;


}

Mode getMode()
{
	if(enemyHero1.b_ghostball==true||enemyHero2.b_ghostball==true)
		return DEFEND_MODE;
	else
		if(myHero1.b_ghostball==true||myHero2.b_ghostball==true)
			return ATTACK_MODE;
		else
			return GRAB_BALL_MODE;
}


TC_Direction getBallDirection(TC_Position pos1,TC_Position pos2)
{
	int length=5;
	if(pos1.x+TC_HERO_WIDTH/2>pos2.x+TC_BALL_WIDTH/2+length)
		if(pos1.y+TC_HERO_HEIGHT/2>pos2.y+TC_BALL_HEIGHT/2+length)
			return TC_DIRECTION_LEFTTOP;
		else
			if(pos1.y+TC_HERO_HEIGHT/2<pos2.y+TC_BALL_HEIGHT/2-length)
				return TC_DIRECTION_LEFTBOTTOM;
			else
				return TC_DIRECTION_LEFT;
	else
		if(pos1.x+TC_HERO_WIDTH/2<pos2.x+TC_BALL_WIDTH/2-length)
			if(pos1.y+TC_HERO_HEIGHT/2>pos2.y+TC_BALL_HEIGHT/2+length)
		    	return TC_DIRECTION_RIGHTTOP;
	       	else
	    		 if(pos1.y+TC_HERO_HEIGHT/2<pos2.y+TC_BALL_HEIGHT/2-length)
		     		return TC_DIRECTION_RIGHTBOTTOM;
		    	else
		     		return TC_DIRECTION_RIGHT;
		else
			if(pos1.y+TC_HERO_HEIGHT/2>pos2.y+TC_BALL_HEIGHT/2+length)
				return TC_DIRECTION_TOP;
			else
				return TC_DIRECTION_BOTTOM;
      
}  
void grabGhostBall()
{
	if(ghostBall.u.target.x==-1&&ghostBall.u.target.y==-1)
	{
		myPassBall=false;
		enemyPassBall=false;
	}
	else
	{
		enemyPassBall=!myPassBall;
	}

	if(myPassBall==false&&enemyPassBall==false)
	{

		
			Tc_Move(hMyHero1,getBallDirection(myHero1.pos,ghostBall.pos));
			Tc_Move(hMyHero2,getBallDirection(myHero2.pos,ghostBall.pos));

	}
	if(myPassBall==true||enemyPassBall==true)
	{
		Tc_Move(hMyHero1,getBallDirection(myHero1.pos,ghostBall.u.target));
		Tc_Move(hMyHero2,getBallDirection(myHero2.pos,ghostBall.u.target));
	}



	if(	myHero1.b_snatch_ghostball	)
     		Tc_SnatchBall(hMyHero1,TC_GHOST_BALL);
	if(	myHero2.b_snatch_ghostball	)
     		Tc_SnatchBall(hMyHero2,TC_GHOST_BALL);
}
void attack()
{
	myPassBall=false;
	enemyPassBall=false;
	TC_Handle hHeroWithBall,hHeroWithNoBall;
	TC_Hero *heroWithBall,*heroWithNoBall;
	if(myHero1.b_ghostball==true)
	{
		hHeroWithBall=hMyHero1;
		hHeroWithNoBall=hMyHero2;
		heroWithBall=&myHero1;
		heroWithNoBall=&myHero2;
	}
	else
	{
		hHeroWithBall=hMyHero2;
		hHeroWithNoBall=hMyHero1;
		heroWithBall=&myHero2;
		heroWithNoBall=&myHero1;
	}

	if(myGate.x<enemyGate.x)
	{ 
		if(heroWithBall->pos.y<=enemyGate.y_upper)
				Tc_Move(hHeroWithBall,TC_DIRECTION_RIGHTBOTTOM);
			else
				if(heroWithBall->pos.y+TC_HERO_HEIGHT>=enemyGate.y_lower)
					Tc_Move(hHeroWithBall,TC_DIRECTION_RIGHTTOP);
				else
					Tc_Move(hHeroWithBall,TC_DIRECTION_RIGHT);
	
		
  
		if(heroWithBall->pos.x+TC_HERO_WIDTH<forbidenArea.right.left-64)
		{
			/*
			if((enemyHero1.pos.y>heroWithBall->pos.y+20)
				&&(enemyHero2.pos.y>heroWithBall->pos.y+20))
			{
				TC_Position pos;
				pos.x=ghostBall.pos.x+(forbidenArea.right.left-ghostBall.pos.x-TC_BALL_WIDTH<400?forbidenArea.right.left-ghostBall.pos.x-TC_BALL_WIDTH:400);
				pos.y=ghostBall.pos.y-240;
				Tc_PassBall(hHeroWithBall,pos);
			}
			if((enemyHero1.pos.y<heroWithBall->pos.y-20)
				&&(enemyHero2.pos.y<heroWithBall->pos.y-20))

			{
				TC_Position pos;
				pos.x=ghostBall.pos.x+(forbidenArea.right.left-ghostBall.pos.x-TC_BALL_WIDTH<400?forbidenArea.right.left-ghostBall.pos.x-TC_BALL_WIDTH:400);
				pos.y=ghostBall.pos.y+240;
				Tc_PassBall(hHeroWithBall,pos);
			}
			*/
			if((enemyHero1.pos.x<heroWithBall->pos.x-20||enemyHero1.pos.x>heroWithBall->pos.x+512||enemyHero1.steps_before_next_snatch>12)
				&&(enemyHero2.pos.x<heroWithBall->pos.x-20||enemyHero2.pos.x>heroWithBall->pos.x+512||enemyHero2.steps_before_next_snatch>12))
			{
				TC_Position pos;
				pos.x=ghostBall.pos.x+(forbidenArea.right.left-ghostBall.pos.x-TC_BALL_WIDTH<512?forbidenArea.right.left-ghostBall.pos.x-TC_BALL_WIDTH:512);
				pos.y=ghostBall.pos.y;
				Tc_PassBall(hHeroWithBall,pos);
			}
			
		}
		if((enemyHero1.type==TC_HERO_RON||enemyHero1.type==TC_HERO_GINNY)&&enemyHero1.b_is_spelling==true
			||(enemyHero2.type==TC_HERO_RON||enemyHero2.type==TC_HERO_GINNY)&&enemyHero2.b_is_spelling==true
			)
		{
			TC_Position pos=ghostBall.pos;
			if(ghostBall.pos.x+TC_BALL_WIDTH/2>1024&&ghostBall.pos.x+TC_BALL_WIDTH<forbidenArea.right.left)
			{
				pos.x=ghostBall.pos.x+(forbidenArea.right.left-ghostBall.pos.x-TC_BALL_WIDTH<512?forbidenArea.right.left-ghostBall.pos.x-TC_BALL_WIDTH:512);
				pos.y=ghostBall.pos.y;
				Tc_PassBall(hHeroWithBall,pos);
			}
			if(ghostBall.pos.x+64>=forbidenArea.right.left)
			{
				pos.x=forbidenArea.right.left-TC_BALL_WIDTH;
					pos.y=ghostBall.pos.y;
				Tc_PassBall(hHeroWithBall,pos);

			}
			

		}
		
	}
	else
	{
		
			if(heroWithBall->pos.y<=enemyGate.y_upper)
				Tc_Move(hHeroWithBall,TC_DIRECTION_LEFTBOTTOM);
			else
				if(heroWithBall->pos.y+TC_HERO_HEIGHT>=enemyGate.y_lower)
					Tc_Move(hHeroWithBall,TC_DIRECTION_LEFTTOP);
				else
					Tc_Move(hHeroWithBall,TC_DIRECTION_LEFT);
	


		if(heroWithBall->pos.x>forbidenArea.left.right+64)
		{
			/*
			if((enemyHero1.pos.y>heroWithBall->pos.y)
				&&(enemyHero2.pos.y>heroWithBall->pos.y))
			{
				TC_Position pos;
				pos.x=ghostBall.pos.x-(ghostBall.pos.x-forbidenArea.left.right<400?ghostBall.pos.x-forbidenArea.left.right:400);
				pos.y=ghostBall.pos.y-240;
				Tc_PassBall(hHeroWithBall,pos);
			}
			if((enemyHero1.pos.y<heroWithBall->pos.y)
				&&(enemyHero2.pos.y<heroWithBall->pos.y))
			{
				TC_Position pos;
				pos.x=ghostBall.pos.x-(ghostBall.pos.x-forbidenArea.left.right<400?ghostBall.pos.x-forbidenArea.left.right:400);
				pos.y=ghostBall.pos.y+240;
				Tc_PassBall(hHeroWithBall,pos);
			}
			*/
			if((enemyHero1.pos.x>heroWithBall->pos.x+20||enemyHero1.pos.x<heroWithBall->pos.x-512||enemyHero1.steps_before_next_snatch>12)
				&&(enemyHero2.pos.x>heroWithBall->pos.x+20||enemyHero2.pos.x<heroWithBall->pos.x-512||enemyHero2.steps_before_next_snatch>12))
			{
				TC_Position pos;
				pos.x=ghostBall.pos.x-(ghostBall.pos.x-forbidenArea.left.right<512?ghostBall.pos.x-forbidenArea.left.right:512);
				pos.y=ghostBall.pos.y;
				Tc_PassBall(hHeroWithBall,pos);
			}

			
		}
		if((enemyHero1.type==TC_HERO_RON||enemyHero1.type==TC_HERO_GINNY)&&enemyHero1.b_is_spelling==true
			||(enemyHero2.type==TC_HERO_RON||enemyHero2.type==TC_HERO_GINNY)&&enemyHero2.b_is_spelling==true
			)
		{
			TC_Position pos=ghostBall.pos;
			if(ghostBall.pos.x+TC_BALL_WIDTH/2<1024&&ghostBall.pos.x>forbidenArea.left.right)
			{
				pos.x=ghostBall.pos.x-(ghostBall.pos.x-forbidenArea.left.right<512?ghostBall.pos.x-forbidenArea.left.right:512);
				pos.y=ghostBall.pos.y;
				Tc_PassBall(hHeroWithBall,pos);
			}
			if(ghostBall.pos.x<=forbidenArea.left.right)
			{
				pos.x=forbidenArea.left.right;
				pos.y=ghostBall.pos.y;
				Tc_PassBall(hHeroWithBall,pos);

			}
			

		}
		

	}




	TC_Position hcpos=heroWithNoBall->pos;
		hcpos.x+=TC_HERO_WIDTH/2;
		hcpos.y+=TC_HERO_HEIGHT/2;
		TC_Position bcpos=ghostBall.pos;
		bcpos.x+=TC_BALL_WIDTH/2;
		bcpos.y+=TC_BALL_HEIGHT/2;


		if(myGate.x<enemyGate.x)
		{
			TC_Position followPos=bcpos;
			followPos.x-=TC_HERO_WIDTH/2;
	    	Tc_Move(hHeroWithNoBall,dataToDiretion(whereIsTheBall(getRelativeLocation(hcpos,followPos),getDirectionData(heroWithBall->speed))));
		}
		else
		{
			TC_Position followPos=bcpos;
			followPos.x+=TC_HERO_WIDTH/2;
	    	Tc_Move(hHeroWithNoBall,dataToDiretion(whereIsTheBall(getRelativeLocation(hcpos,followPos),getDirectionData(heroWithBall->speed))));
		}
		
		if(heroWithBall->abnormal_type!=TC_SPELLED_BY_NONE)
		{
			if(heroWithNoBall->b_snatch_ghostball==true)
				Tc_SnatchBall(hHeroWithNoBall,TC_GHOST_BALL);
			else
			{
	
			Tc_Move(hHeroWithNoBall,dataToDiretion(whereIsTheBall(getRelativeLocation(hcpos,bcpos),getDirectionData(heroWithBall->speed))));

			}
		}
//==============================施法====================
		int spell1=0;
		int spell2=0;

		if(myGate.x<enemyGate.x)
		{
			if(ghostBall.pos.x+64>forbidenArea.right.left+150)
			{
				if(Tc_CanBeSpelled(hMyHero1,hEnemyHero1)&&enemyHero1.pos.x+64>ghostBall.pos.x+32-512)
				{Tc_Spell(hMyHero1,hEnemyHero1);spell1=1;}
				if(Tc_CanBeSpelled(hMyHero1,hEnemyHero2)&&enemyHero2.pos.x+64>ghostBall.pos.x+32-512)
				{Tc_Spell(hMyHero1,hEnemyHero2);spell1=2;}
			}
		}
		else
		{
			if(ghostBall.pos.x<forbidenArea.left.right-150)
				{
					if(Tc_CanBeSpelled(hMyHero1,hEnemyHero1)&&enemyHero1.pos.x+64<ghostBall.pos.x+32+512)
						 {Tc_Spell(hMyHero1,hEnemyHero1);spell1=1;}
					if(Tc_CanBeSpelled(hMyHero1,hEnemyHero2)&&enemyHero1.pos.x+64<ghostBall.pos.x+32+512)
						{Tc_Spell(hMyHero1,hEnemyHero2);spell1=2;}
				}
		}

		if(myGate.x<enemyGate.x)
		{
			if(ghostBall.pos.x+64>forbidenArea.right.left)
			{
				if(Tc_CanBeSpelled(hMyHero2,hEnemyHero1)&&getCriticality(enemyHero1,enemyLastSpell1)<1&&spell1!=1&&enemyHero1.pos.x+64>ghostBall.pos.x+32-512)
					  Tc_Spell(hMyHero2,hEnemyHero1);
				if(Tc_CanBeSpelled(hMyHero2,hEnemyHero2)&&getCriticality(enemyHero2,enemyLastSpell2)<1&&spell1!=2&&enemyHero2.pos.x+64>ghostBall.pos.x+32-512)
					  Tc_Spell(hMyHero2,hEnemyHero2);
				
			}
			if(ghostBall.pos.x+64>forbidenArea.right.left-50)
			{
			
			if(Tc_CanBeSpelled(hMyHero2,hEnemyHero1)&&getCriticality(enemyHero1,enemyLastSpell1)<2&&spell1!=1&&(enemyHero1.type==TC_HERO_RON||enemyHero1.type==TC_HERO_GINNY)&&enemyHero1.pos.x+64>ghostBall.pos.x+32-512)
					  Tc_Spell(hMyHero2,hEnemyHero1);
				if(Tc_CanBeSpelled(hMyHero2,hEnemyHero2)&&getCriticality(enemyHero2,enemyLastSpell2)<2&&spell1!=2&&(enemyHero2.type==TC_HERO_RON||enemyHero2.type==TC_HERO_GINNY)&&enemyHero2.pos.x+64>ghostBall.pos.x+32-512)
					  Tc_Spell(hMyHero2,hEnemyHero2);
			}
		}
			else
			{
				if(ghostBall.pos.x<forbidenArea.left.right)
				{
					if(Tc_CanBeSpelled(hMyHero2,hEnemyHero1)&&getCriticality(enemyHero1,enemyLastSpell1)<1&&spell1!=1&&enemyHero1.pos.x+64<ghostBall.pos.x+32+512)
						  Tc_Spell(hMyHero2,hEnemyHero1);
					if(Tc_CanBeSpelled(hMyHero2,hEnemyHero2)&&getCriticality(enemyHero2,enemyLastSpell2)<1&&spell1!=2&&enemyHero2.pos.x+64<ghostBall.pos.x+32+512)
						  Tc_Spell(hMyHero2,hEnemyHero2);
					
				}
				if(ghostBall.pos.x<forbidenArea.left.right+50)
				{
					if(Tc_CanBeSpelled(hMyHero2,hEnemyHero1)&&getCriticality(enemyHero1,enemyLastSpell1)<1&&spell1!=1&&(enemyHero1.type==TC_HERO_RON||enemyHero1.type==TC_HERO_GINNY)&&enemyHero1.pos.x+64<ghostBall.pos.x+32+512)
					  Tc_Spell(hMyHero2,hEnemyHero1);
					if(Tc_CanBeSpelled(hMyHero2,hEnemyHero2)&&getCriticality(enemyHero2,enemyLastSpell2)<1&&spell1!=2&&(enemyHero2.type==TC_HERO_RON||enemyHero2.type==TC_HERO_GINNY)&&enemyHero2.pos.x+64<ghostBall.pos.x+32+512)
					  Tc_Spell(hMyHero2,hEnemyHero2);
				}
			}





}
void defend()
{
	myPassBall=false;
	enemyPassBall=false;

	TC_Handle hHeroWithBall,hHeroWithNoBall;
	TC_Hero *heroWithBall,*heroWithNoBall;
	if(enemyHero1.b_ghostball==true)
	{
		hHeroWithBall=hEnemyHero1;
		hHeroWithNoBall=hEnemyHero2;
		heroWithBall=&enemyHero1;
		heroWithNoBall=&enemyHero2;
	}
	else
	{
		hHeroWithBall=hEnemyHero2;
		hHeroWithNoBall=hEnemyHero1;
		heroWithBall=&enemyHero2;
		heroWithNoBall=&enemyHero1;
	}


		
			
				TC_Position cpos1=myHero1.pos;
            	cpos1.x+=TC_HERO_WIDTH/2;
				cpos1.y+=TC_HERO_HEIGHT/2;
				TC_Position bcpos=ghostBall.pos;
				bcpos.x+=TC_BALL_WIDTH/2;
				bcpos.y+=TC_BALL_HEIGHT/2;
			    Tc_Move(hMyHero1,dataToDiretion(whereIsTheBall(getRelativeLocation(cpos1,bcpos),getDirectionData(heroWithBall->speed))));
				TC_Position cpos2=myHero2.pos;
            	cpos2.x+=TC_HERO_WIDTH/2;
				cpos2.y+=TC_HERO_HEIGHT/2;
			    Tc_Move(hMyHero2,dataToDiretion(whereIsTheBall(getRelativeLocation(cpos2,bcpos),getDirectionData(heroWithBall->speed))));
				
	int length=7;
	if(cpos1.x>bcpos.x-length&&cpos1.x<bcpos.x+length)
	{
		if(cpos1.y<bcpos.y-length)
			Tc_Move(hMyHero1,TC_DIRECTION_BOTTOM);
		if(cpos1.y>bcpos.y+length)
			Tc_Move(hMyHero1,TC_DIRECTION_TOP);
	}
	
	if(cpos1.y>bcpos.y-length&&cpos1.y<bcpos.y+length)
     	{
     		if(cpos1.x<bcpos.x-length)
	    		Tc_Move(hMyHero1,TC_DIRECTION_RIGHT);
	    	if(cpos1.x>bcpos.x+length)
	    		Tc_Move(hMyHero1,TC_DIRECTION_LEFT);
    	}


	if(cpos2.x>bcpos.x-length&&cpos2.x<bcpos.x+length)
	{
		if(cpos2.y<bcpos.y-length)
			Tc_Move(hMyHero2,TC_DIRECTION_BOTTOM);
		if(cpos2.y>bcpos.y+length)
			Tc_Move(hMyHero2,TC_DIRECTION_TOP);
	}
	if(cpos2.y>bcpos.y-length&&cpos2.y<bcpos.y+length)
    	{
	    	if(cpos2.x<bcpos.x-length)
	    		Tc_Move(hMyHero2,TC_DIRECTION_RIGHT);
	    	if(cpos2.x>bcpos.x+length)
	    		Tc_Move(hMyHero2,TC_DIRECTION_LEFT);
    	}

//==============================施法===============================================
		if(myGate.x<enemyGate.x)
		{
			if(ghostBall.pos.x<=forbidenArea.left.right)
			{
			if(Tc_CanBeSpelled(hMyHero2,hHeroWithBall))
				  Tc_Spell(hMyHero2,hHeroWithBall);
			if(enemyHero1.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hMyHero2,hEnemyHero1)&&getCriticality(enemyHero1,enemyLastSpell1)<2)
				 Tc_Spell(hMyHero2,hEnemyHero1);
			if(enemyHero2.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hMyHero2,hEnemyHero2)&&getCriticality(enemyHero2,enemyLastSpell2)<2)
				 Tc_Spell(hMyHero2,hEnemyHero2);
			}
		}
		else
		{
			if(ghostBall.pos.x+64>=forbidenArea.right.left)
			{
				if(Tc_CanBeSpelled(hMyHero2,hHeroWithBall))
				  Tc_Spell(hMyHero2,hHeroWithBall);
				if(enemyHero1.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hMyHero2,hEnemyHero1)&&getCriticality(enemyHero1,enemyLastSpell1)<2)
				 Tc_Spell(hMyHero2,hEnemyHero1);
			if(enemyHero2.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hMyHero2,hEnemyHero2)&&getCriticality(enemyHero2,enemyLastSpell2)<2)
				 Tc_Spell(hMyHero2,hEnemyHero2);
			}
		}

		if(myGate.x<enemyGate.x)
		{
			if(ghostBall.pos.x<=forbidenArea.left.right)
			{
				if(Tc_CanBeSpelled(hMyHero1,hHeroWithBall))
					  Tc_Spell(hMyHero1,hHeroWithBall);
//				if(enemyHero1.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hMyHero1,hEnemyHero1)&&getCriticality(enemyHero1,enemyLastSpell1)<2)
//				 Tc_Spell(hMyHero1,hEnemyHero1);
//			if(enemyHero2.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hMyHero1,hEnemyHero2)&&getCriticality(enemyHero2,enemyLastSpell2)<2)
//				 Tc_Spell(hMyHero1,hEnemyHero2);
			}
		}
		else
		{
			if(ghostBall.pos.x+64>=forbidenArea.right.left)
			{
				if(Tc_CanBeSpelled(hMyHero1,hHeroWithBall))
				  Tc_Spell(hMyHero1,hHeroWithBall);
	//			if(enemyHero1.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hMyHero1,hEnemyHero1)&&getCriticality(enemyHero1,enemyLastSpell1)<2)
	//			 Tc_Spell(hMyHero1,hEnemyHero1);
	//		if(enemyHero2.type==TC_HERO_MALFOY&&Tc_CanBeSpelled(hMyHero1,hEnemyHero2)&&getCriticality(enemyHero2,enemyLastSpell2)<2)
		//		 Tc_Spell(hMyHero1,hEnemyHero2);
			}
		}
				 
	//==================抢球================================
	if(myHero1.b_snatch_ghostball==true)
		Tc_SnatchBall(hMyHero1,TC_GHOST_BALL);
	if(myHero2.b_snatch_ghostball==true)
		Tc_SnatchBall(hMyHero2,TC_GHOST_BALL);
}
void GoldBall()
{
	
	if(goldBall.b_visible==true)
	{
		/*
				TC_Position cpos1=myHero1.pos;
            	cpos1.x+=TC_HERO_WIDTH/2;
				cpos1.y+=TC_HERO_HEIGHT/2;
				TC_Position bcposUp=goldBall.pos;
				bcposUp.x+=TC_BALL_WIDTH/2;
				TC_Position bcposDown=goldBall.pos;
				bcposDown.x+=TC_BALL_WIDTH/2;
				bcposDown.y+=TC_BALL_HEIGHT;
				Tc_Move(hMyHero1,dataToDiretion(whereIsTheBall(getRelativeLocation(cpos1,bcposUp),getDirectionData(goldBall.speed))));
				TC_Position cpos2=myHero2.pos;
            	cpos2.x+=TC_HERO_WIDTH/2;
				cpos2.y+=TC_HERO_HEIGHT/2;
			    Tc_Move(hMyHero2,dataToDiretion(whereIsTheBall(getRelativeLocation(cpos1,bcposDown),getDirectionData(goldBall.speed))));

				*/
		if((enemyHero1.pos.x+TC_HERO_WIDTH/2-goldBall.pos.x-TC_BALL_WIDTH/2)^2+(enemyHero1.pos.y+TC_HERO_HEIGHT/2-goldBall.pos.y-TC_BALL_HEIGHT/2)^2<150^2
			&&Tc_CanBeSpelled(hMyHero1,hEnemyHero1))
			 Tc_Spell(hMyHero1,hEnemyHero1);

		if((enemyHero2.pos.x+TC_HERO_WIDTH/2-goldBall.pos.x-TC_BALL_WIDTH/2)^2+(enemyHero2.pos.y+TC_HERO_HEIGHT/2-goldBall.pos.y-TC_BALL_HEIGHT/2)^2<150^2
			&&Tc_CanBeSpelled(hMyHero1,hEnemyHero2))
			 Tc_Spell(hMyHero1,hEnemyHero2);


		if((enemyHero1.pos.x+TC_HERO_WIDTH/2-goldBall.pos.x-TC_BALL_WIDTH/2)^2+(enemyHero1.pos.y+TC_HERO_HEIGHT/2-goldBall.pos.y-TC_BALL_HEIGHT/2)^2<150^2
			&&Tc_CanBeSpelled(hMyHero2,hEnemyHero1))
			 Tc_Spell(hMyHero2,hEnemyHero1);

		if((enemyHero2.pos.x+TC_HERO_WIDTH/2-goldBall.pos.x-TC_BALL_WIDTH/2)^2+(enemyHero2.pos.y+TC_HERO_HEIGHT/2-goldBall.pos.y-TC_BALL_HEIGHT/2)^2<150^2
			&&Tc_CanBeSpelled(hMyHero2,hEnemyHero2))
			 Tc_Spell(hMyHero2,hEnemyHero2);

				if(myHero1.b_snatch_goldball==true)
					Tc_SnatchBall(hMyHero1,TC_GOLD_BALL);
				if(myHero2.b_snatch_goldball==true)
					Tc_SnatchBall(hMyHero2,TC_GOLD_BALL);

	}
	

}

int getCriticality(TC_Hero hero,int lastSpell)
{
	if(hero.type==TC_HERO_RON)
	{
		if(hero.b_can_spell==true)
			return 0;
		else
			if(hero.curr_blue+50<=hero.spell_cost||lastSpell>50)
				return 2;
			else
				return 1;
	}
	if(hero.type==TC_HERO_HERMIONE)
	{
		if(hero.b_can_spell==true)
			return 1;
		else
			if(hero.curr_blue+50<=hero.spell_cost||lastSpell>50)
				return 3;
			else
				return 2;
	}

	if(hero.type==TC_HERO_MALFOY)
	{
		if(hero.b_can_spell==true)
			return 0;
		else
			if(hero.curr_blue+50<=hero.spell_cost||lastSpell>50)
				return 2;
			else
				return 1;
	}

	if(hero.type==TC_HERO_HARRY)
	{
		if(hero.b_can_spell==true)
			return 2;
		else
			if(hero.curr_blue+50<=hero.spell_cost||lastSpell>50)
				return 3;
			else
				return 2;
	}
 
	if(hero.type==TC_HERO_VOLDEMORT)
	{
		if(hero.b_can_spell==true)
			return 2;
		else
			if(hero.curr_blue+50<=hero.spell_cost||lastSpell>50)
				return 3;
			else
				return 2;
	}
    if(hero.type==TC_HERO_GINNY)
	{
		if(hero.b_can_spell==true)
			return 0;
		else
			if(hero.curr_blue+50<=hero.spell_cost||lastSpell>50)
				return 2;
			else
				return 1;
	}

}