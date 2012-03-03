#include <Windows.h>
#include <tchar.h>
#include "config.h"

int _tmain()
{
	GameConfig config;
	DWORD dwStatus;

	config.GameTime = 60000;
	
	// balls
	config.FreeballHideTime.Max = 30;
	config.FreeballHideTime.Min = 20;
	config.FreeballShowTime.Max = 150;
	config.FreeballShowTime.Min = 100;
	config.FreeballSpeed = 12;

	config.GhostballSpeed = 12;

	config.GoldBallSpeed = 12;
	config.GoldBallShowTime = 50000;
	config.GoldBallSlowDownSteps = 100;
	
	// heros
	config.Ginny.MaxMana = 5500;
	config.Ginny.SpellCost = 2500;
	config.Ginny.SpellInterval = 300;

	config.Harry.MaxMana = 5500;
	config.Harry.SpellCost = 2500;
	config.Harry.SpellInterval = 300;

	config.Hermione.MaxMana = 5500;
	config.Hermione.SpellCost = 2500;
	config.Hermione.SpellInterval = 500;

	config.Malfoy.MaxMana = 5500;
	config.Malfoy.SpellCost = 2500;
	config.Malfoy.SpellInterval = 500;
	
	config.Ron.MaxMana = 4500;
	config.Ron.SpellCost = 2000;
	config.Ron.SpellInterval = 350;

	config.Voldemort.MaxMana = 5500;
	config.Voldemort.SpellCost = 2500;
	config.Voldemort.SpellInterval = 300;

	config.SpelledByRonTime = 30;
	config.SpelledByGinnyTime = 30;
	config.SpelledByHarryTime = 30;
	config.SpelledByFreeBallTime = 50;
	config.SpelledByHermioneTime = 320;
	config.SpelledByMalfoyTime = 180;
	config.SpelledByVoldemortTime = 100;
		
	config.SpelledByVoldemortDelta = 50;
	
	ConfigInit();
	dwStatus = ConfigSave(TEXT("config.cfg"), &config);

	if( dwStatus != ERROR_SUCCESS )
	{
		_tprintf(TEXT("%s\n"), TEXT("Failed to save config file."));
		return -1;
	}

	return 0;
}

