#pragma once

#include "State.h"
#include "typeinfo"

template<typename Entity>
class cStateMachine
{
public:
	cStateMachine(Entity* p_pOwner)
		:m_pOwner(p_pOwner),
		 m_pCurrState(NULL){}

	void	Render()
	{
		m_pCurrState->Render(m_pOwner);
	}

	void	Update()
	{
		if( m_pCurrState )
			m_pCurrState->Update(m_pOwner);
	}

	void	ChangeState(cState<Entity>* p_pNewState)
	{
		m_pCurrState->Exit(m_pOwner);
		m_pCurrState = p_pNewState;
		m_pCurrState->Enter(m_pOwner);
	}

	bool	IsInState(const cState<Entity>* p_pState)
	{
		return typeid(*p_pState) == typeid(*m_pCurrState);
	}

	void	SetCurrentState(cState<Entity>* p_pState) { m_pCurrState = p_pState; }

	void	HandleMessage()
	{
		m_pCurrState->HandleMessage();
	}

private:
	Entity*				m_pOwner;
	cState<Entity>*		m_pCurrState;
};