#pragma once
#include "Hero.h"

class cHeroHarry :
	public cHero
{
public:
	cHeroHarry(const sPosition &p_pos);
	~cHeroHarry();

	void Spell();
};