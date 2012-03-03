/*
	@author: zombie.fml

	@description:
	this class is intended only for playing background music,

	I was using TuringCup8Music.dll, but it has some problems with music playback,

	so I moved to the famous BASS.dll

*/

#include "Music.h"

#pragma comment(lib, "bass.lib")

#define ASSERT_BASS_INIT_DONE(h) {if(h == NULL) return false;}

bool cMusic::init()
{
	m_hStream = NULL;

	return true;
}

bool cMusic::loadMusicFromFile( const char* p_musicFile )
{
	m_hStream = BASS_StreamCreateFile(FALSE, p_musicFile, 0, 0, 0);

	return (m_hStream != NULL);
}

bool cMusic::setPlayMode( bool p_Loop )
{
	ASSERT_BASS_INIT_DONE(m_hStream);

	DWORD dwFlagSet = p_Loop ? BASS_SAMPLE_LOOP : 0;
	DWORD dwRet = BASS_ChannelFlags(m_hStream, dwFlagSet, BASS_SAMPLE_LOOP);

	return (dwRet != -1);
}

bool cMusic::release()
{
	ASSERT_BASS_INIT_DONE(m_hStream);

	BASS_StreamFree(m_hStream);
	m_hStream = NULL;

	return true;
}

bool cMusic::play( bool p_Reset )
{
	ASSERT_BASS_INIT_DONE(m_hStream);

	BOOL bReset = p_Reset ? TRUE : FALSE;
	BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, 0.4f);
	BOOL bRet = BASS_ChannelPlay(m_hStream, bReset);

	return (bRet ? true : false);
}

bool cMusic::pause()
{
	ASSERT_BASS_INIT_DONE(m_hStream);

	BOOL bRet = BASS_ChannelPause(m_hStream);

	return (bRet ? true : false);
}

bool cMusic::stop()
{
	ASSERT_BASS_INIT_DONE(m_hStream);

	BOOL bRet = BASS_ChannelStop(m_hStream);

	return (bRet ? true : false);
}

cMusic::cMusic()
{
	init();
}

cMusic::~cMusic()
{
	release();
}