#pragma once

template<typename Entity>
class cState
{
public:
	virtual ~cState(){};
	virtual	void	Enter(Entity*)	= 0;
	virtual	void	Update(Entity*) = 0;
	virtual	void	Exit(Entity*)	= 0;

	virtual	void	Render(Entity*)	= 0;

	virtual	void	HandleMessage(){}

protected:
	cState(){}
};