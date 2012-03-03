#include "HeroRon.h"
#include "HeroState.h"

int RON_SPELL_COST = 2000;
int RON_SPELL_INTERVAL = 350;
int RON_MAX_BLUE = 4500;

cHeroRon::cHeroRon(const sPosition& p_pos)
:cHero(p_pos)
{
	m_objectType = HERO_RON;

	m_currBlue = m_maxBlue = RON_MAX_BLUE;
	m_SpellCost = RON_SPELL_COST;
	m_StaticSpellInterval = RON_SPELL_INTERVAL;
}

cHeroRon::~cHeroRon(void)
{
}

void cHeroRon::Spell()
{
	if( m_SpellInterval == 0 &&
		m_currBlue >= RON_SPELL_COST )
	{
		m_SpellInterval = RON_SPELL_INTERVAL;
		GetStateMachine()->ChangeState(cSpellingState::Instance());
		m_currBlue -= RON_SPELL_COST;
	}
}
