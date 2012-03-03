/*
	@author: zombie.fml

	@description:
	this class is intended only for playing background music,

	I was using TuringCup8Music.dll, but it has some problems with music playback,

	so I moved to the famous BASS.dll

*/

#pragma once

#include "bass.h"

class cMusic
{
public:
	cMusic();
	~cMusic();

	bool init();
	bool loadMusicFromFile(const char* p_musicFile);	
	bool setPlayMode(bool p_Loop);
	bool release();
	bool play(bool p_Reset);
	bool pause();
	bool stop();	
	
private:
	HSTREAM m_hStream;
};
