/*
	@author:	zombie.fml
	@history:	11/22/2009

	@description:
		I have rewritten this whole class using BASS.DLL
*/

#define _CRT_RAND_S
#include <stdlib.h>

#include "SoundEffect.h"

#include <StrSafe.h>


cSoundEffect::cSoundEffect(void)
{

}

cSoundEffect::~cSoundEffect(void)
{
	release();
}

bool cSoundEffect::create(int num, TCHAR** p_szSoundEffect)
{
	m_vecSamples.clear();

	for(int i = 0; i < num; i++)
	{
		HSAMPLE hSample = BASS_SampleLoad(FALSE, p_szSoundEffect[i],
			0, 0, 4, 
			BASS_UNICODE | BASS_SAMPLE_OVER_VOL);

		m_vecSamples.push_back(hSample);
	}

	return true;
}

void cSoundEffect::release()
{
	for(vector<HSAMPLE>::iterator i = m_vecSamples.begin(); i != m_vecSamples.end(); i++)
	{
		BASS_SampleFree(*i);
	}

	m_vecSamples.clear();
}

bool cSoundEffect::play(unsigned int index)
{
	if( index < m_vecSamples.size() )
	{
		HCHANNEL hChannel = BASS_SampleGetChannel(m_vecSamples[index], FALSE);

		BASS_ChannelSetAttribute(hChannel, BASS_ATTRIB_VOL, 1.0f);
		BASS_ChannelSetAttribute(hChannel, BASS_ATTRIB_PAN, 0.0f);

		return BASS_ChannelPlay(hChannel, TRUE) == TRUE;
	}

	return false;
}


