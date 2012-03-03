#ifndef		__HERO_H__
#define		__HERO_H__

#include "GlobalDef.h"
#include "GameObject.h"
#include "StateMachine.h"
#include "Engine/Texture.h"

class cMap;
class cBall;

class cHero
	:public cGameObject
{
public:
// 	cHero()
// 		:m_pStateMachine(this){}
	virtual	~cHero(){}
	cHero(const sPosition& p_pos);

	virtual	void	Spell() = 0;
	virtual	bool	CanSpell();
	virtual bool	CanMove();
	virtual void	setAction(int p_action);

	void	SetSpellObject(cHero* P_pHero) { m_spellObject = P_pHero; }
	virtual bool	CanBeSpelled();
	cTexture*		GetTexture();

	int		GetTextureX();
	int		GetTextureY();
	
	cStateMachine<cHero>*	GetStateMachine() { return &m_pStateMachine; }
	void	PassBall(sPosition &p_pPos);
	bool	HaveGhostBall();
	bool	HaveGoldBall();
	void	Update();
	void	Render();
	bool	SnatchBall(int p_Type);
	void	LoseBall();
	void	GetBall(cBall *p_pBall, bool p_bGoldBall);
	double  GetLength(sPosition pos1, sPosition pos2);
	int		GetMaxBlue() { return m_maxBlue; }
	int		GetCurrBlue() { return m_currBlue; }
	int		GetAbnormalType() { return m_AbnormalType; }
	bool	CanSnatchGhostBall();
	bool	CanSnatchGoldBall();
	int		GetSpellCost() { return m_SpellCost; }
	bool	IsSpelling();
	void	SetTeamName(PCTSTR pszName);
	PCTSTR	GetTeamName() { return m_teamName; }
	void	SetHeroName(PCTSTR pszName);
	PCTSTR	GetHeroName() { return m_szHeroName; }
	unsigned	GetSnatchSteps() { return m_SnatchInterval; }
	bool	CanPassBall();
	int		GetStaticSpellInterval() { return m_StaticSpellInterval; }
	int		GetSpellInterval() { return m_SpellInterval; }
	void    SetSpellInterval(int itvl) { m_SpellInterval = itvl; }
	void	SetCurrBlue(int blue) { m_currBlue = blue; }
	int		GetSpellDirection(cHero *pHeroTarget);


	friend	class cNormalState;
	friend	class cSpellingState;
	friend	class cSpelledByGinnyState;
	friend	class cSpelledByHermioneState;
	friend	class cSpelledByMalfoyState;
	friend	class cSpelledByRonState;
	friend  class cSpelledByFreeballState;
	friend  class cSpelledByHarryState;
	friend  class cSpelledByVoldemortState;

	unsigned	m_SnatchInterval;	

protected:
	cStateMachine<cHero>	m_pStateMachine;

	int			m_SpellInterval;
	int			m_StaticSpellInterval;
	int			m_currBlue;			//current blue
	int			m_maxBlue;			//max blue
	int			m_SpellCost;		//Spell cost

	cHero*		m_spellObject;		//temp sotrage for spell
	cBall*		m_pBall;			//if the hero owned a ball, then it will point to it,otherwise this member will be NULL

	int			m_currAbnormalStep;	//current steps of abnormal state

	int			m_AbnormalType;	// abnormal type
	int			m_currSpellStep;	//

	bool		m_bGoldBall;		// determine the ball type, if and only if the hero owns a ball

	TCHAR		m_teamName[TEAM_NAME_LEN];			// team name
	TCHAR		m_szHeroName[HERO_NAME_LEN];		// hero name

protected:
	virtual void CheckPosition();

};

#endif		__HERO_H__