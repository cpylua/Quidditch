#include <windows.h>
#include <tchar.h>
#include "Config.h"
#include "crc32.h"

__inline void Usage()
{
	_tprintf(TEXT("%s\n"), 
		TEXT("DumpCfg\tzombie.fml\n")
		TEXT("Usage: dumpcfg filename\n"));
}

__inline void PrintMana(PCTSTR pszName, PGameMana pMana)
{
	_tprintf(TEXT("%s:\n\t")
		TEXT("SpelleCost = %d\n\t")
		TEXT("SpellInterval = %d\n\t")
		TEXT("MaxMana = %d\n"),
		pszName, pMana->SpellCost, pMana->SpellInterval, pMana->MaxMana);
}

__inline void PrintRange(PCTSTR pszName, PGameRange pRange)
{
	_tprintf(TEXT("%s\n\t")
		TEXT("Min = %d\n\t")
		TEXT("Max = %d\n"),
		pszName, pRange->Min, pRange->Max);
}

int _tmain(int argc, TCHAR *argv[])
{
	GameConfig Config;
	DWORD dwStatus;

	if( argc != 2 )
	{
		Usage();
		return -1;
	}
	
	ConfigInit();
	dwStatus = ConfigLoad(argv[1], &Config, sizeof(GameConfig));
	if( dwStatus != ERROR_SUCCESS )
	{
		_tprintf(TEXT("%s"), TEXT("Failed to load config file.\n"));
		return (int)dwStatus;
	}

	_tprintf(TEXT("GametTime = %d\n"), Config.GameTime);

	PrintMana(TEXT("Ron"), &Config.Ron);
	PrintMana(TEXT("Hermione"), &Config.Hermione);
	PrintMana(TEXT("Malfoy"), &Config.Malfoy);
	PrintMana(TEXT("Ginny"), &Config.Ginny);
	PrintMana(TEXT("Harry"), &Config.Harry);
	PrintMana(TEXT("Voldemort"), &Config.Voldemort);

	_tprintf(TEXT("SpelledByRonTime = %d\n"), Config.SpelledByRonTime);
	_tprintf(TEXT("SpelledByGinnyTime = %d\n"), Config.SpelledByGinnyTime);
	_tprintf(TEXT("SpelledByHarryTime = %d\n"), Config.SpelledByHarryTime);
	_tprintf(TEXT("SpelledByHermioneTime = %d\n"), Config.SpelledByHermioneTime);
	_tprintf(TEXT("SpelledByMalfoyTime = %d\n"), Config.SpelledByMalfoyTime);
	_tprintf(TEXT("SpelledByFreeBallTime = %d\n"), Config.SpelledByFreeBallTime);
	_tprintf(TEXT("SpelledByVoldemortTime = %d\n"), Config.SpelledByVoldemortTime);
	_tprintf(TEXT("SpelledByVoldemortDelta = %d\n"), Config.SpelledByVoldemortDelta);

	//_tprintf(TEXT("SnatchInterval = %u\n"), Config.SnatchInterval);

	PrintRange(TEXT("FreeballShow"), &Config.FreeballShowTime);
	PrintRange(TEXT("FreeballHide"), &Config.FreeballHideTime);
	_tprintf(TEXT("Freeball Speed = %d\n"), Config.FreeballSpeed);

	_tprintf(TEXT("Ghostball Speed = %d\n"), Config.GhostballSpeed);

	_tprintf(TEXT("GoldBall Speed = %d\n"), Config.GoldBallSpeed);
	_tprintf(TEXT("GoldBall Show time = %d\n"), Config.GoldBallShowTime);
	_tprintf(TEXT("GoldBall slow-down steps = %d\n"), Config.GoldBallSlowDownSteps);

	return 0;
}