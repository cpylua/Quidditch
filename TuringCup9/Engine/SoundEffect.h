/*
	@author:	zombie.fml
	@history:	11/22/2009

	@description:
		I have rewritten this whole class using BASS.DLL
*/


#pragma once

#include "bass.h"

#include <vector>

using namespace std;

#pragma comment(lib, "bass.lib")


class cSoundEffect
{
public:
	cSoundEffect();
	~cSoundEffect();

	bool	create(int num, TCHAR** p_szSoundEffect);
	bool	play(unsigned int index);

	void	release();	
private:
	vector<HSAMPLE> m_vecSamples;
};

