/*--------------------------------------------------------------------
 *	Author:		Altfee
 *	Created:	2008/8/22
 *	File:		Font.cpp
 *--------------------------------------------------------------------*/

#include "Font.h"


cFont::cFont()
{
	m_pFont = NULL;
}

cFont::~cFont()
{
	destroy();
}

void cFont::destroy()
{
	SAFE_RELEASE(m_pFont)
}

bool cFont::create( cGraphics* p_pGraphics, const TCHAR* p_name, 
				   long p_width /*= 0*/, int p_height /*= 0*/, 
				   long p_bold /*= 0*/, bool p_italic /*= false*/ )
{
	if( FAILED( D3DXCreateFont( p_pGraphics->getD3DDevice(), p_height, p_width,
		p_bold, 0, p_italic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY, 0, p_name, &m_pFont ) ) )
	{
		return false;
	}
	return  true;
}

bool cFont::print( const TCHAR* p_text, long p_XPos, long p_YPos, 
				  long p_width, long p_height, 
				  D3DCOLOR p_color /*= 0xffffffff*/, unsigned p_format /*= 0*/ )
{
	if( m_pFont == NULL )
		return false;

	if( p_width == 0 )
		p_width = USHRT_MAX;
	if( p_height == 0 )
		p_height = USHRT_MAX;

	RECT rect;
	rect.left	= p_XPos;
	rect.top	= p_YPos;
	rect.right	= p_XPos + p_width;
	rect.bottom	= p_YPos + p_height;

	if( FAILED(m_pFont->DrawText(NULL, p_text, -1, &rect, p_format, p_color)) )
		return false;
	return true;
}