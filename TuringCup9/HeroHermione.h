#pragma once
#include "Hero.h"

class cHeroHermione :
	public cHero
{
public:
	cHeroHermione(const sPosition &p_pos);
	~cHeroHermione();

	void Spell();
};