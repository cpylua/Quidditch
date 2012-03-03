#include "cBall.h"
#include "Game.h"
#include "HeroState.h"

#include <cmath>
#include <algorithm>
using namespace std;

extern cHero* g_HeroTeam1[AI_NUM];
extern cHero* g_HeroTeam2[AI_NUM];
extern cHero* g_Heros[AI_NUM * 2];
extern Gate g_Gate[AI_NUM];
extern DWORD g_GameTime;
extern DWORD g_TurnStartTime;

// freeball
int FREE_BALL_SHOW_TIME_MIN = 100;
int FREE_BALL_SHOW_TIME_MAX = 150;

int FREE_BALL_HIDE_TIME_MIN = 20;
int FREE_BALL_HIDE_TIME_MAX = 30;

int FREE_BALL_SPEED = 12;

// ghostball
int GHOST_BALL_SPEED = 12;

// goldball
int GOLD_BALL_SPEED = 12;
int GOLD_BALL_SHOW_TIME = 10000;
int GOLD_BALL_SLOWDOWN_STEPS = 150;


//////////////////////////////////////////////////////////////////////////
/////////////////////////////////Ball/////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
cBall::cBall(void)
{
	m_currWidth = BALL_WIDTH;
	m_currHeight = BALL_WIDTH;
}

cBall::~cBall(void)
{
}

double cBall::GetRandomNum( double iStart, double iEnd )
{
	unsigned int i;
	rand_s(&i);
	return ( (double)i / UINT_MAX  * (iEnd - iStart) + iStart );
}

double cBall::GetLength( sPosition pos1, sPosition pos2 )
{
	return sqrt( pow((double)pos1.x - pos2.x, 2) + pow((double)pos1.y - pos2.y, 2) );
}

void cBall::CheckPosition()
{
	int tmp = m_position.x;
	m_position.x = (m_position.x < 0) ? 0 : m_position.x;
	m_position.x = (m_position.x > 2048 - BALL_WIDTH) ? 2048 - BALL_WIDTH : m_position.x;
	if( tmp != m_position.x )
	{
		m_speed.vx = -m_speed.vx;
		m_speed.vy = -m_speed.vy;
	}
	tmp = m_position.y;
	m_position.y = (m_position.y < 0) ? 0 : m_position.y;
	m_position.y = (m_position.y > 768 - BALL_WIDTH) ? 768 - BALL_WIDTH : m_position.y;
	if( tmp != m_position.y )
	{
		m_speed.vx = -m_speed.vx;
		m_speed.vy = -m_speed.vy;
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
///////////////////////////////FreeBall///////////////////////////////////
//////////////////////////////////////////////////////////////////////////

cFreeBall::cFreeBall( void )
{
	m_currFrame = 0;

	m_dwShowTime = (DWORD)GetRandomNum(FREE_BALL_SHOW_TIME_MIN, FREE_BALL_SHOW_TIME_MAX);
	m_dwHideTime = 0;

	m_bIsVisible = true;
	m_bKnockedSomeone =  false;

	m_type = BALL_FREE;

	RePosition();
}

cFreeBall::~cFreeBall( void )
{

}

void cFreeBall::Update()
{
	// show time has gone!
	if( m_dwShowTime == 0 )
	{
		m_bIsVisible = false;

		if( m_dwHideTime > 0 )
		{
			// just hide the ball,
			// do nothing else
			m_dwHideTime--;
		}
		else
		{
			// show time arrived!
			m_dwShowTime = (DWORD)GetRandomNum(FREE_BALL_SHOW_TIME_MIN, FREE_BALL_SHOW_TIME_MAX);
			m_bIsVisible = true;
			m_currFrame = 0;

			RePosition();

			m_dwHideTime = (DWORD)GetRandomNum(FREE_BALL_HIDE_TIME_MIN, FREE_BALL_HIDE_TIME_MAX);
		}
		return;
	}
	else	// show time arrived!
	{
		//
		// deal with show time
		//
		m_dwShowTime--;

		// no one has been knocked
		m_bKnockedSomeone = false;

		// move to the next frame
		m_currFrame++;
		m_currFrame %= 4;

		// update ball position
		move();

		//
		// we need to move back and forth
		//
		int iMax, iMin;
		iMax = (m_posStart.x > m_posEnd.x) ? m_posStart.x : m_posEnd.x;
		iMin = (iMax == m_posEnd.x) ? m_posStart.x : m_posEnd.x;
		if( m_position.x > iMax )
		{
			// move back
			m_speed.vx = -m_speed.vx;
			m_speed.vy = -m_speed.vy;
		}
		else if( m_position.x < iMin )
		{
			// move back
			m_speed.vx = -m_speed.vx;
			m_speed.vy = -m_speed.vy;
		}

		//
		// deal with collides
		//
		for(int i = 0; i < _countof(g_Heros); i++)
		{
			if( g_Heros[i]->CanBeSpelled() &&
				CollideWithHero(g_Heros[i]) )
			{
				g_Heros[i]->GetStateMachine()->ChangeState(cSpelledByFreeballState::Instance());
				m_bKnockedSomeone = true;
			}
		}

		//
		// reposition the ball if it knocked someone
		//
		if( m_bKnockedSomeone )
		{
			m_currFrame = 0;

			RePosition();
		}
	} /* end if */
}

void cFreeBall::Render()
{
	cTexture *pTex = cGame::GetResourceManager()->GetElementTexture(MAGICS);
	int iTexX = m_currFrame * BALL_WIDTH;
	int iTexY = 4 * HERO_WIDTH;

	sPosition pos = getScreenPosition();

	if( m_bIsVisible )
		pTex->draw(pos.x, pos.y, iTexX, iTexY, BALL_WIDTH, BALL_WIDTH);
}

void cFreeBall::RePosition()
{
	int iRoad = cGame::GetResourceManager()->getMap()->GetRoad();

	// start position
	m_posStart.x = (int)GetRandomNum(iRoad + 340, iRoad + GAME_WINDOW_WIDTH - 340);
	m_posStart.y = (int)GetRandomNum(256, GAME_WINDOW_HEIGHT - 256);

	// end position
	double tmp[2];
	unsigned int rnd;
	rand_s(&rnd);
	if( rnd % 201 > 100 )
	{
		tmp[0] = GetRandomNum(iRoad + 64, iRoad + 128 + 64);
		tmp[1] = GetRandomNum(iRoad + GAME_WINDOW_WIDTH - 64 - 128, iRoad + GAME_WINDOW_WIDTH - 64);

		rand_s(&rnd);
		if( rnd % 201 > 100 )
			m_posEnd.x = (int)tmp[0];
		else
			m_posEnd.x = (int)tmp[1];

		m_posEnd.y = (int)GetRandomNum(64, GAME_WINDOW_HEIGHT - 64);
	}
	else
	{
		tmp[0] = GetRandomNum(64, 128+64);
		tmp[1] = GetRandomNum(GAME_WINDOW_HEIGHT - 64 - 128, GAME_WINDOW_HEIGHT - 64);

		rand_s(&rnd);
		if( rnd % 201 > 100 )
			m_posEnd.y = (int)tmp[0];
		else
			m_posEnd.y = (int)tmp[1];

		m_posEnd.x = (int)GetRandomNum(iRoad + 64 + 128, iRoad + GAME_WINDOW_WIDTH - 64 - 128);
	}

	// update speed
	double len = GetLength(m_posStart, m_posEnd);
	m_speed.vx = (int)(FREE_BALL_SPEED * (m_posEnd.x - m_posStart.x) / len);
	m_speed.vy = (int)(FREE_BALL_SPEED * (m_posEnd.y - m_posStart.y) / len);

	// move to start position
	m_position = m_posStart;
}

void cFreeBall::CheckPosition()
{

}

bool cFreeBall::CollideWithHero( cHero *p_Hero )
{
	const int delta = BALL_WIDTH;

	sPosition posHero;
	p_Hero->GetPos(posHero);
	sPosition posBall = m_position;

	posBall.x += BALL_WIDTH / 2;
	posBall.y += BALL_WIDTH / 2;
	posHero.x += HERO_WIDTH / 2;
	posHero.y += HERO_WIDTH / 2;

	double len = GetLength(posBall, posHero);

	return ( len <=  delta );
}

unsigned cFreeBall::GetShowTime()
{
	if( m_dwHideTime > 0 )
		return 0;
	else
		return m_dwShowTime;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
///////////////////////////////GhostBall//////////////////////////////////
//////////////////////////////////////////////////////////////////////////


cGhostBall::cGhostBall( void )
{
	m_pOwner = NULL;
	m_pPassballHero = NULL;
	m_posTarget.x = m_posTarget.y = 0;
	m_bMoving = false;
	m_bStartMove = false;
	m_speed.vx = m_speed.vy = 0;
	m_dwSteps = 0;

	m_type = BALL_GHOST;

	int road = cGame::GetResourceManager()->getMap()->GetRoad();

	// 中线某一点
	m_position.x = road + GAME_WINDOW_WIDTH / 2 - BALL_WIDTH / 2;
	m_position.y = (int)GetRandomNum(GAME_WINDOW_HEIGHT / 5, GAME_WINDOW_HEIGHT * 7 / 10);
}

cGhostBall::~cGhostBall( void )
{

}

void cGhostBall::Update()
{
	//
	// spin
	//
	m_currFrame = m_dwSteps % 3;
	m_dwSteps++;

	if( m_pOwner != NULL )
		return;

	if( m_bStartMove )
	{
		double len = GetLength(m_position, m_posTarget);
		m_speed.vx = (int)(GHOST_BALL_SPEED * (m_posTarget.x - m_position.x) / len);
		m_speed.vy = (int)(GHOST_BALL_SPEED * (m_posTarget.y - m_position.y) / len);

		m_bStartMove = false;
		m_bMoving = true;
		m_dwSteps = 0;
	}
	if( m_bMoving )
	{
		move();

		if( (m_speed.vx <= 0 && m_position.x <= m_posTarget.x ||
			m_speed.vx > 0 && m_position.x >= m_posTarget.x)/* &&
			(m_speed.vy <= 0 && m_position.y <= m_posTarget.y ||
			m_speed.vy > 0 && m_position.y >= m_posTarget.y)*/ )
		{
				//m_bMoving = false;
				m_position.x = m_posTarget.x;
		}

		if( m_speed.vy <= 0 && m_position.y <= m_posTarget.y ||
			m_speed.vy > 0 && m_position.y > m_posTarget.y )
		{
			m_position.y = m_posTarget.y;
		}

		if( m_position.x == m_posTarget.x && 
			m_position.y == m_posTarget.y )
		{
			//
			// we arrived, stop moving
			//
			m_speed.vx = m_speed.vy = 0;
			m_bMoving = false;
		}
	}
}

void cGhostBall::Render()
{
	//
	// draw the ball if the ball has no owner
	//
	if( m_pOwner == NULL )
	{
		cTexture *pTex = cGame::GetResourceManager()->GetElementTexture(MAGICS);
		int iTexX = m_currFrame * BALL_WIDTH;
		int iTexY = 4 * HERO_WIDTH + BALL_WIDTH;

		sPosition pos = getScreenPosition();

		pTex->draw(pos.x, pos.y, iTexX, iTexY, BALL_WIDTH, BALL_WIDTH);
	}
}

void cGhostBall::SetOwner( cHero *pHero )
{
	m_pOwner = pHero;
	m_bStartMove = false;
	m_bMoving = false;
	m_posTarget.x = m_posTarget.y = 0;

	m_speed.vx = m_speed.vy = 0;
}


//////////////////////////////////////////////////////////////////////////
////////////////////////////////GoldBall//////////////////////////////////
//////////////////////////////////////////////////////////////////////////

cGoldBall::cGoldBall( void )
{
	m_currFrame = 0;
	m_pOwner = NULL;

	m_type = BALL_GOLD;

	m_bVisible = false;

	m_currWidth = HERO_WIDTH;
	m_currHeight = HERO_WIDTH;

	//////////////////////////////////////////////////////////////////////////

	m_position.x = 1024;
	m_position.y = 320;

	MIN_DIST_TO_GOLDBALL = 300;
	CRITICAL_NORMAL_MOVING_DIST = 74;	// 64 + \Delta(10)
	CRITICAL_SLOWDOWN_MOVING_DIST = 174; // 64 + slowdown_time(100) + \Delta(10)

	m_preSpeed = m_speed;
	m_ShowTime = GOLD_BALL_SHOW_TIME;

	m_vSpeed = GOLD_BALL_SPEED;

	m_SlowDownSteps = 0;
}

cGoldBall::~cGoldBall( void )
{

}

double cGoldBall::dist(sPosition goldball_pos, sPosition hero_pos)
{
	sPosition pos1, pos2;
	pos1.x = goldball_pos.x+BALL_WIDTH;
	pos1.y = goldball_pos.y+BALL_WIDTH;
	pos2.x = hero_pos.x+HERO_WIDTH/2;
	pos2.y = hero_pos.y+HERO_WIDTH/2;
	return sqrt((pos1.x-pos2.x)*(pos1.x-pos2.x)*1.0+(pos1.y-pos2.y)*(pos1.y-pos2.y));
}

void cGoldBall::setNextSpeed()
{
	double dist_to_goldball[4];
	double CRITICAL_MOVING_DIST;
	double t[4];
	int i, j;
	bool DIR_DETERMINATE_FLAG;
	bool SLOWDOWN_FLAG;

	struct mySpeed
	{
		double distance;
		int vx, vy;
		bool operator<(mySpeed sp)
		{
			return this->distance<sp.distance;
		}
	}myspeed[360];

	int myspeedNum;

	int start_degree = 0;

	getHerosInfo();

	for(i=0; i<4; i++)
	{
		dist_to_goldball[i] = dist(m_position, herosPos[i]);
	}

	// sort the dist ascending
	sort(dist_to_goldball, dist_to_goldball+4);

	m_speed.vx = m_speed.vy = 0;

	m_vSpeed = GOLD_BALL_SPEED;
	if(m_SlowDownSteps>0)
	{
		m_vSpeed /= 2;
	}
	if(dist_to_goldball[0]<MIN_DIST_TO_GOLDBALL)
	{
		DIR_DETERMINATE_FLAG = false;

		myspeedNum = 0;
		for(i=start_degree; i<start_degree+360; i++)
		{
			int vx = int(m_vSpeed*cos(i*asin(1.0)/90));
			int vy = int(m_vSpeed*sin(i*asin(1.0)/90));
			sPosition tmpGoldNextPos;
			tmpGoldNextPos.x = m_position.x+vx;
			tmpGoldNextPos.y = m_position.y+vy;

			SLOWDOWN_FLAG = false;
			if(m_SlowDownSteps>0)
			{
				SLOWDOWN_FLAG = true;
			}

			if(tmpGoldNextPos.x+HERO_WIDTH>g_Gate[1].x)
			{
				tmpGoldNextPos.x = g_Gate[0].x;
				SLOWDOWN_FLAG = true;
			}
			else if(tmpGoldNextPos.x<g_Gate[0].x)
			{
				tmpGoldNextPos.x = g_Gate[1].x-HERO_WIDTH;
				SLOWDOWN_FLAG = true;
			}

			if(tmpGoldNextPos.y<0)
			{
				tmpGoldNextPos.y = 768-HERO_WIDTH;
				SLOWDOWN_FLAG = true;
			}
			else if(tmpGoldNextPos.y+HERO_WIDTH>768)
			{
				tmpGoldNextPos.y = 0;
				SLOWDOWN_FLAG = true;
			}

			CRITICAL_MOVING_DIST = CRITICAL_NORMAL_MOVING_DIST;
			if (SLOWDOWN_FLAG)
			{
				CRITICAL_MOVING_DIST = CRITICAL_SLOWDOWN_MOVING_DIST;
			}


			for(j=0; j<4; j++)
			{
				t[j] = dist(tmpGoldNextPos, herosNextPos[j]);
			}

			if(t[0]>CRITICAL_MOVING_DIST && t[1]>CRITICAL_MOVING_DIST
				&& t[2]>CRITICAL_MOVING_DIST && t[3]>CRITICAL_MOVING_DIST)
			{
				sort(t, t+4);
				myspeed[myspeedNum].distance = t[0];
				myspeed[myspeedNum].vx = vx;
				myspeed[myspeedNum++].vy = vy;
				DIR_DETERMINATE_FLAG = true;
			}
		}

		if(DIR_DETERMINATE_FLAG)
		{
			sort(myspeed, myspeed+myspeedNum);
			m_speed.vx = myspeed[myspeedNum-1].vx;
			m_speed.vy = myspeed[myspeedNum-1].vy;
		}
		if(!DIR_DETERMINATE_FLAG)
		{
			myspeedNum = 0;
			for(i=start_degree; i<start_degree+360; i++)
			{
				int vx = int(m_vSpeed*cos(i*asin(1.0)/90));
				int vy = int(m_vSpeed*sin(i*asin(1.0)/90));
				sPosition tmpGoldNextPos;
				tmpGoldNextPos.x = m_position.x+vx;
				tmpGoldNextPos.y = m_position.y+vy;

				SLOWDOWN_FLAG = false;
				if(m_SlowDownSteps>0)
				{
					SLOWDOWN_FLAG = true;
				}

				if(tmpGoldNextPos.x+HERO_WIDTH>g_Gate[1].x)
				{
					tmpGoldNextPos.x = g_Gate[0].x;
					SLOWDOWN_FLAG = true;
				}
				else if(tmpGoldNextPos.x<g_Gate[0].x)
				{
					tmpGoldNextPos.x = g_Gate[1].x-HERO_WIDTH;
					SLOWDOWN_FLAG = true;
				}

				if(tmpGoldNextPos.y<0)
				{
					tmpGoldNextPos.y = 768-HERO_WIDTH;
					SLOWDOWN_FLAG = true;
				}
				else if(tmpGoldNextPos.y+HERO_WIDTH>768)
				{
					tmpGoldNextPos.y = 0;
					SLOWDOWN_FLAG = true;
				}

				CRITICAL_MOVING_DIST = CRITICAL_NORMAL_MOVING_DIST;
				if (SLOWDOWN_FLAG)
				{
					CRITICAL_MOVING_DIST = CRITICAL_SLOWDOWN_MOVING_DIST;
				}

				for(j=0; j<4; j++)
				{
					t[j] = dist(tmpGoldNextPos, herosNextPos[j]);
				}

				if( (t[0]>CRITICAL_MOVING_DIST || t[0]>dist(m_position, herosNextPos[0]))
					&& (t[1]>CRITICAL_MOVING_DIST|| t[1]>dist(m_position, herosNextPos[1]))
					&& (t[2]>CRITICAL_MOVING_DIST|| t[2]>dist(m_position, herosNextPos[2])) 
					&& (t[3]>CRITICAL_MOVING_DIST|| t[3]>dist(m_position, herosNextPos[3])) )
				{
					sort(t, t+4);
					myspeed[myspeedNum].distance = t[0];
					myspeed[myspeedNum].vx = vx;
					myspeed[myspeedNum++].vy = vy;
				}
			}
			sort(myspeed, myspeed+myspeedNum);
			m_speed.vx = myspeed[myspeedNum-1].vx;
			m_speed.vy = myspeed[myspeedNum-1].vy;
		}
	}
}

void cGoldBall::move()
{
	m_position.x += m_speed.vx;
	m_position.y += m_speed.vy;
}

void cGoldBall::Update()
{
	if(!m_bVisible && (g_GameTime + GetTickCount()- g_TurnStartTime) > (DWORD)m_ShowTime)
	{
		m_bVisible = true;
		if(!CanBeHere())
		{
			RandPosition();		
		}
	}

	if(m_bVisible)
	{
		m_SlowDownSteps--;

		m_currFrame++;
		m_currFrame %= 8;

		setNextSpeed();
		move();

		Reposition();
	}
}

void cGoldBall::Render()
{
	if( m_pOwner == NULL && m_bVisible )
	{
		cTexture *pTex = cGame::GetResourceManager()->GetElementTexture(MAGICS);
		int iTexX = m_currFrame * HERO_WIDTH;
		int iTexY = 5 * HERO_WIDTH;

		sPosition pos = getScreenPosition();

		pTex->draw(pos.x, pos.y, iTexX, iTexY, HERO_WIDTH, HERO_WIDTH);
	}
}

void cGoldBall::Reposition()
{
	if(m_position.x<g_Gate[0].x)
	{
		m_position.x = g_Gate[1].x-HERO_WIDTH;
		m_SlowDownSteps = GOLD_BALL_SLOWDOWN_STEPS;
	}
	if(m_position.x+HERO_WIDTH>g_Gate[1].x)
	{
		m_position.x = g_Gate[0].x;
		m_SlowDownSteps = GOLD_BALL_SLOWDOWN_STEPS;
	}

	if(m_position.y+HERO_WIDTH>768)
	{
		m_position.y = 0;
		m_SlowDownSteps = GOLD_BALL_SLOWDOWN_STEPS;
	}
	if(m_position.y<0)
	{
		m_position.y = 768-HERO_WIDTH;
		m_SlowDownSteps = GOLD_BALL_SLOWDOWN_STEPS;
	}
}

void cGoldBall::RandPosition()
{
	int i;
	int rightx=-1, leftx=3000, upy=800, downy=-1;
	m_position.x = 200;
	m_position.y = 0;

	getHerosInfo();
	for(i=0; i<4; i++)
	{
		rightx = rightx>herosPos[i].x?rightx:herosPos[i].x;
		leftx = leftx<herosPos[i].x?leftx:herosPos[i].x;
		upy = upy<herosPos[i].y?upy:herosPos[i].y;
		downy = downy>herosPos[i].y?downy:herosPos[i].y;
	}

	if(rightx<1300)
	{
		m_position.x = rightx+410;
		m_position.y = (int)GetRandomNum(0, 768-HERO_WIDTH);
	}

	if(leftx>410)
	{
		m_position.x = leftx-410;
		m_position.y = (int)GetRandomNum(0, 768-HERO_WIDTH);
	}

	if(upy>256)
	{
		m_position.x = (int)GetRandomNum(200, 1848-HERO_WIDTH);
		m_position.y = upy-256;
	}

	if(downy<500)
	{
		m_position.x = (int)GetRandomNum(200, 1848-HERO_WIDTH);
		m_position.y = downy+256;
	}
}

void cGoldBall::getHerosInfo()
{
	for(int i=0; i<4; i++)
	{	
		herosPos[i] = g_Heros[i]->getPosition();
		herosSpeed[i] = g_Heros[i]->GetSpeed();
		herosNextPos[i].x = herosPos[i].x+herosSpeed[i].vx;
		herosNextPos[i].y = herosPos[i].y+herosSpeed[i].vy;
	}
}

bool cGoldBall::CanBeHere()
{
	getHerosInfo();
	return dist(m_position, herosPos[0])>MIN_DIST_TO_GOLDBALL &&
		dist(m_position, herosPos[1])>MIN_DIST_TO_GOLDBALL &&
		dist(m_position, herosPos[2])>MIN_DIST_TO_GOLDBALL &&
		dist(m_position, herosPos[3])>MIN_DIST_TO_GOLDBALL;
}