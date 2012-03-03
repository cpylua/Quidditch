/*
	@author: zombie.fml

	@histroy:
		11/6/2009	create file

		11/7/2009	add harry and voldemort

	@description:
		This file defines the structure of the config file format
*/

#pragma once

#pragma pack(push)
#pragma pack(1)


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GameMana_tag
{
	int SpellCost;
	int SpellInterval;
	int MaxMana;
} GameMana, *PGameMana;

typedef struct _GameRange_tag
{
	int Min;
	int Max;
} GameRange, *PGameRange;

typedef struct _GameConfig_tag
{
	unsigned GameTime;
	
	GameMana Ron;
	GameMana Hermione;
	GameMana Malfoy;
	GameMana Ginny;
	GameMana Harry;
	GameMana Voldemort;
	
	int SpelledByRonTime;
	int SpelledByHarryTime;
	int SpelledByGinnyTime;
	int SpelledByHermioneTime;
	int SpelledByMalfoyTime;
	int SpelledByFreeBallTime;
	int SpelledByVoldemortTime;
	int SpelledByVoldemortDelta;
	
	GameRange FreeballShowTime;
	GameRange FreeballHideTime;
	int FreeballSpeed;
	
	int GhostballSpeed;
	
	int GoldBallShowTime;
	int GoldBallSlowDownSteps;
	int GoldBallSpeed;

} GameConfig, *PGameConfig;

#define GAME_FILE_HEADER_MAGIC 0x1A462E5A
#define GAME_FILE_HEADER_VERSION	1

typedef struct _GameFileHeader_tag
{
	unsigned Magic;			// must be 0x1A462E5A
	unsigned Version;
	unsigned long CheckSum; 
} GameFileHeader, *PGameFileHeader;

#pragma pack(pop)

VOID ConfigInit();
DWORD ConfigSave(IN PCTSTR pszConfigFile, IN PGameConfig pConfig);
DWORD ConfigLoad(IN PCTSTR pszConfigFile, OUT PGameConfig pConfig, DWORD dwLength);

#ifdef __cplusplus
}
#endif

