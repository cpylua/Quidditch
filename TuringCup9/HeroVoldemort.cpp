#include "HeroVoldemort.h"
#include "HeroState.h"

int VOLDEMORT_SPELL_COST = 2500;
int VOLDEMORT_SPELL_INTERVAL = 300;
int VOLDEMORT_MAX_BLUE = 5500;

cHeroVoldemort::cHeroVoldemort( const sPosition &p_pos )
:cHero(p_pos)
{
	m_objectType = HERO_VOLDEMORT;
	m_maxBlue = m_currBlue = VOLDEMORT_MAX_BLUE;
	m_SpellCost = VOLDEMORT_SPELL_COST;
	m_StaticSpellInterval = VOLDEMORT_SPELL_INTERVAL;
}

cHeroVoldemort::~cHeroVoldemort()
{

}

void cHeroVoldemort::Spell()
{
	if( m_SpellInterval == 0 &&
		m_currBlue >= VOLDEMORT_SPELL_COST )
	{
		m_SpellInterval = VOLDEMORT_SPELL_INTERVAL;
		//g_SoundEffectIndex.push_back(SND_RON_GINNY_SPELLING);
		GetStateMachine()->ChangeState(cSpellingState::Instance());
		m_currBlue -= VOLDEMORT_SPELL_COST;
	}
}

