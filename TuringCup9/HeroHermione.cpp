#include "HeroHermione.h"
#include "HeroState.h"

int HERMIONE_SPELL_COST = 2500;
int HERMIONE_SPELL_INTERVAL = 500;
int HERMIONE_MAX_BLUE = 5500;

cHeroHermione::cHeroHermione(const sPosition &p_pos)
:cHero(p_pos)
{
	m_objectType = HERO_HERMIONE;

	m_currBlue = m_maxBlue = HERMIONE_MAX_BLUE;
	m_SpellCost = HERMIONE_SPELL_COST;
	m_StaticSpellInterval = HERMIONE_SPELL_INTERVAL;
}

cHeroHermione::~cHeroHermione()
{

}

void cHeroHermione::Spell()
{
	if( m_SpellInterval == 0 &&
		m_currBlue >= HERMIONE_SPELL_COST )
	{
		m_SpellInterval = HERMIONE_SPELL_INTERVAL;
		//
		// add sound effect
		//
		GetStateMachine()->ChangeState(cSpellingState::Instance());
		m_currBlue -= HERMIONE_SPELL_COST;
	}
}

