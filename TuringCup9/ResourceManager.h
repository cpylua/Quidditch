#ifndef		__RESOURCEMANAGER_H__
#define		__RESOURCEMANAGER_H__

#include "Engine/SoundEffect.h"
#include "Engine/Graphics.h"
#include "Engine/Texture.h"
#include "Engine/font.h"
#include "Engine/Music.h"
#include "GlobalDef.h"
#include "Map.h"

class cResourceManager
{
public:
	cResourceManager();
	~cResourceManager();

	void		init(cGraphics* p_pGraphics, HWND p_hWnd);
	void		Reset();
	void		Release();

	BOOL		OnLostDevice(){ return (BOOL)m_Font.getFontInterface()->OnLostDevice(); }

	bool		loadAllResources(HWND hwndDlg, D3DCOLOR p_keyColor = 0);
	bool		loadHero(Hero_Type p_heroType, D3DCOLOR p_keyColor = 0);
	bool		loadElements(D3DCOLOR p_keyColor = 0);
	bool		loadSoundEffect();
	bool		loadBackGroundMusic();
	bool		loadReayMusic();
	bool		loadMap();

	bool		playBackGroundMusic();
	bool		stopBackGroundMusic();
	bool		pauseBackGroundMusic();

	bool		playReadyMusic();
	bool		stopReadyMusic();

	cTexture*	GetHeroTexture(Hero_Type p_heroType);
	cTexture*	GetElementTexture(Element_Type p_Type);
	cMap*		getMap() { return &m_map; }

	cMusic*		GetMusicInterface() { return &m_backGroundMusic; }

public:	
	//hero texture
	cTexture*		m_heroRon;
	cTexture*		m_heroMalfoy;
	cTexture*		m_heroGinny;
	cTexture*		m_heroHermione;
	cTexture*		m_heroHarry;
	cTexture*		m_heroVoldemort;

	//magic texture
	cTexture*		m_Magic;
	cTexture*		m_Banner;
	cTexture*		m_Fireworks[4];

	cSoundEffect	m_SoundEffect;			//soundeffect component

	cMap			m_map;
	cFont			m_Font;					//font component
	cMusic			m_backGroundMusic;		//to play background music
	cMusic			m_readyMusic;			// play when waiting for start

	HWND			m_hWnd;
	cGraphics*		m_pGraphics;
};

#endif