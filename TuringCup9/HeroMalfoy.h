#pragma once
#include "Hero.h"

class cHeroMalfoy : 
	public cHero
{
public:
	cHeroMalfoy(const sPosition &p_pos);
	~cHeroMalfoy();

	void Spell();
};
