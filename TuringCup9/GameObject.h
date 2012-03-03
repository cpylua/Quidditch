#ifndef		__GAMEOBJECT_H__
#define		__GAMEOBJECT_H__

#include "GlobalDef.h"
#include <windows.h>

class cMap;

class cGameObject
{
public:
	cGameObject();

	virtual ~cGameObject(){};

	const sPosition&	getPosition()		{ return m_position; }
	sPosition			getScreenPosition();
	int					getCurrWidth()		{ return m_currWidth; }
	int					getCurrHeight()		{ return m_currHeight; }
	const unsigned		getType() const		{ return m_objectType; }
	bool				collide(cGameObject& p_obj);
	void				SetPos(int p_x, int p_y);
	void				GetPos(sPosition &p_pos);
	sSpeed				GetSpeed() { return m_speed; }
	void				SetSpeed(sSpeed &speed) { m_speed.vx = speed.vx; m_speed.vy = speed.vy; }


	virtual	int			getAction() { return m_currAction; }
	virtual	void		move();		//default movement
	virtual	void		move(int p_offsetX, int p_offsetY);	//specified movement

	int					m_objectType;


protected:
	unsigned			m_currFrame;		//current frame index
	int					m_lastFrameTime;		//time that last change m_currFrame

	sPosition			m_position;			//coordinate of the left-up conner of object
	int					m_currWidth;		//current width of the object
	int					m_currHeight;		//current height of the object
	sSpeed				m_speed;			//speed of the object
	int					m_currAction;
	int					m_prevAction;
	static	cMap*		s_pMap;		//global map

	virtual void CheckPosition() = 0;
};

#endif