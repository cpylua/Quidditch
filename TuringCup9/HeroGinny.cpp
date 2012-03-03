#include "HeroGinny.h"
#include "HeroState.h"

int GINNY_SPELL_COST = 2500;
int GINNY_SPELL_INTERVAL = 300;
int GINNY_MAX_BLUE = 5500;

cHeroGinny::cHeroGinny( const sPosition &p_pos )
:cHero(p_pos)
{
	m_objectType = HERO_GINNY;

	m_maxBlue = m_currBlue = GINNY_MAX_BLUE;
	m_SpellCost = GINNY_SPELL_COST;
	m_StaticSpellInterval = GINNY_SPELL_INTERVAL;
}

cHeroGinny::~cHeroGinny()
{

}

void cHeroGinny::Spell()
{
	if( m_SpellInterval == 0 &&
		m_currBlue >= GINNY_SPELL_COST )
	{
		m_SpellInterval = GINNY_SPELL_INTERVAL;
		GetStateMachine()->ChangeState(cSpellingState::Instance());
		m_currBlue -= GINNY_SPELL_COST;
	}
}
