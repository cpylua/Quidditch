#pragma once
#include "Hero.h"

class cHeroVoldemort :
	public cHero
{
public:
	cHeroVoldemort(const sPosition &p_pos);
	~cHeroVoldemort();

	void Spell();
};