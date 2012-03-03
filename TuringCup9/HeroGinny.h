#pragma once
#include "Hero.h"

class cHeroGinny :
	public cHero
{
public:
	cHeroGinny(const sPosition &p_pos);
	~cHeroGinny();

	void Spell();
};