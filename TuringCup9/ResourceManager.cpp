#include "ResourceManager.h"
#include <tchar.h>
#include <strsafe.h>

extern cGraphics*		g_pGraphics;

static TCHAR* szSoundEffect[] = 
{
	 	_T("../res/sound/slow_spelled_by_hermione.wav"),
	 	_T("../res/sound/bang_spelled_by_malfoy.wav"),
		_T("../res/sound/ron_ginny_spelling.wav"),
		_T("../res/sound/game_over.wav"),
		_T("../res/sound/new_turn.wav"),
		_T("../res/sound/spelled_by_harry.wav"),
		_T("../res/sound/spelled_by_voldemort.wav")
};

const int SOUNDEFFECT_NUM = _countof(szSoundEffect);
extern cSoundEffect*	g_pSoundEffect;



cResourceManager::cResourceManager()
{
	Reset();
}

cResourceManager::~cResourceManager()
{
}

void cResourceManager::init( cGraphics* p_pGraphics, HWND p_hWnd )
{
	m_pGraphics = p_pGraphics;
	m_hWnd		= p_hWnd;
}

void cResourceManager::Release()
{
	m_map.release();
	m_backGroundMusic.release();
	m_readyMusic.release();

	delete m_heroRon;
	delete m_heroGinny;
	delete m_heroHermione;
	delete m_heroMalfoy;
	delete m_Magic;
	delete m_heroHarry;
	delete m_heroVoldemort;
	delete m_Banner;

	for(int i = 0; i < 4; i++)
		delete m_Fireworks[i];

	m_SoundEffect.release();

	Reset();

	BASS_Free();
}

bool cResourceManager::loadBackGroundMusic()
{
	bool res = true;
 	res &= m_backGroundMusic.init();
 	res &= m_backGroundMusic.loadMusicFromFile("../res/sound/back.mp3");
	m_backGroundMusic.setPlayMode(true);	// loop
	return res;
}

bool cResourceManager::playBackGroundMusic()
{
	return m_backGroundMusic.play(false);
}

bool cResourceManager::stopBackGroundMusic()
{
	return m_backGroundMusic.stop();
}

bool cResourceManager::pauseBackGroundMusic()
{
	return m_backGroundMusic.pause();
}

bool cResourceManager::loadMap()
{
 	return m_map.loadMap( m_pGraphics );
}

bool cResourceManager::loadHero( Hero_Type p_heroType, D3DCOLOR p_keyColor /*= 0*/ )
{
	UNREFERENCED_PARAMETER(p_keyColor);

	bool res = false;
	switch( p_heroType )
	{
	case HERO_RON:
		if( !m_heroRon )
		{
			m_heroRon = new cTexture;
			res = m_heroRon->load( m_pGraphics, _T("../res/pic/ron.png"), 0, 1024, 1024 );
		}
		break;
	case HERO_MALFOY:
		if( !m_heroMalfoy )
		{
			m_heroMalfoy = new cTexture;
			res = m_heroMalfoy->load( m_pGraphics, _T("../res/pic/malfoy.png"), 0, 1024, 1024 );
		}
		break;
	case HERO_GINNY:
		if( !m_heroGinny )
		{
			m_heroGinny = new cTexture;
			res = m_heroGinny->load( m_pGraphics, _T("../res/pic/ginny.png"), 0, 1024, 1024 );
		}
		break;
	case HERO_HERMIONE:
		if( !m_heroHermione )
		{
			m_heroHermione = new cTexture;
			res = m_heroHermione->load( m_pGraphics, _T("../res/pic/hermione.png"), 0, 1024, 1024 );
		}
		break;

	case HERO_HARRY:
		if( !m_heroHarry )
		{
			m_heroHarry = new cTexture;
			res = m_heroHarry->load(m_pGraphics, TEXT("../res/pic/harry.png"), 0, 1024, 1024);
		}
		break;

	case HERO_VOLDEMORT:
		if( !m_heroVoldemort )
		{
			m_heroVoldemort = new cTexture;
			res = m_heroVoldemort->load(m_pGraphics, TEXT("../res/pic/voldemort.png"), 0, 1024, 1024);
		}
		break;
	}
	return res;
}

bool cResourceManager::loadAllResources( HWND hwndDlg, D3DCOLOR p_keyColor /*= 0*/ )
{
	UNREFERENCED_PARAMETER(p_keyColor);

	BASS_Init(-1, 44100, BASS_DEVICE_3D, m_hWnd, NULL);

	bool res = true;

	res &= loadHero(HERO_VOLDEMORT);
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadElements();
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadSoundEffect();
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadMap();
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadHero(HERO_RON);
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadHero(HERO_GINNY);
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadBackGroundMusic();
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadHero(HERO_MALFOY);
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadHero(HERO_HERMIONE);
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadReayMusic();
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);

	res &= loadHero(HERO_HARRY);
	SendMessage(hwndDlg, WM_LOAD_IN_PROGRESS, 0, 0);



	return res;
}

bool cResourceManager::loadSoundEffect()
{
	g_pSoundEffect = &m_SoundEffect;
	return m_SoundEffect.create( SOUNDEFFECT_NUM, szSoundEffect );
}

void cResourceManager::Reset()
{
	m_pGraphics		= NULL;
	m_hWnd			= NULL;	
	m_heroRon		= NULL;
	m_heroMalfoy	= NULL;
	m_heroGinny		= NULL;
	m_heroHermione	= NULL;
	m_Magic			= NULL;
	m_heroHarry		= NULL;
	m_heroVoldemort = NULL;
	m_Banner		= NULL;
	for(int i = 0; i < 4; i++)
		m_Fireworks[i] = NULL;
}

cTexture* cResourceManager::GetHeroTexture( Hero_Type p_heroType )
{
	switch( p_heroType )
	{
	case HERO_RON:
		return m_heroRon;
	case HERO_MALFOY:
		return m_heroMalfoy;
	case HERO_GINNY:
		return m_heroGinny;
	case HERO_HERMIONE:
		return m_heroHermione;
	case HERO_HARRY:
		return m_heroHarry;
	case HERO_VOLDEMORT:
		return m_heroVoldemort;
	default:
		return NULL;
	}
}

bool cResourceManager::loadElements( D3DCOLOR p_keyColor /*= 0*/ )
{
	UNREFERENCED_PARAMETER(p_keyColor);

	bool bRet = false;
	static TCHAR szFirework[32];

	if( !m_Magic )
	{
		m_Magic = new cTexture;
		bRet = m_Magic->load(m_pGraphics, TEXT("../res/pic/magic.png"), 0, 1024, 1024);
	}

	if( !m_Banner )
	{
		m_Banner = new cTexture;
		bRet &= m_Banner->load(m_pGraphics, TEXT("../res/pic/banner.png"), 0, 1024, 256);
	}

	for(int i = 0; i < 4; i++)
	{
		if( !m_Fireworks[i] )
		{
			m_Fireworks[i] = new cTexture;
			StringCchPrintf(szFirework, _countof(szFirework),
				TEXT("%s%d%s"), TEXT("../res/pic/firework"), i + 1, TEXT(".png"));
			bRet &= m_Fireworks[i]->load(m_pGraphics, szFirework, 0, 2048, 256);
		}
	}

	return bRet;
}

cTexture* cResourceManager::GetElementTexture( Element_Type p_Type )
{
	switch(p_Type)
	{
	case MAGICS:
		return m_Magic;

	case BANNER:
		return m_Banner;

	case FIREWORK1:
		return m_Fireworks[0];

	case FIREWORK2:
		return m_Fireworks[1];

	case FIREWORK3:
		return m_Fireworks[2];

	case FIREWORK4:
		return m_Fireworks[3];
		
	default:
		return NULL;
	}
}

bool cResourceManager::loadReayMusic()
{
	bool res = true;
	res &= m_readyMusic.init();
	res &= m_readyMusic.loadMusicFromFile("../res/sound/ready.mp3");
	m_readyMusic.setPlayMode(true);	// loop
	return res;
}

bool cResourceManager::playReadyMusic()
{
	return m_readyMusic.play(false);
}

bool cResourceManager::stopReadyMusic()
{
	return m_readyMusic.stop();
}