#include "Hero.h"
#include "Game.h"
#include "HeroState.h"
#include "ResourceManager.h"
#include "TuringCup9ClientAPI.h"
#include "cBall.h"

#include <strsafe.h>

extern cBall* g_Balls[BALL_NUM];
extern unsigned g_TeamScore[AI_NUM];		// store team scores
extern TC_Rect g_ForbiddenAreas[AI_NUM];

/*
	Do NOT forget

	update TC_SNATCH_DISTANCE_GHOST 

*/
#define SNATCH_DISTANCE_GHOST	64
#define SNATCH_DISTANCE_GOLD	64
#define GOLD_BALL_SCORE			10

#define MANA_RECHARGE	1

#pragma warning(push)
#pragma warning(disable:4355)
cHero::cHero( const sPosition& p_pos )
:m_pStateMachine(this)
{
	m_position		= p_pos;
	m_spellObject	= NULL;
	m_pBall			= NULL;
	m_SnatchInterval = 0;
	m_currWidth = HERO_WIDTH;
	m_currHeight = HERO_WIDTH;
	m_bGoldBall = false;
	m_SpellInterval = 0;

	m_speed.vx = HERO_SPEED;
	m_speed.vy = HERO_SPEED;

	m_prevAction = -1;
	m_currAction = ACTION_HERO_STOP;

	m_AbnormalType = SPELLED_BY_NONE;
	m_pStateMachine.SetCurrentState(cNormalState::Instance());
}
#pragma warning(pop)

bool cHero::CanBeSpelled()
{
	return ( GetStateMachine()->IsInState(cNormalState::Instance()) );

}

void cHero::Update()
{
	//
	// update blue
	//
	if( m_currBlue < m_maxBlue )
		m_currBlue++;
	m_currBlue %= m_maxBlue + MANA_RECHARGE;

	//
	// update spell interval
	//
	if( m_SpellInterval > 0 )
		m_SpellInterval--;

	//
	// Update snatch interval
	//
	if( m_SnatchInterval > 0 )
		m_SnatchInterval--;


	//
	// update status
	//
	m_pStateMachine.Update();

	//
	// update ball position if having a ball
	//
	if( m_pBall != NULL && !m_bGoldBall)
		m_pBall->SetPos(m_position.x + HERO_WIDTH / 4, m_position.y + HERO_WIDTH / 4);
}

void cHero::Render()
{
	m_pStateMachine.Render();
}

cTexture* cHero::GetTexture()
{
	return cGame::GetResourceManager()->GetHeroTexture((Hero_Type)m_objectType);
}

int cHero::GetTextureX()
{
	if( getAction() == -1 ||
		getAction() == ACTION_HERO_STOP )
	{
		switch(m_prevAction)
		{
		case ACTION_HERO_BOTTOM:
			return 0;
		case ACTION_HERO_LEFTBOTTOM:
			return 1 * HERO_WIDTH;
		case ACTION_HERO_LEFT:
			return 2 * HERO_WIDTH;
		case ACTION_HERO_LEFTTOP:
			return 3 * HERO_WIDTH;
		case ACTION_HERO_TOP:
			return 4 * HERO_WIDTH;
		case ACTION_HERO_RIGHTTOP:
			return 5 * HERO_WIDTH;
		case ACTION_HERO_RIGHT:
			return 6 * HERO_WIDTH;
		case ACTION_HERO_RIGHTBOTTOM:
			return 7 * HERO_WIDTH;
		case ACTION_HERO_SPELLING:
			return 0;
		default:
			return 0;
		}
	}

	switch( getAction() )
	{
	case ACTION_HERO_BOTTOM:
		return 0;
	case ACTION_HERO_LEFTBOTTOM:
		return 1 * HERO_WIDTH;
	case ACTION_HERO_LEFT:
		return 2 * HERO_WIDTH;
	case ACTION_HERO_LEFTTOP:
		return 3 * HERO_WIDTH;
	case ACTION_HERO_TOP:
		return 4 * HERO_WIDTH;
	case ACTION_HERO_RIGHTTOP:
		return 5 * HERO_WIDTH;
	case ACTION_HERO_RIGHT:
		return 6 * HERO_WIDTH;
	case ACTION_HERO_RIGHTBOTTOM:
		return 7 * HERO_WIDTH;
	case ACTION_HERO_SPELLING:
		/*return m_currFrame * HERO_WIDTH;*/
		return 0;
	default:
		return 0;
	}	
}

int cHero::GetTextureY()
{
	int iRet = 0;

	if( GetStateMachine()->IsInState(cSpelledByVoldemortState::Instance()) )
	{
		if( m_pBall != NULL && !m_bGoldBall)
			iRet = 4;
		else
			iRet = 3;
	}
	else if( GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) )
	{
		if( m_pBall != NULL && !m_bGoldBall)
			iRet = 2;
		else
			iRet = 2;
	}
	else
	{
		if(	m_pBall != NULL && !m_bGoldBall)
			iRet = 1;
		else
			iRet = 0;
	}

	return iRet * HERO_WIDTH;;
}



bool cHero::SnatchBall(int p_Type)
{
	//
	// NOTE
	// GetBall will slow down hero's speed
	// and LoseBall will increase hero's speed
	//

	bool bRet = false;

	if( m_SnatchInterval > 0 )
		return bRet;

	if( m_pBall != NULL && 
		( (m_bGoldBall && p_Type == BALL_GHOST) || 
		  (!m_bGoldBall && p_Type == BALL_GOLD) ) )
	{
		return bRet;
	}

	if( p_Type == BALL_GHOST )
	{
		if( CanSnatchGhostBall() )
		{
			cHero *pOwner = ((cGhostBall*)g_Balls[1])->m_pOwner;
			if( pOwner != NULL && pOwner->GetStateMachine()->IsInState(cSpellingState::Instance()) )
				return bRet;

			//
			// the ball has no owner
			//
			if( pOwner == NULL )
			{
				((cGhostBall*)g_Balls[1])->SetOwner(this);
				g_Balls[1]->SetPos(m_position.x + HERO_WIDTH / 4, m_position.y + HERO_WIDTH / 4);
				GetBall(g_Balls[1], false);

				bRet = true;
			}
			else	// the ball already has an owner
			{
				bool bSpelled = 
					pOwner->GetStateMachine()->IsInState(cSpelledByMalfoyState::Instance()) ||
					pOwner->GetStateMachine()->IsInState(cSpelledByFreeballState::Instance());

				unsigned int rnd;
				rand_s(&rnd);
				rnd %= 201;
				if( rnd < 100 || bSpelled )
				{
					//
					// get the ball
					//
					cHero *pHero = ((cGhostBall*)g_Balls[1])->m_pOwner;
					pHero->LoseBall();
					GetBall(g_Balls[1], false);

					((cGhostBall*)g_Balls[1])->SetOwner(this);
					g_Balls[1]->SetPos(m_position.x + HERO_WIDTH / 4, m_position.y + HERO_WIDTH / 4);		

					bRet = true;
				}
			}
		}  /*  in range */
	}	/* Ghost ball */
	else if( p_Type == BALL_GOLD )
	{
		//
		// invisible, can't snatch it
		//
		if( !((cGoldBall*)g_Balls[2])->m_bVisible )
			return bRet;

		//
		// are u feeling lucky?
		//
		if( CanSnatchGoldBall() )
		{
			cHero *pOwner = ((cGoldBall*)g_Balls[2])->m_pOwner;
			if( pOwner != NULL )
				return bRet;

			unsigned int rnd;
			rand_s(&rnd);
			rnd %= 201;
			if( rnd > 0 )
			{
				//
				// get the ball
				//
				GetBall(g_Balls[2], true);
				int index = cGame::FindHeroTeam(this);
				g_TeamScore[index] += GOLD_BALL_SCORE;
				
				//
				// no one else can get the ball in the future,
				// the game is over
				//
				((cGoldBall*)g_Balls[2])->m_pOwner= this;

				bRet = true;
			}
		} /* can snatch gold ball */

	}	/* p_Type == BALL_GOLD */

	return bRet;
}

double cHero::GetLength( sPosition pos1, sPosition pos2 )
{
	return sqrt( pow((double)pos1.x - pos2.x, 2) + pow((double)pos1.y - pos2.y, 2) );
}

void cHero::LoseBall()
{
	m_pBall = NULL;

	//
	// increase hero's speed
	//

	int spd = (int)ceil(HERO_SPEED / sqrt(2.0));
	int spd_straight = HERO_SPEED;
	if( GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) )
	{
		spd /= 2;
		spd_straight /= 2;
	}

	int action = (m_currAction == -1) ? m_prevAction : m_currAction;
	switch(action)
	{
	case ACTION_HERO_BOTTOM:
		m_speed.vx = 0;
		m_speed.vy = spd_straight;
		break;
	case ACTION_HERO_LEFTBOTTOM:
		m_speed.vx = -spd;
		m_speed.vy = spd;
		break;
	case ACTION_HERO_LEFT:
		m_speed.vx = -spd_straight;
		m_speed.vy = 0;
		break;
	case ACTION_HERO_LEFTTOP:
		m_speed.vx = -spd;
		m_speed.vy = -spd;
		break;
	case ACTION_HERO_TOP:
		m_speed.vx = 0;
		m_speed.vy = -spd_straight;
		break;
	case ACTION_HERO_RIGHTTOP:
		m_speed.vx = spd;
		m_speed.vy = -spd;
		break;
	case ACTION_HERO_RIGHT:
		m_speed.vx = spd_straight;
		m_speed.vy = 0;
		break;
	case ACTION_HERO_RIGHTBOTTOM:
		m_speed.vx = spd;
		m_speed.vy = spd;
		break;
	case ACTION_HERO_STOP:
	case ACTION_HERO_SPELLING:
		m_speed.vx = 0;
		m_speed.vy = 0;
		break;
	} /* end switch */
}

void cHero::GetBall( cBall *p_pBall, bool p_bGoldBall )
{
	m_pBall = p_pBall;
	m_bGoldBall = p_bGoldBall;

	//
	// slow down the hero's speed
	//
	int spd = (int)(HERO_SPEED_SLOW / sqrt(2.0));
	int spd_straight = HERO_SPEED_SLOW;
	if( GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) )
	{
		spd /= 2;
		spd_straight /= 2;
	}

	int action = (m_currAction == -1) ? m_prevAction : m_currAction;
	switch(action)
	{
	case ACTION_HERO_BOTTOM:
		m_speed.vx = 0;
		m_speed.vy = spd_straight;
		break;
	case ACTION_HERO_LEFTBOTTOM:
		m_speed.vx = -spd;
		m_speed.vy = spd;
		break;
	case ACTION_HERO_LEFT:
		m_speed.vx = -spd_straight;
		m_speed.vy = 0;
		break;
	case ACTION_HERO_LEFTTOP:
		m_speed.vx = -spd;
		m_speed.vy = -spd;
		break;
	case ACTION_HERO_TOP:
		m_speed.vx = 0;
		m_speed.vy = -spd_straight;
		break;
	case ACTION_HERO_RIGHTTOP:
		m_speed.vx = spd;
		m_speed.vy = -spd;
		break;
	case ACTION_HERO_RIGHT:
		m_speed.vx = spd_straight;
		m_speed.vy = 0;
		break;
	case ACTION_HERO_RIGHTBOTTOM:
		m_speed.vx = spd;
		m_speed.vy = spd;
		break;
	case ACTION_HERO_STOP:
	case ACTION_HERO_SPELLING:
		m_speed.vx = 0;
		m_speed.vy = 0;
		break;
	} /* end switch */
}

void cHero::PassBall( sPosition &p_pPos )
{
	cGhostBall *pGhostBall = ((cGhostBall*)g_Balls[1]);

	pGhostBall->m_posTarget.x = p_pPos.x;
	pGhostBall->m_posTarget.y = p_pPos.y;
	pGhostBall->m_bStartMove = true;
	pGhostBall->m_pOwner = NULL;
	pGhostBall->m_pPassballHero = this;

	LoseBall();
}

void cHero::CheckPosition()
{
	sPosition PrevPos;
	PrevPos.x = m_position.x - m_speed.vx;
	PrevPos.y = m_position.y - m_speed.vy;

	m_position.y = (m_position.y < 0) ? 0 : m_position.y;
	m_position.y = (m_position.y > GAME_WINDOW_HEIGHT - HERO_WIDTH) ? 
		GAME_WINDOW_HEIGHT - HERO_WIDTH : m_position.y;

	// left edge
	if( PrevPos.x >= g_Gate[0].x && PrevPos.y <= g_Gate[0].y_upper )
	{
		m_position.x = (m_position.x < g_Gate[0].x) ? g_Gate[0].x : m_position.x;
	}
	if( PrevPos.x >= g_Gate[0].x && PrevPos.y >= g_Gate[0].y_lower - HERO_WIDTH )
	{
		m_position.x = (m_position.x < g_Gate[0].x) ? g_Gate[0].x : m_position.x;
	}

	if( PrevPos.x < g_Gate[0].x )
	{
		m_position.x = (m_position.x < 0) ? 0 : m_position.x;

		m_position.y = (m_position.y < g_Gate[0].y_upper) ?
			g_Gate[0].y_upper : m_position.y;
		m_position.y = (m_position.y > g_Gate[0].y_lower - HERO_WIDTH) ?
			g_Gate[0].y_lower - HERO_WIDTH : m_position.y;
	}

	// right edge
	if( PrevPos.x <= g_Gate[1].x - HERO_WIDTH && PrevPos.y <= g_Gate[1].y_upper )
	{
		m_position.x = (m_position.x > g_Gate[1].x - HERO_WIDTH) ?
			g_Gate[1].x - HERO_WIDTH : m_position.x;
	}
	if( PrevPos.x <= g_Gate[1].x - HERO_WIDTH && PrevPos.y >= g_Gate[1].y_lower - HERO_WIDTH )
	{
		m_position.x = (m_position.x > g_Gate[1].x - HERO_WIDTH) ?
			g_Gate[1].x - HERO_WIDTH : m_position.x;
	}

	if( PrevPos.x > g_Gate[1].x - HERO_WIDTH )
	{
		m_position.x = (m_position.x > GAME_MAP_WIDTH - HERO_WIDTH) ?
			GAME_MAP_WIDTH - HERO_WIDTH : m_position.x;

		m_position.y = (m_position.y < g_Gate[1].y_upper) ?
			g_Gate[1].y_upper : m_position.y;
		m_position.y = (m_position.y > g_Gate[1].y_lower - HERO_WIDTH) ?
			g_Gate[1].y_lower - HERO_WIDTH : m_position.y;
	}
}

bool cHero::CanMove()
{
	return ( GetStateMachine()->IsInState(cNormalState::Instance()) ||
		GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) ||
		GetStateMachine()->IsInState(cSpelledByVoldemortState::Instance()) );
}

bool cHero::HaveGhostBall()
{
	return (m_pBall && !m_bGoldBall);
}

bool cHero::HaveGoldBall()
{
	return (m_pBall && m_bGoldBall);
}

bool cHero::CanSnatchGhostBall()
{
	if( !(GetStateMachine()->IsInState(cNormalState::Instance()) ||
		GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) ||
		GetStateMachine()->IsInState(cSpelledByVoldemortState::Instance())) )
	{
		return false;
	}

	if( m_SnatchInterval > 0 )
		return false;

	// we can not snatch the ball when it is moving in forbidden areas
	cGhostBall *pGhostBall = (cGhostBall*)g_Balls[1];
	if( pGhostBall->IsMoving() )
	{
		sPosition pos;
		pGhostBall->GetPos(pos);
		int i = cGame::FindHeroTeam(this);
		if( i == cGame::FindHeroTeam(pGhostBall->m_pPassballHero) )
		{
			i = i == 0 ? 1 : 0;
			if( pos.x > g_ForbiddenAreas[i].left - BALL_WIDTH && pos.x < g_ForbiddenAreas[i].right &&
				pos.y > g_ForbiddenAreas[i].top - BALL_WIDTH && pos.y < g_ForbiddenAreas[i].bottom )
			{
				return false;
			}
		}
	}

	// is the ball in range?
	sPosition pos1, pos2;
	pos1.x = m_position.x + HERO_WIDTH / 2;
	pos1.y = m_position.y + HERO_WIDTH / 2;
	g_Balls[1]->GetPos(pos2);
	pos2.x += BALL_WIDTH / 2;
	pos2.y += BALL_WIDTH / 2;

	double len = GetLength(pos1, pos2);

	return (len <= SNATCH_DISTANCE_GHOST);
}

bool cHero::CanSnatchGoldBall()
{
	if( !(GetStateMachine()->IsInState(cNormalState::Instance()) ||
		GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) ||
		GetStateMachine()->IsInState(cSpelledByVoldemortState::Instance())) )
	{
		return false;
	}

	if( m_SnatchInterval > 0 )
		return false;

	sPosition pos1, pos2;
	pos1.x = m_position.x + HERO_WIDTH / 2;
	pos1.y = m_position.y + HERO_WIDTH / 2;
	g_Balls[2]->GetPos(pos2);
	pos2.x += BALL_WIDTH;
	pos2.y += BALL_WIDTH;

	double len = GetLength(pos1, pos2);

	return (len <= SNATCH_DISTANCE_GOLD);
}

void cHero::setAction( int p_action )
{
	if( m_currAction != -1 && m_currAction != ACTION_HERO_SPELLING && m_currAction != ACTION_HERO_STOP )
	{
		m_prevAction = m_currAction;
	}

	m_currAction = p_action; 
	
	int spd = (m_pBall == NULL) ?
		(int)ceil(HERO_SPEED / sqrt(2.0)) : (int)ceil(HERO_SPEED_SLOW / sqrt(2.0));
	int spd_straight = (m_pBall == NULL) ?
		HERO_SPEED : HERO_SPEED_SLOW;

	// slow down if spelled by hermione
	if( GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) )
	{
		spd /= 2;
		spd_straight /= 2;
	}

	switch(p_action)
	{
	case ACTION_HERO_BOTTOM:
		m_speed.vx = 0;
		m_speed.vy = spd_straight;
		break;
	case ACTION_HERO_LEFTBOTTOM:
		m_speed.vx = -spd;
		m_speed.vy = spd;
		break;
	case ACTION_HERO_LEFT:
		m_speed.vx = -spd_straight;
		m_speed.vy = 0;
		break;
	case ACTION_HERO_LEFTTOP:
		m_speed.vx = -spd;
		m_speed.vy = -spd;
		break;
	case ACTION_HERO_TOP:
		m_speed.vx = 0;
		m_speed.vy = -spd_straight;
		break;
	case ACTION_HERO_RIGHTTOP:
		m_speed.vx = spd;
		m_speed.vy = -spd;
		break;
	case ACTION_HERO_RIGHT:
		m_speed.vx = spd_straight;
		m_speed.vy = 0;
		break;
	case ACTION_HERO_RIGHTBOTTOM:
		m_speed.vx = spd;
		m_speed.vy = spd;
		break;
	case ACTION_HERO_STOP:
	case ACTION_HERO_SPELLING:
		m_speed.vx = 0;
		m_speed.vy = 0;
		break;
	} /* end switch */

}

bool cHero::CanSpell()
{
	if( m_objectType != HERO_HARRY ) 
	{
		return ( GetStateMachine()->IsInState(cNormalState::Instance()) &&
			 m_currBlue >= m_SpellCost && m_SpellInterval == 0 );
	}
	else
	{
		return ( 
			( GetStateMachine()->IsInState(cNormalState::Instance()) ||
			GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) ||
			GetStateMachine()->IsInState(cSpelledByMalfoyState::Instance()) || 
			GetStateMachine()->IsInState(cSpelledByVoldemortState::Instance()) ) &&
			m_currBlue >= m_SpellCost && m_SnatchInterval == 0 );
	}
}

bool cHero::IsSpelling()
{
	return ( GetStateMachine()->IsInState(cSpellingState::Instance()) );
}

void cHero::SetTeamName( PCTSTR pszName )
{
	StringCchCopy(m_teamName, _countof(m_teamName), pszName);
}

void cHero::SetHeroName( PCTSTR pszName )
{
	StringCchCopy(m_szHeroName, _countof(m_szHeroName), pszName);
}

bool cHero::CanPassBall()
{
	return ( GetStateMachine()->IsInState(cNormalState::Instance()) ||
		GetStateMachine()->IsInState(cSpelledByHermioneState::Instance()) ||
		GetStateMachine()->IsInState(cSpelledByVoldemortState::Instance()) );
}

int cHero::GetSpellDirection( cHero *pHeroTarget )
{
	if( pHeroTarget == NULL )
		return 0;	// default

	sPosition posTarget = pHeroTarget->getPosition();

	if( m_position.x == posTarget.x && m_position.y >= posTarget.y )
		return 4;
	if( m_position.x == posTarget.x && m_position.y < posTarget.y )
		return 0;
	if( m_position.x > posTarget.x && m_position.y > posTarget.y )
		return 3;
	if( m_position.x > posTarget.x && m_position.y == posTarget.y )
		return 2;
	if( m_position.x > posTarget.x && m_position.y < posTarget.y )
		return 1;
	if( m_position.x < posTarget.x && m_position.y > posTarget.y )
		return 5;
	if( m_position.x < posTarget.x && m_position.y == posTarget.y )
		return 6;
	if( m_position.x < posTarget.x && m_position.y < posTarget.y )
		return 7;

	return GetTextureX();
}