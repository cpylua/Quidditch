#ifndef		__MAP_H__
#define		__MAP_H__

#include "Engine/Graphics.h"
#include "Engine/Texture.h"
#include "GlobalDef.h"
#include <tchar.h>

class cMap
{
public:
	cMap();
	~cMap();
	void		release();

	void		display();

	void		Move(int p_way);
	int			GetRoad() { return m_road; }
	void		SetRoad(int p_road) { m_road = p_road; }
	bool		loadMap(cGraphics *p_pGraphics, unsigned p_transparent = 0);
	void		Reset() { m_road = GAME_WINDOW_WIDTH / 2; }

private:
	cTexture		m_map;
	int				m_road;
	int				m_fullWidth;
	int				m_fullHeight;
};

#endif