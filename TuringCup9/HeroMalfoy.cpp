#include "HeroMalfoy.h"
#include "HeroState.h"

int MALFOY_SPELL_COST = 2500;
int MALFOY_SPELL_INTERVAL = 500;
int MALFOY_MAX_BLUE = 5500;

void cHeroMalfoy::Spell()
{
	if( m_SpellInterval == 0 &&
		m_currBlue >= MALFOY_SPELL_COST )
	{
		m_SpellInterval = MALFOY_SPELL_INTERVAL;
		//
		// add sound effect
		//
		GetStateMachine()->ChangeState(cSpellingState::Instance());
		m_currBlue -= MALFOY_SPELL_COST;
	}
}


cHeroMalfoy::cHeroMalfoy( const sPosition &p_pos )
:cHero(p_pos)
{
	m_objectType = HERO_MALFOY;

	m_currBlue = m_maxBlue = MALFOY_MAX_BLUE;
	m_SpellCost = MALFOY_SPELL_COST;
	m_StaticSpellInterval = MALFOY_SPELL_INTERVAL;
}

cHeroMalfoy::~cHeroMalfoy()
{
}

