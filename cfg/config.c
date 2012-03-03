#include <Windows.h>
#include <tchar.h>
#include "Config.h"
#include "crc32.h"

VOID ConfigInit()
{
	init_crc32_table();
}

DWORD ConfigSave(IN PCTSTR pszConfigFile, IN PGameConfig pConfig)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwBytesWritten = 0;
	DWORD dwStatus = ERROR_SUCCESS;
	BOOL bWriteStatus = FALSE;
	GameFileHeader FileHeader;

	if( pConfig == NULL || 
		pszConfigFile == NULL || pszConfigFile[0] == TEXT('\0') )
	{
		dwStatus = ERROR_SUCCESS;
		goto __EXIT;
	}

	hFile = CreateFile(pszConfigFile, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		dwStatus = GetLastError();
		goto __EXIT;
	}
	
	// Write file
	SetFilePointer(hFile, sizeof(GameFileHeader), NULL, FILE_BEGIN);
	bWriteStatus = WriteFile(hFile, pConfig, sizeof(GameConfig), &dwBytesWritten, NULL);
	if( !bWriteStatus || dwBytesWritten != sizeof(GameConfig) )
	{
		dwStatus = GetLastError();
		goto __EXIT;
	}

	FileHeader.CheckSum = crc32((PCHAR)pConfig, sizeof(GameConfig));	
	FileHeader.Magic = GAME_FILE_HEADER_MAGIC;
	FileHeader.Version = GAME_FILE_HEADER_VERSION;

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	bWriteStatus = WriteFile(hFile, &FileHeader, sizeof(GameFileHeader), &dwBytesWritten, NULL);
	if( !bWriteStatus || dwBytesWritten !=  sizeof(GameFileHeader) )
	{
		dwStatus = GetLastError();
		goto __EXIT;
	}

	CloseHandle(hFile);

__EXIT:
	return dwStatus;
}

DWORD ConfigLoad(IN PCTSTR pszConfigFile, OUT PGameConfig pConfig, DWORD dwLength)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwBytesRead = 0;
	DWORD dwStatus = ERROR_SUCCESS;
	BOOL bReadStatus = FALSE;
	GameFileHeader FileHeader;
	GameConfig Config;

	if( pConfig == NULL ||
		pszConfigFile == NULL || pszConfigFile[0] == TEXT('\0') )
	{
		dwStatus = ERROR_INVALID_PARAMETER;
		goto __EXIT;
	}

	hFile = CreateFile(pszConfigFile, FILE_READ_ACCESS, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		dwStatus = GetLastError();
		goto __EXIT;
	}
	
	bReadStatus = ReadFile(hFile, &FileHeader, sizeof(GameFileHeader), &dwBytesRead, NULL);
	if( !bReadStatus || dwBytesRead != sizeof(GameFileHeader) )
	{
		dwStatus = GetLastError();
		goto __EXIT;
	}	
	bReadStatus = ReadFile(hFile, &Config, sizeof(GameConfig), &dwBytesRead, NULL);
	if( !bReadStatus || dwBytesRead != sizeof(GameConfig) )
	{
		dwStatus = GetLastError();
		goto __EXIT;
	}
	CloseHandle(hFile);

	// Check file header
	if( FileHeader.Magic != GAME_FILE_HEADER_MAGIC ||
		FileHeader.Version != GAME_FILE_HEADER_VERSION )
	{
		dwStatus = ERROR_INVALID_DATA;
		goto __EXIT;
	}

	// Check CRC32
	if( FileHeader.CheckSum != crc32((PCHAR)&Config, sizeof(GameConfig)) )
	{
		dwStatus = ERROR_INVALID_IMAGE_HASH;
		goto __EXIT;
	}

	// Copy data
	if( dwLength < sizeof(GameConfig) )
	{
		dwStatus = ERROR_INSUFFICIENT_BUFFER;
		goto __EXIT;
	}
	memcpy_s(pConfig, dwLength, &Config, sizeof(GameConfig));

__EXIT:
	return dwStatus;
}
