#include "HeroState.h"
#include "Hero.h"
#include <D3d9types.h>
#include "Engine/Texture.h"
#include "Game.h"
#include "cBall.h"


int SPELLED_BY_GINNY_PERIOD	= 30;
int SPELLED_BY_RON_PERIOD = 30;
int SPELLED_BY_HARRY_PERIOD	= 30;

int SPELLED_BY_FREEBALL_PERIOD;
int SPELLED_BY_MALFOY_PERIOD;
int SPELLED_BY_HERMIONE_PERIOD;
int SPELLED_BY_VOLDEMORT_PERIOD;
int SPELLED_BY_VOLDEMORT_DELTA;

#define LONG_SPELLE_PERIOD		20
#define SHORT_SPELLE_PERIOD		20

//////////////////////////////////////////////////////////////////////////
///////////////////////////////Normal State///////////////////////////////
//////////////////////////////////////////////////////////////////////////
void cNormalState::Enter( cHero* p_pHero )
{
	p_pHero->m_AbnormalType = SPELLED_BY_NONE;
}

void cNormalState::Update( cHero* p_pHero )
{
	switch(p_pHero->getAction())
	{
	case ACTION_HERO_BOTTOM:
	case ACTION_HERO_LEFTBOTTOM:
	case ACTION_HERO_LEFT:
	case ACTION_HERO_LEFTTOP:
	case ACTION_HERO_TOP:	
	case ACTION_HERO_RIGHTTOP:
	case ACTION_HERO_RIGHT:
	case ACTION_HERO_RIGHTBOTTOM:
	case ACTION_HERO_STOP:
		{
			p_pHero->move();
			break;
		}

	case ACTION_HERO_SPELLING:
		{
			p_pHero->Spell();
			break;
		}
	} /* end switch */
}

void cNormalState::Exit( cHero* p_pHero )
{
	UNREFERENCED_PARAMETER(p_pHero);
}

void cNormalState::Render( cHero* p_pHero )
{
	cTexture* pTex = p_pHero->GetTexture();
	const sPosition& pos = p_pHero->getScreenPosition();
	int xTex = p_pHero->GetTextureX();
	int yTex = p_pHero->GetTextureY();

	pTex->draw( pos.x, pos.y, xTex, yTex, HERO_WIDTH, HERO_WIDTH );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
///////////////////////////////Spelling State/////////////////////////////
//////////////////////////////////////////////////////////////////////////
void cSpellingState::Enter( cHero* p_pHero )
{
	p_pHero->m_currSpellStep = 0;
	p_pHero->m_AbnormalType = SPELLED_BY_NONE;

	m_BottomIndex = m_HandIndex = 0;
}

void cSpellingState::Update( cHero* p_pHero )
{
	int iSpellTime;

	if( p_pHero->m_objectType == HERO_GINNY || 
		p_pHero->m_objectType == HERO_RON )
	{
		iSpellTime = LONG_SPELLE_PERIOD;
	}
	else
	{
		iSpellTime = SHORT_SPELLE_PERIOD;
	}

	if( p_pHero->m_currSpellStep < iSpellTime )
	{
		if( p_pHero->m_currSpellStep == iSpellTime / 3 )
		{
			m_BottomIndex++;
		}
		else if( p_pHero->m_currSpellStep == 2 * iSpellTime / 3 )
		{
			m_BottomIndex++;
		}

		if( p_pHero->m_currAbnormalStep % 3 == 0 )
		{
			m_HandIndex++;
			m_HandIndex %= 2;
		}

		p_pHero->m_currSpellStep++;
		return;
	}
	
	if( !p_pHero->m_spellObject )
		return ;
	switch(p_pHero->m_objectType)
	{
	case HERO_RON:
		p_pHero->m_spellObject->GetStateMachine()->ChangeState(cSpelledByRonState::Instance());
		break;

	case HERO_GINNY:
		p_pHero->m_spellObject->GetStateMachine()->ChangeState(cSpelledByGinnyState::Instance());
		break;

	case HERO_MALFOY:
		p_pHero->m_spellObject->GetStateMachine()->ChangeState(cSpelledByMalfoyState::Instance());
		break;

	case HERO_HERMIONE:
		p_pHero->m_spellObject->GetStateMachine()->ChangeState(cSpelledByHermioneState::Instance());
		break;

	case HERO_HARRY:
		p_pHero->m_spellObject->GetStateMachine()->ChangeState(cSpelledByHarryState::Instance());
		p_pHero->GetStateMachine()->ChangeState(cSpelledByHarryState::Instance());
		return;		/* return cuz' harry potter is special */

	case HERO_VOLDEMORT:
		p_pHero->m_spellObject->GetStateMachine()->ChangeState(cSpelledByVoldemortState::Instance());
		break;
	}

	// spelle done
	p_pHero->GetStateMachine()->ChangeState(cNormalState::Instance());
	p_pHero->m_spellObject = NULL;
}

void cSpellingState::Exit( cHero* p_pHero )
{
	UNREFERENCED_PARAMETER(p_pHero);

	//p_pHero->m_spellObject = NULL;
}


void cSpellingState::Render( cHero* p_pHero )
{
	cTexture *pTex = p_pHero->GetTexture();

	sPosition pos = p_pHero->getScreenPosition();
	int iTexX = p_pHero->GetSpellDirection(p_pHero->m_spellObject) * HERO_WIDTH;
	int iTexY = p_pHero->GetTextureY();

	pTex->draw(pos.x, pos.y, iTexX, iTexY, HERO_WIDTH, HERO_WIDTH);

	D3DCOLOR color = D3DCOLOR_ARGB(125, 255, 255, 255);
	cTexture *pTexMagic = cGame::GetResourceManager()->GetElementTexture(MAGICS);
	iTexY = 7 * HERO_WIDTH;
	iTexX = m_BottomIndex;
	pTexMagic->draw(pos.x, pos.y + 2 * HERO_WIDTH / 5, iTexX, iTexY, HERO_WIDTH, HERO_WIDTH, color);

	iTexY = m_HandIndex;
	pTexMagic->draw(pos.x, pos.y, iTexX, iTexY, HERO_WIDTH, HERO_WIDTH, color);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
///////////////////////////Spelled by Ron State///////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef struct Pos_tag
{
	int x;
	int y;
} Pos;
static const Pos iCornors[] = {
	{g_Gate[0].x + HERO_WIDTH, 0 + 16},
	{g_Gate[0].x + HERO_WIDTH, 768 - HERO_WIDTH},
	{g_Gate[1].x - 2 * HERO_WIDTH, 0 + 16},
	{g_Gate[1].x - 2 * HERO_WIDTH, 768 - HERO_WIDTH}
};

void cSpelledByRonState::Enter( cHero* p_pHero )
{
	p_pHero->m_currFrame = 0;
	p_pHero->m_currAbnormalStep = 0;
	p_pHero->m_AbnormalType = SPELLED_BY_RON;

	g_SoundEffectIndex.push_back(SND_RON_GINNY_SPELLING);
}

void cSpelledByRonState::Update( cHero* p_pHero )
{
	if( p_pHero->m_currAbnormalStep <= SPELLED_BY_RON_PERIOD )
	{
		if( p_pHero->m_currAbnormalStep == 3)
			++p_pHero->m_currFrame;
		else if( p_pHero->m_currAbnormalStep == 6 )
			++p_pHero->m_currFrame;

		p_pHero->m_currAbnormalStep++;

		// blow away
		if( p_pHero->m_currAbnormalStep == 2 * SPELLED_BY_RON_PERIOD / 3 )
		{
			Pos p;
			unsigned int rnd;
			if( p_pHero == g_HeroTeam1[0] || p_pHero == g_HeroTeam1[1] )
			{
				rand_s(&rnd);
				p = iCornors[rnd % 2];
			}
			else
			{
				rand_s(&rnd);
				p = iCornors[ rnd % 2 + 2 ];
			}

			p_pHero->SetPos(p.x, p.y);
			cGhostBall *pGhostBall = (cGhostBall *)p_pHero->m_pBall;
			if( pGhostBall )
			{
				pGhostBall->SetPos(p.x + HERO_WIDTH / 4, p.y + HERO_WIDTH / 4);
			}
		}

		return;
	}

	p_pHero->GetStateMachine()->ChangeState(cNormalState::Instance());
}

void cSpelledByRonState::Exit( cHero* p_pHero )
{
	UNREFERENCED_PARAMETER(p_pHero);
}

void cSpelledByRonState::Render( cHero* p_pHero )
{
	cTexture *pTexHero = p_pHero->GetTexture();
	cTexture *pTexMagic = cGame::GetResourceManager()->GetElementTexture(MAGICS);
	sPosition pos = p_pHero->getScreenPosition();
	int iTexHeroX = p_pHero->GetTextureX();
	int iTexHeroY = p_pHero->GetTextureY();
	int iTexMagicX = p_pHero->m_currFrame * HERO_WIDTH;
	int iTexmagicY = 0;

	D3DCOLOR color = D3DCOLOR_ARGB(125, 255, 255, 255);

	pTexHero->draw(pos.x, pos.y, iTexHeroX, iTexHeroY, HERO_WIDTH, HERO_WIDTH);
	pTexMagic->draw(pos.x, pos.y, iTexMagicX, iTexmagicY, HERO_WIDTH, HERO_WIDTH, color);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
////////////////////////Spelled by Hermione State/////////////////////////
//////////////////////////////////////////////////////////////////////////

void cSpelledByHermioneState::Enter( cHero* p_pHero )
{
	p_pHero->m_currFrame = 0;
	p_pHero->m_currAbnormalStep = 0;

	p_pHero->m_AbnormalType = SPELLED_BY_HERMIONE;

	m_OldSpeed.vx = p_pHero->m_speed.vx;
	m_OldSpeed.vy = p_pHero->m_speed.vy;
	p_pHero->m_speed.vx /= 2;
	p_pHero->m_speed.vy /= 2;

	g_SoundEffectIndex.push_back(SND_SPELLED_BY_HERMIONE);
}

void cSpelledByHermioneState::Update( cHero* p_pHero )
{
	if( p_pHero->m_currAbnormalStep <= SPELLED_BY_HERMIONE_PERIOD )
	{
		p_pHero->m_currAbnormalStep++;
	}
	else
	{
		p_pHero->GetStateMachine()->ChangeState(cNormalState::Instance());
		return;
	}

	//g_SoundEffectIndex.push_back(SND_SPELLED_BY_HERMIONE);

	switch(p_pHero->getAction())
	{
	case ACTION_HERO_BOTTOM:
	case ACTION_HERO_LEFTBOTTOM:
	case ACTION_HERO_LEFT:
	case ACTION_HERO_LEFTTOP:
	case ACTION_HERO_TOP:
	case ACTION_HERO_RIGHTTOP:
	case ACTION_HERO_RIGHT:
	case ACTION_HERO_RIGHTBOTTOM:
	case ACTION_HERO_STOP:
		{
			p_pHero->move();
			break;
		}
	case ACTION_HERO_SPELLING:
		if( p_pHero->m_objectType == HERO_HARRY )
			p_pHero->Spell();
		break;
	}
}

void cSpelledByHermioneState::Exit( cHero* p_pHero )
{
	int spd = (p_pHero->m_pBall == NULL) ?
		(int)ceil(HERO_SPEED / sqrt(2.0)) : (int)(HERO_SPEED_SLOW / sqrt(2.0));
	int spd_straight = (p_pHero->m_pBall == NULL) ? HERO_SPEED : HERO_SPEED_SLOW;

	switch(p_pHero->m_currAction)
	{
	case ACTION_HERO_BOTTOM:
		p_pHero->m_speed.vx = 0;
		p_pHero->m_speed.vy = spd_straight;
		break;
	case ACTION_HERO_LEFTBOTTOM:
		p_pHero->m_speed.vx = -spd;
		p_pHero->m_speed.vy = spd;
		break;
	case ACTION_HERO_LEFT:
		p_pHero->m_speed.vx = -spd_straight;
		p_pHero->m_speed.vy = 0;
		break;
	case ACTION_HERO_LEFTTOP:
		p_pHero->m_speed.vx = -spd;
		p_pHero->m_speed.vy = -spd;
		break;
	case ACTION_HERO_TOP:
		p_pHero->m_speed.vx = 0;
		p_pHero->m_speed.vy = -spd_straight;
		break;
	case ACTION_HERO_RIGHTTOP:
		p_pHero->m_speed.vx = spd;
		p_pHero->m_speed.vy = -spd;
		break;
	case ACTION_HERO_RIGHT:
		p_pHero->m_speed.vx = spd_straight;
		p_pHero->m_speed.vy = 0;
		break;
	case ACTION_HERO_RIGHTBOTTOM:
		p_pHero->m_speed.vx = spd;
		p_pHero->m_speed.vy = spd;
		break;
	case ACTION_HERO_STOP:
	case ACTION_HERO_SPELLING:
		p_pHero->m_speed.vx = 0;
		p_pHero->m_speed.vy = 0;
		break;
	} /* end switch */
}

void cSpelledByHermioneState::Render( cHero* p_pHero )
{
	cTexture *pTexHero = p_pHero->GetTexture();
	sPosition pos = p_pHero->getScreenPosition();
	int iTexX = p_pHero->GetTextureX();
	int iTexY = p_pHero->GetTextureY();

	pTexHero->draw(pos.x, pos.y, iTexX, iTexY, HERO_WIDTH, HERO_WIDTH);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////Spelled by Malfoy State/////////////////////////
//////////////////////////////////////////////////////////////////////////
#define BANG_STEP	25
#define BANG_POS_Y	64
#define BANG_POS_X	64

void cSpelledByMalfoyState::Enter( cHero* p_pHero )
{
	p_pHero->m_currAbnormalStep = 1;
	p_pHero->m_currFrame = 0;

	p_pHero->m_AbnormalType = SPELLED_BY_MALFOY;

	sPosition pos;
	p_pHero->GetPos(pos);

	m_BangPosY = pos.y -  HERO_WIDTH / 2 - BANG_POS_Y;
	m_BangPosX = pos.x + BANG_POS_X;

	g_SoundEffectIndex.push_back(SND_SPELLED_BY_MALFOY);
}

void cSpelledByMalfoyState::Update( cHero* p_pHero )
{
	if( p_pHero->m_currAbnormalStep <= SPELLED_BY_MALFOY_PERIOD )
	{
		// bang
		if( p_pHero->m_currAbnormalStep <= BANG_STEP )
		{
			p_pHero->m_currAbnormalStep++;

			// Update hammer position
			sPosition pos = p_pHero->getPosition();

			if( m_BangPosY < pos.y - HERO_WIDTH / 2 )
				m_BangPosY += 2;

			if( m_BangPosX > pos.x )
				m_BangPosX -= 2;

			return;
		}

		// loop
		if( p_pHero->m_currAbnormalStep % 6 == 0 )
		{
			p_pHero->m_currFrame++;
			p_pHero->m_currFrame = p_pHero->m_currFrame % 3 + 1;
		}

		p_pHero->m_currAbnormalStep++;

		if( p_pHero->getAction() == ACTION_HERO_SPELLING &&
			p_pHero->m_objectType == HERO_HARRY )
		{
			p_pHero->Spell();
		}
	}
	else
		p_pHero->GetStateMachine()->ChangeState(cNormalState::Instance());

}

void cSpelledByMalfoyState::Exit( cHero* p_pHero )
{
	UNREFERENCED_PARAMETER(p_pHero);
}

void cSpelledByMalfoyState::Render( cHero* p_pHero )
{
	cTexture *pTexHero = p_pHero->GetTexture();
	cTexture *pTexMagic = cGame::GetResourceManager()->GetElementTexture(MAGICS);

	sPosition pos = p_pHero->getScreenPosition();
	int iHeroTexX = p_pHero->GetTextureX();
	int iHeroTexY = p_pHero->GetTextureY();
	int road = cGame::GetResourceManager()->getMap()->GetRoad();

	int iMagicTexX = p_pHero->m_currFrame * HERO_WIDTH;
	int iMagicTexY = 1 * HERO_WIDTH;

	D3DCOLOR color = D3DCOLOR_ARGB(196, 255, 255, 255);
	pTexHero->draw(pos.x, pos.y, iHeroTexX, iHeroTexY, HERO_WIDTH, HERO_WIDTH);
	pTexMagic->draw((p_pHero->m_currFrame == 0) ? m_BangPosX - road : pos.x, 
		(p_pHero->m_currFrame == 0) ? m_BangPosY : pos.y - HERO_WIDTH / 2,
		iMagicTexX, iMagicTexY, HERO_WIDTH, HERO_WIDTH, color);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////Spelled by Ginny State//////////////////////////
//////////////////////////////////////////////////////////////////////////

void cSpelledByGinnyState::Enter( cHero* p_pHero )
{
	p_pHero->m_currFrame = 0;
	p_pHero->m_currAbnormalStep = 0;

	p_pHero->m_AbnormalType = SPELLED_BY_GINNY;

	//
	// deal with the ball
	//
	if( p_pHero->HaveGhostBall() )
	{
		unsigned int rnd;
		rand_s(&rnd);
		g_Balls[1]->SetPos(GAME_MAP_WIDTH / 2 - BALL_WIDTH / 2, 
			(int)((double)rnd / UINT_MAX * (GAME_MAP_HEIGHT - BALL_WIDTH)));
		((cGhostBall*)g_Balls[1])->m_pOwner = NULL;

		sSpeed spd;
		spd.vx = spd.vy = 0;
		g_Balls[1]->SetSpeed(spd);

		p_pHero->LoseBall();
	}

	g_SoundEffectIndex.push_back(SND_RON_GINNY_SPELLING);
}


void cSpelledByGinnyState::Update( cHero* p_pHero )
{
	if( p_pHero->m_currAbnormalStep <= SPELLED_BY_GINNY_PERIOD )
	{
		if( p_pHero->m_currAbnormalStep == SPELLED_BY_GINNY_PERIOD / 3 )
			p_pHero->m_currFrame++;
		else if( p_pHero->m_currAbnormalStep == SPELLED_BY_GINNY_PERIOD * 2 / 3 )
			p_pHero->m_currFrame++;

		p_pHero->m_currAbnormalStep++;

		return;
	}
	
	p_pHero->GetStateMachine()->ChangeState(cNormalState::Instance());
}

void cSpelledByGinnyState::Exit( cHero* p_pHero )
{
	UNREFERENCED_PARAMETER(p_pHero);
}

void cSpelledByGinnyState::Render( cHero* p_pHero )
{
	cTexture *pTexHero = p_pHero->GetTexture();
	cTexture *pTexMagic = cGame::GetResourceManager()->GetElementTexture(MAGICS);

	sPosition pos = p_pHero->getScreenPosition();

	int iHeroTexX = p_pHero->GetTextureX();
	int iHeroTexY = p_pHero->GetTextureY();
	int iMagicTexX = p_pHero->m_currFrame * HERO_WIDTH;
	int iMagicTexY = 3 * HERO_WIDTH;

	D3DCOLOR color = D3DCOLOR_ARGB(125, 255, 255, 255);
	pTexHero->draw(pos.x, pos.y, iHeroTexX, iHeroTexY, HERO_WIDTH, HERO_WIDTH);
	pTexMagic->draw(pos.x, pos.y - HERO_WIDTH / 2, iMagicTexX, iMagicTexY, HERO_WIDTH, HERO_WIDTH, color);
}

//////////////////////////////////////////////////////////////////////////
//	Spelled by freeball state	//
//////////////////////////////////////////////////////////////////////////

void cSpelledByFreeballState::Enter( cHero* p_pHero )
{
	p_pHero->m_currAbnormalStep = 0;
	p_pHero->m_currFrame = 0;

	p_pHero->m_AbnormalType = SPELLED_BY_FREEBALL;
}

void cSpelledByFreeballState::Update( cHero* p_pHero )
{
	if( p_pHero->m_currAbnormalStep <= SPELLED_BY_FREEBALL_PERIOD )
	{
		if( p_pHero->m_currAbnormalStep == SPELLED_BY_FREEBALL_PERIOD / 3 )
			p_pHero->m_currFrame++;
		if( p_pHero->m_currAbnormalStep == SPELLED_BY_FREEBALL_PERIOD * 2 / 3 )
			p_pHero->m_currFrame++;

		p_pHero->m_currAbnormalStep++;
	}
	else
	{
		p_pHero->GetStateMachine()->ChangeState(cNormalState::Instance());
	}
}

void cSpelledByFreeballState::Exit( cHero* p_pHero )
{
	UNREFERENCED_PARAMETER(p_pHero);
}

void cSpelledByFreeballState::Render( cHero* p_pHero )
{
	D3DCOLOR color = D3DCOLOR_ARGB(125, 255, 255, 255);
	cTexture *pTexHero = p_pHero->GetTexture();
	sPosition pos = p_pHero->getScreenPosition();

	cTexture *pTexMagic = cGame::GetResourceManager()->GetElementTexture(MAGICS);

	int iTexHeroX = p_pHero->GetTextureX();
	int iTexHeroY = p_pHero->GetTextureY();
	int iTexMagicX = p_pHero->m_currFrame * HERO_WIDTH;
	int iTexMagicY = 6 * HERO_WIDTH;

	pTexHero->draw(pos.x, pos.y, iTexHeroX, iTexHeroY, HERO_WIDTH, HERO_WIDTH);

	pTexMagic->draw(pos.x, pos.y - HERO_WIDTH / 2, iTexMagicX, iTexMagicY, HERO_WIDTH, HERO_WIDTH, color);
}

/*
	Spelled by Harry state
*/
void cSpelledByHarryState::Enter( cHero* p_pHero )
{
	p_pHero->m_currAbnormalStep = 0;
	p_pHero->m_currFrame = 0;

	p_pHero->m_AbnormalType = SPELLED_BY_HARRY;

	g_SoundEffectIndex.push_back(SND_SPELLED_BY_HARRY);
}

void cSpelledByHarryState::Update( cHero* p_pHero )
{
	if( p_pHero->m_currAbnormalStep <= SPELLED_BY_HARRY_PERIOD )
	{
		if( p_pHero->m_currAbnormalStep == 3 )
			p_pHero->m_currFrame++;
		if( p_pHero->m_currAbnormalStep == 6 )
			p_pHero->m_currFrame++;

		p_pHero->m_currAbnormalStep++;
	}
	else
	{
		p_pHero->GetStateMachine()->ChangeState(cNormalState::Instance());
	}
}

void cSpelledByHarryState::Exit( cHero* p_pHero )
{
	UNREFERENCED_PARAMETER(p_pHero);
}

void cSpelledByHarryState::Render( cHero* p_pHero )
{
	cTexture *pTexHero = p_pHero->GetTexture();
	sPosition pos = p_pHero->getScreenPosition();

	cTexture *pTexMagic = cGame::GetResourceManager()->GetElementTexture(MAGICS);

	int iTexHeroX = p_pHero->GetTextureX();
	int iTexHeroY = p_pHero->GetTextureY();
	int iTexMagicX = p_pHero->m_currFrame * HERO_WIDTH;
	int iTexMagicY = 2 * HERO_WIDTH;

	pTexHero->draw(pos.x, pos.y, iTexHeroX, iTexHeroY, HERO_WIDTH, HERO_WIDTH);
	D3DCOLOR color = D3DCOLOR_ARGB(125, 255, 255, 255);
	pTexMagic->draw(pos.x, pos.y, iTexMagicX, iTexMagicY, HERO_WIDTH, HERO_WIDTH, color);
}


/*
	Spelled by Voldemort state
*/
void cSpelledByVoldemortState::Enter( cHero* p_pHero )
{
	p_pHero->m_currAbnormalStep = 0;
	p_pHero->m_currFrame = 0;

	p_pHero->m_AbnormalType = SPELLED_BY_VOLDEMORT;

	m_Glint = 0;

	g_SoundEffectIndex.push_back(SND_SPELLED_BY_VOLDEMORT);
}

void cSpelledByVoldemortState::Update( cHero* p_pHero )
{
	if( p_pHero->m_currAbnormalStep <= SPELLED_BY_VOLDEMORT_PERIOD )
	{
		p_pHero->m_currBlue -= SPELLED_BY_VOLDEMORT_DELTA;
		p_pHero->m_currBlue %= p_pHero->m_maxBlue + 1;

		p_pHero->m_currAbnormalStep++;

		if( p_pHero->m_currAbnormalStep % 5 == 0 )
		{
			m_Glint++;
			m_Glint %= 2;
		}
	}
	else
	{
		p_pHero->GetStateMachine()->ChangeState(cNormalState::Instance());
		return;
	}

	switch(p_pHero->getAction())
	{
	case ACTION_HERO_BOTTOM:
	case ACTION_HERO_LEFTBOTTOM:
	case ACTION_HERO_LEFT:
	case ACTION_HERO_LEFTTOP:
	case ACTION_HERO_TOP:
	case ACTION_HERO_RIGHTTOP:
	case ACTION_HERO_RIGHT:
	case ACTION_HERO_RIGHTBOTTOM:
	case ACTION_HERO_STOP:
		{
			p_pHero->move();
			break;
		}
	case ACTION_HERO_SPELLING:
		if( p_pHero->m_objectType == HERO_HARRY )
			p_pHero->Spell();
		break;
	}
}

void cSpelledByVoldemortState::Exit( cHero* p_pHero )
{
	UNREFERENCED_PARAMETER(p_pHero);
}

void cSpelledByVoldemortState::Render( cHero* p_pHero )
{
	cTexture *pTexHero = p_pHero->GetTexture();
	int iTexX = p_pHero->GetTextureX();
	int iTexY = p_pHero->GetTextureY();
	sPosition pos = p_pHero->getScreenPosition();

	if( m_Glint == 0 )
	{
		if( p_pHero->m_pBall )
			iTexY = 1;
		else
			iTexY = 0;
	}

	pTexHero->draw(pos.x, pos.y, iTexX, iTexY, HERO_WIDTH, HERO_WIDTH);
}