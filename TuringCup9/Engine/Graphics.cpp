/*--------------------------------------------------------------------
 *	Author:		Altfee
 *	Created:	2008/8/14
 *	File:		Graphics.cpp
 *--------------------------------------------------------------------*/

#include "Graphics.h"


cGraphics::cGraphics()
{
	m_hWnd		 = NULL;
	m_pD3D		 = NULL;
	m_pD3DDevice = NULL;
	m_pSprite	 = NULL;

	//m_ambientRed   = (char)255;
	//m_ambientGreen = (char)255;
	//m_ambientBlue  = (char)255;

	m_width	 = 0;
	m_height = 0;
	m_BPP	 = 0;

	m_windowed = true;
	//m_HAL	   = true;
}

cGraphics::~cGraphics()
{
	destroy();
}

bool cGraphics::destroy()
{
	SAFE_RELEASE(m_pSprite)
	SAFE_RELEASE(m_pD3DDevice)
	SAFE_RELEASE(m_pD3D)
	return true;
}

bool cGraphics::init()
{
	destroy();

	if( ( m_pD3D = Direct3DCreate9(D3D_SDK_VERSION) ) == NULL )
		return false;

	return true;
}

/*设置显示模式*/
bool cGraphics::setMode( HWND p_hWnd, bool p_windowed, /*= true*/
						long p_width, /*= 0*/ long p_height, /*= 0*/ 
						char p_BBP /*= 0*/ )
{
	//D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );

	if( ( m_hWnd = p_hWnd) == NULL )
		return false;
	if( ( m_pD3D == NULL ) )
		return false;
	
	if( FAILED( m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_d3ddm) ) )
		return false;
	m_windowed = p_windowed;
	if( !m_windowed )
	{
		m_width = m_d3ddm.Width;
		m_height = m_d3ddm.Height;
	}
	else
	{
		m_width	 = p_width;
		m_height = p_height;
	}
	
	m_d3dpp.BackBufferWidth  = m_width;
	m_d3dpp.BackBufferHeight = m_height;
	if ( (m_BPP = p_BBP) == 0 || p_windowed == true )
	{
		if( (m_BPP = getFormatBBP(m_d3ddm.Format)) == 0  )
			return false;
	}

	m_d3dpp.Windowed = m_windowed;
	if ( m_windowed )
	{
		m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		m_d3dpp.BackBufferFormat = m_d3ddm.Format;
	} 
	else
	{
		m_d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
		
		D3DFORMAT format = D3DFMT_X8R8G8B8;
		D3DFORMAT altFormat = D3DFMT_X8R8G8B8;

		switch (m_BPP)
		{
		case 32:
			format    = D3DFMT_X8R8G8B8;
			altFormat = D3DFMT_X8R8G8B8;
			break;
		case 24:
			format    = D3DFMT_R8G8B8;
			altFormat = D3DFMT_R8G8B8;
			break;
		case 16:
			format    = D3DFMT_R5G6B5;
			altFormat = D3DFMT_X1R5G5B5;
			break;
		case 8:
			format    = D3DFMT_P8;
			altFormat = D3DFMT_P8;
			break;
		}
		
		if ( !checkFormat(format, m_windowed) )
		{
			if ( !checkFormat(altFormat, m_windowed) )
				return false;
			format = altFormat;
		}
		m_d3dpp.BackBufferFormat = format;
	}
	
	m_d3dpp.PresentationInterval = 0;//D3DPRESENT_INTERVAL_IMMEDIATE;
	if ( FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&m_d3dpp, &m_pD3DDevice)) )
	{
		if ( FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&m_d3dpp, &m_pD3DDevice)) )
		{
			return false;
		}
	}
	
	
	enableAlphaBlending(FALSE);
	enableAlphaTesting(FALSE);
	
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	if(FAILED(D3DXCreateSprite(m_pD3DDevice, &m_pSprite)))
		return FALSE;

	return true;
}

char cGraphics::getFormatBBP( D3DFORMAT p_format )
{
	switch(p_format) {
    case D3DFMT_A8R8G8B8:	case D3DFMT_X8R8G8B8:
		return 32;
		break;
		
    case D3DFMT_R8G8B8:
		return 24;
		break;
		
    case D3DFMT_R5G6B5:    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:    case D3DFMT_A4R4G4B4:
		return 16;
		break;
		
    case D3DFMT_A8P8:    case D3DFMT_P8:
		return 8;
		break;
		
    default:
		return 0;
	}
}

bool cGraphics::checkFormat( D3DFORMAT p_format, bool p_windowed )
{
	if(FAILED(m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL,	p_format, p_format, p_windowed)))
		return false;
	
	return true;
}
/*********************有待检验*****************************************/
bool cGraphics::enableAlphaBlending( bool p_enable /*= true*/,
									 long p_src /*= D3DBLEND_SRCALPHA*/,
									 long p_dest /*= D3DBLEND_INVSRCALPHA*/ )
{
	if( m_pD3DDevice == NULL )
		return false;
	if( FAILED(m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, p_enable)) )
		return false;

	if ( p_enable )
	{
		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, p_src );
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, p_dest );
	}
	return true;
}
/*********************有待检验*****************************************/
bool cGraphics::enableAlphaTesting( bool p_enable )
{
	if( m_pD3DDevice == NULL )
		return false;
	if( FAILED(m_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, p_enable )) )
		return false;

	if ( p_enable )
	{
		m_pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 0x01 );
		m_pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	}
	return true;
}

bool cGraphics::beginScene()
{
	if( m_pD3DDevice == NULL )
		return false;

	if( FAILED(m_pD3DDevice->BeginScene()) )
		return false;

	return true;
}

bool cGraphics::endScene()
{
	if( m_pD3DDevice == NULL )
		return false;
	if( FAILED(m_pD3DDevice->EndScene()) )
		return false;

	/*for(i=0;i<8;i++)
		m_pD3DDevice->SetTexture(i, NULL);*/
	return true;
}

bool cGraphics::clearDisplay( long color /*= 0*/ )
{
	if( m_pD3DDevice == NULL )
		return false;
	if( FAILED(m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0 )) )
		return false;

	return true;
}
//
//bool cGraphics::setAmbientLight( char p_red, char p_green, char p_blue )
//{
//	if( m_pD3DDevice == NULL )
//		return false;
//
//	D3DCOLOR color;
//	color = D3DCOLOR_XRGB( (m_ambientRed = p_red), 
//		(m_ambientGreen = p_green), (m_ambientBlue = p_blue) );
//
//	if( FAILED(m_pD3DDevice->SetRenderState(D3DRS_AMBIENT, color)) )
//		return false;
//
//	return true;
//}
//
//bool cGraphics::getAmbientLight( char* p_pRed, char* p_pGreen, char* p_pBlue )
//{
//	if( p_pRed )
//		*p_pRed = m_ambientRed;
//	if( p_pGreen )
//		*p_pGreen = m_ambientGreen;
//	if( p_pBlue )
//		*p_pBlue = m_ambientBlue;
//}

bool cGraphics::display()
{
	if( m_pD3DDevice == NULL )
		return false;
	
	if( FAILED(m_pD3DDevice->Present(NULL, NULL, NULL, NULL)) )
		return false;


	return true;
}

bool cGraphics::TestDevice()
{
	switch ( (getD3DDevice())->TestCooperativeLevel() )
	{
	case D3D_OK:
		return true;
	case D3DERR_DEVICELOST:
		return false;
	case D3DERR_DEVICENOTRESET:
		//ERRORBOX("D3DERR_DEVICENOTRESET");
		getSprite()->OnLostDevice();
		(getD3DDevice())->Reset(&getD3DPP());
		return true;
	default:
		//PostQuitMessage(0);
		break;
	}

	return true;
}