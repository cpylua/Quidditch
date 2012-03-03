/*--------------------------------------------------------------------
*	Author:		Altfee
*	Created:	2008/8/26
*	File:		Core_GlobalDef.h
*--------------------------------------------------------------------*/

#ifndef		__COREGLOBALDEF_H__
#define		__COREGLOBALDEF_H__

#include <Windows.h>
#include <tchar.h>

#define SAFE_RELEASE(x)		if(x) { x->Release(); x = NULL; } 

#endif