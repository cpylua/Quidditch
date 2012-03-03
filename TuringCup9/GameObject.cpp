#include "GameObject.h"
#include "Game.h"
#include "Map.h"

cMap* cGameObject::s_pMap;


cGameObject::cGameObject()
{
	s_pMap = cGame::GetResourceManager()->getMap();

}

void cGameObject::move( int offsetX, int offsetY )
{
	m_position.x += offsetX;
	m_position.y += offsetY;

	CheckPosition();
}

void cGameObject::move()
{
	m_position.x += m_speed.vx;
	m_position.y += m_speed.vy;

	CheckPosition();
}

bool cGameObject::collide( cGameObject& p_obj )
{
	return p_obj.getPosition().x < m_position.x + m_currWidth 
		&& p_obj.getPosition().x > m_position.x - p_obj.getCurrWidth()
		&& p_obj.getPosition().y < m_position.y + m_currHeight
		&& p_obj.getPosition().y > m_position.y - p_obj.getCurrHeight();
}

sPosition cGameObject::getScreenPosition()
{
	return sPosition( m_position.x - s_pMap->GetRoad(), m_position.y );
}

void cGameObject::SetPos( int p_x, int p_y )
{
	m_position.x = p_x;
	m_position.y = p_y;
}

void cGameObject::GetPos( sPosition &p_pos )
{
	p_pos.x = m_position.x;
	p_pos.y = m_position.y;
}