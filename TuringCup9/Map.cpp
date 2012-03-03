#include "Map.h"
#include <tchar.h>

extern cGraphics* g_pGraphics;

cMap::cMap()
{
	Reset();
}

cMap::~cMap()
{
}

void cMap::display()
{
	m_map.draw( 0, 0, m_road, 0, 1024, 768 );
}

bool cMap::loadMap(cGraphics *p_pGraphics, unsigned p_transparent /*= 0*/)
{
	UNREFERENCED_PARAMETER(p_transparent);

	return m_map.load( p_pGraphics, _T("../res/pic/map.png"), 0, 2048, 1024);
}

void cMap::release()
{

}

void cMap::Move( int p_way )
{
	m_road -= p_way;
}