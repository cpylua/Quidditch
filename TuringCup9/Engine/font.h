/*--------------------------------------------------------------------
 *	Author:		Altfee
 *	Created:	2008/8/22
 *	File:		Font.h
 *--------------------------------------------------------------------*/

#ifndef		__FONT_H__
#define		__FONT_H__

#include <D3dx9core.h>
#include "Core_GlobalDef.h"
#include "Graphics.h"

class cFont
{
public:
	cFont();
	~cFont();

	/*************************************
		创建字体：
			parameters：
				p_name：要创建的字体名字;
				p_width：字体宽度;
				p_height：字体高度;
				p_italic：是否斜体;
				p_underLine：是否有下划线 
	**************************************/
	bool	create(cGraphics* p_pGraphics, const TCHAR* p_name,
					long p_width = 0, int p_height = 0,
					long p_bold = 0, bool p_italic = false );
	void	destroy();
	bool	print(const TCHAR* p_text, long p_XPos, long p_YPos,
					long p_width = 0, long p_height = 0, 
					D3DCOLOR p_color = 0xffffffff,
					unsigned p_format = 0);

	ID3DXFont*	getFontInterface() { return m_pFont; }

	
private:
	ID3DXFont*	m_pFont;
};
#endif