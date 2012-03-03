#include <Windows.h>
#include "resource.h"
#include "TuringCup9Server.h"
#include "Config.h"

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' \
version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "Winmm.lib")

int WINAPI WinMain
(
	 HINSTANCE hInstance,      // handle to current instance
	 HINSTANCE hPrevInstance,  // handle to previous instance
	 LPSTR lpCmdLine,          // command line
	 int nCmdShow              // show state
 )
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	timeBeginPeriod(1);

	ConfigInit();

	cTuringCup9ServerApp().run();

	timeEndPeriod(1);
}