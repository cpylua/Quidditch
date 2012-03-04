#pragma once

#ifdef CLIENT_EXPORT
/*
	CLIENT_EXPORT should be defined in all the DLL's source
	code modules before this header file is included
*/
#define CLIENT_EXPORT_API __declspec(dllexport)
#endif

CLIENT_EXPORT_API void __stdcall Init();

CLIENT_EXPORT_API void __stdcall AI();