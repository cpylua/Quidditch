#pragma once
#include "hero.h"

class cHeroRon :
	public cHero
{
public:
	cHeroRon(const sPosition& p_pos);
	~cHeroRon(void);

	void	Spell();
};
