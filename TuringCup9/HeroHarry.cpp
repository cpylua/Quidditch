#include "HeroHarry.h"
#include "HeroState.h"

int HARRY_SPELL_COST = 2500;
int HARRY_SPELL_INTERVAL = 300;
int HARRY_MAX_BLUE = 5500;

cHeroHarry::cHeroHarry( const sPosition &p_pos )
:cHero(p_pos)
{
	m_objectType = HERO_HARRY;

	m_maxBlue = m_currBlue = HARRY_MAX_BLUE;
	m_SpellCost = HARRY_SPELL_COST;
	m_StaticSpellInterval = HARRY_SPELL_INTERVAL;
}

cHeroHarry::~cHeroHarry()
{

}

void cHeroHarry::Spell()
{
	if( m_SpellInterval == 0 &&
		m_currBlue >= HARRY_SPELL_COST )
	{
		m_SpellInterval = HARRY_SPELL_INTERVAL;
		//g_SoundEffectIndex.push_back(SND_RON_GINNY_SPELLING);
		GetStateMachine()->ChangeState(cSpellingState::Instance());
		m_currBlue -= HARRY_SPELL_COST;
	}
}
