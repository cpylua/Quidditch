/*--------------------------------------------------------------------
*	Author:		Altfee
*	Created:	2008/8/14
*	File:		Graphics.h
*--------------------------------------------------------------------*/

#ifndef		__GRAPHICS_H__
#define		__GRAPHICS_H__

#include <windows.h>
#include "d3d9.h"
#include "Core_GlobalDef.h"
#include "D3dx9core.h"
#pragma comment(lib, "d3d9.lib")

class cGraphics
{
public:
	cGraphics();
	~cGraphics();

	bool	init();
	bool	destroy();

	bool	setMode(HWND p_hWnd, bool p_windowed = true, \
		long p_width = 0, long p_height = 0,
		char p_BBP = 0);

	//bool	getDisplayModeInfo(long p_num, D3DDISPLAYMODE *p_mode, \
	//							D3DFORMAT p_format);

	char	getFormatBBP(D3DFORMAT p_format);
	bool	checkFormat(D3DFORMAT p_format, bool p_windowed);
	bool	display();

	bool	beginScene();
	bool	endScene();

	bool	TestDevice();
	bool	clearDisplay(long color = 0);
	long	getWidth()  { return m_width;  }
	long	getHeight() { return m_height; }
	char	getBPP()	{ return m_BPP;	   }

	//inline getHAL();

	//inline bool setLight();
	inline bool setMaterial();	//cMaterial* p_pMaterial
	inline bool setTexture();	//short p_num,cTexture* p_pTexture

	//inline bool	setAmbientLight(char p_red, char p_green, char p_blue);
	//inline bool getAmbientLight(char* p_pRed, char* p_pGreen, char* p_pBlue);

	//inline bool enableLight(long p_num, bool p_enable = true);
	//inline bool enableLighting(bool p_enable = true);
	inline bool	enableAlphaBlending(bool p_enable = true, \
		long p_src = D3DBLEND_SRCALPHA,\
		long p_dest = D3DBLEND_INVSRCALPHA);
	inline bool enableAlphaTesting(bool p_enable);


	IDirect3D9*				getD3D()		{ return m_pD3D;		}
	IDirect3DDevice9*		getD3DDevice()	{ return m_pD3DDevice;	}
	ID3DXSprite*			getSprite()		{ return m_pSprite;		}
	D3DPRESENT_PARAMETERS&	getD3DPP()		{ return m_d3dpp;		}

protected:
	HWND					m_hWnd;
	IDirect3D9*				m_pD3D;
	IDirect3DDevice9*		m_pD3DDevice;
	ID3DXSprite*			m_pSprite;

	D3DDISPLAYMODE			m_d3ddm;
	D3DPRESENT_PARAMETERS	m_d3dpp;

	bool					m_windowed;

	long					m_width;
	long					m_height;

	char					m_BPP;
	//char					m_ambientRed;
	//char					m_ambientGreen;
	//char					m_ambientBlue;

};


#endif