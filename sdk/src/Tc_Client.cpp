#include <Windows.h>
#include <tchar.h>
#include <math.h>

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include "../inc/TuringCup9ClientAPI.h"
#pragma comment(lib, "../lib/TuringCup9.lib")

#define CLIENT_EXPORT
#include "Tc_Client.h"

/*
	How to debug?

	Goto
	Project -. Properties --> Configuration Properties --> Debugging

	Copy "$(ProjectDir)bin\TuringCup9.exe" 
	to the Command field.(Without quotation marks)
*/

CLIENT_EXPORT_API void __stdcall Init()
{
	Tc_SetTeamName(_T("team name here"));
	
	Tc_ChooseHero(TC_MEMBER1,TC_HERO_HARRY,L"h1");
	Tc_ChooseHero(TC_MEMBER2,TC_HERO_GINNY,L"h2");
}

TC_Handle h1, h2;

CLIENT_EXPORT_API void __stdcall AI()
{
	//
	// TODO: add your AI here
	//
	h1 = Tc_GetHeroHandle(TC_MEMBER1);
	h2 = Tc_GetHeroHandle(TC_MEMBER2);

	if( Tc_CanBeSpelled(h1, h2) )
	{
		Tc_Stop(h1);
		Tc_Spell(h1, h2);
	}
	else
		Tc_Move(h1, TC_DIRECTION_BOTTOM);
}