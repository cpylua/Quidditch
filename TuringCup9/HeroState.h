#pragma once
#include "state.h"
#include "GlobalDef.h"

class cHero;

template<typename Entity>
class cHeroState :
	public cState<cHero>
{
public:
};

//Normal State
class cNormalState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cNormalState*	Instance()
	{
		static cNormalState s_normalState;
		return &s_normalState;
	}
};

//Spelling State
class cSpellingState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cSpellingState*	Instance()
	{
		static cSpellingState s_normalState;
		return &s_normalState;
	}

private:
	int m_BottomIndex;
	int m_HandIndex;
};

//Spelled by Ron State
class cSpelledByRonState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cSpelledByRonState*	Instance()
	{
		static cSpelledByRonState s_ronState;
		return &s_ronState;
	}
};

//Spelled by Hermione State
class cSpelledByHermioneState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cSpelledByHermioneState*	Instance()
	{
		static cSpelledByHermioneState s_hermioneState;
		return &s_hermioneState;
	}

private:
	sSpeed m_OldSpeed;
};

//Spelled by Malfoy State
class cSpelledByMalfoyState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cSpelledByMalfoyState*	Instance()
	{
		static cSpelledByMalfoyState s_malfoyState;
		return &s_malfoyState;
	}

private:
	int m_BangPosY;
	int m_BangPosX;
};

//Spelled by Ginny State
class cSpelledByGinnyState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cSpelledByGinnyState*	Instance()
	{
		static cSpelledByGinnyState s_ginnyState;
		return &s_ginnyState;
	}
};

//Spelled by Freeball State
class cSpelledByFreeballState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cSpelledByFreeballState*	Instance()
	{
		static cSpelledByFreeballState s_FreeballState;
		return &s_FreeballState;
	}
};

// Spelled by Harry state
class cSpelledByHarryState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cSpelledByHarryState*	Instance()
	{
		static cSpelledByHarryState s_HarryState;
		return &s_HarryState;
	}
};

// Spelled by voldemort state
class cSpelledByVoldemortState :
	public cHeroState<cHero>
{
public:
	virtual	void	Enter(cHero* p_pHero);
	virtual	void	Update(cHero* p_pHero);
	virtual	void	Exit(cHero* p_pHero);

	virtual	void	Render(cHero* p_pHero);

	static	cSpelledByVoldemortState*	Instance()
	{
		static cSpelledByVoldemortState s_VoldemortState;
		return &s_VoldemortState;
	}

private:
	int m_Glint;

};