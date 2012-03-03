#pragma once

#include "GameObject.h"
#include "Hero.h"

#define BALL_WIDTH		64

class cBall
	:public cGameObject
{
public:
	cBall(void);
	virtual ~cBall(void);

	virtual	void	Update() = 0;
	virtual	void	Render() = 0;
	virtual bool	IsVisible() = 0;

	int	GetBallType() { return m_type; }

	
protected:
	double GetRandomNum(double iStart, double iEnd);
	double GetLength(sPosition pos1, sPosition pos2);
	virtual void CheckPosition();

	Ball_Type m_type;
};

class cFreeBall :
	public cBall
{
public:
	cFreeBall(void);
	~cFreeBall(void);

	void	Update();
	void	Render();
	void	RePosition();
	bool	CollideWithHero(cHero *p_Hero);

	virtual bool IsVisible() { return m_bIsVisible; }

	sPosition GetStartPos() { return m_posStart; }
	sPosition GetEndPos() { return m_posEnd; }
	unsigned GetShowTime();

protected:
	bool m_bIsVisible;

	sPosition m_posStart;
	sPosition m_posEnd;

	DWORD m_dwHideTime;
	DWORD m_dwShowTime;

	bool m_bKnockedSomeone;

	virtual void CheckPosition();

};


class cGhostBall :
	public cBall
{
public:
	cGhostBall(void);
	~cGhostBall(void);

	void	Update();
	void	Render();
	virtual bool IsVisible(){ return true; }
	bool	IsMoving() { return m_bMoving; }
	void	SetOwner(cHero *pHero);

	sPosition m_posTarget;
	bool m_bStartMove;
	cHero *m_pOwner;
	cHero *m_pPassballHero;


protected:

	bool m_bMoving;
	DWORD m_dwSteps;
	
};

class cGoldBall :
	public cBall
{
public:
	cGoldBall(void);
	~cGoldBall(void);

	void	Update();
	void	Render();
	virtual bool IsVisible() { return m_bVisible; }
	DWORD	GetSlowDownSteps() { return m_SlowDownSteps; }

	cHero *m_pOwner;		// the game is over if it's true
	bool m_bVisible;

private:

	//////////////////////////////////////////////////////////////////////////
	void setNextSpeed();
	double dist(sPosition goldball_pos, sPosition hero_pos);
	void move();

	void RandPosition();
	void getHerosInfo();
	bool CanBeHere();
	//////////////////////////////////////////////////////////////////////////

	int m_ShowTime;
	int m_SlowDownSteps;
	double m_vSpeed;
	sPosition herosPos[4];
	sPosition herosNextPos[4];

	sSpeed m_preSpeed;
	sSpeed herosSpeed[4];

	double MIN_DIST_TO_GOLDBALL;
	double CRITICAL_NORMAL_MOVING_DIST;
	double CRITICAL_SLOWDOWN_MOVING_DIST;

	void Reposition();

};