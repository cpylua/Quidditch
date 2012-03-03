/*--------------------------------------------------------------------
 *	Author:		Altfee
 *	Created:	2008/8/18
 *	File:		Texture.h
 *--------------------------------------------------------------------*/


#ifndef		__TEXTURE_H__
#define		__TEXTURE_H__


#include "d3d9.h"
#include "d3dx9.h"
#pragma comment(lib, "d3dx9.lib")

class cGraphics;

class cTexture 
{
public:
	cTexture();
	~cTexture();

	/*bool	create(cGraphics* p_pGraphics, 
					unsigned p_width, unsigned p_height, D3DFORMAT p_format);
	
	bool	create(cGraphics* p_pGraphics, IDirect3DTexture9* p_pTexture);*/
	
	bool	load(cGraphics *p_pGraphics, 
				TCHAR * p_fileName,
				unsigned p_transparent = 0,
				unsigned p_width = D3DX_DEFAULT, 
				unsigned p_height = D3DX_DEFAULT,
				D3DFORMAT p_format = D3DFMT_UNKNOWN);

	BOOL	loadFromResource(cGraphics *p_pGraphics, 
						TCHAR* pSrcResource, 
						unsigned p_transparent = 0,
						unsigned p_width = D3DX_DEFAULT,
						unsigned p_height = D3DX_DEFAULT,
						D3DFORMAT p_format = D3DFMT_UNKNOWN);
	
	bool	free();
	bool	isLoaded();


	bool	draw(long p_destX, long p_destY, 
				long p_srcX  = 0, long p_srcY   = 0,
				long p_width = 0, long p_height = 0,
				D3DCOLOR p_color = 0xffffffff,
				D3DXVECTOR2 *p_pRotationCenter = NULL,
				float p_angle = 0.0f,
				float p_XScale = 1.0f, float p_YScale = 1.0f);
	
	D3DFORMAT			getFormat()			  { return m_D3DFormat;	}
	unsigned			getWidth()			  { return m_width;		}
	unsigned			getHeight()			  { return m_height;	}
	IDirect3DTexture9*	getTextureInterface() { return m_pTexture;	}

private:
	cGraphics*			m_pGraphics;
	IDirect3DTexture9*	m_pTexture;
	unsigned			m_width;
	unsigned			m_height;
	D3DFORMAT			m_D3DFormat;
};


#endif