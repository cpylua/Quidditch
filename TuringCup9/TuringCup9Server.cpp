/*
	@author: zombie.fml
*/


#include <windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include <assert.h>
#include <process.h>
#include <tchar.h>
#include <ctime>
#include "TuringCup9Server.h"
#include "TuringCup9ClientAPI.h"
#include "Game.h"
#include "resource.h"
#include <StrSafe.h>

using namespace std;


extern	bool	g_QuitCurrentGame;
extern  BOOL	g_bExitThread[AI_NUM];
extern	int		g_GameMode1[2];
extern	int		g_GameMode2[2];

extern int g_AI_HeroType[AI_NUM * 2];		// half by half

bool g_bUseOvertimeConfig = false;

void ChooseMode();

bool g_GameStart = false;
extern	bool g_AfterReady;

PTSTR g_pszMsg = NULL;
TCHAR g_szAIPath1[MAX_PATH], g_szAIPath2[MAX_PATH];

HANDLE g_hEventNewGame;
HANDLE g_hThreadNewGame;
BOOL g_bExitGameThread;

RECT g_rcOld;

/*
	if(all client dlls are loaded and we got AIs' addresses)
	{
		if(g_pfnAI[index] == NULL)
			this team is keyboard mode
		else 
			this team is AI mode, g_pfnAI[index] points to the AI's addr
	}
	else
	{
		something bad happened, you should not start the game
	}
*/

PFN_AI g_pfnAI[AI_NUM] = {0};
PFN_INIT g_pfnInit[AI_NUM] = {0};

#define MSG_BUF_SIZE	512

cTuringCup9ServerApp::cTuringCup9ServerApp()
{
	g_pApp  = this;
	m_hInst = GetModuleHandle(NULL);

	StringCchCopy( m_szClassName, MAX_PATH, TEXT("TuringServer") );
	StringCchCopy( m_szCaptionName, MAX_PATH, TEXT("TuringCup 9") );

	m_hCursor	= LoadCursor( NULL, IDC_ARROW );			//set cursor
	m_hIcon	= LoadIcon( m_hInst, MAKEINTRESOURCE(IDI_TC9) );		//set icon
	m_style		= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	m_xPos		= 400;
	m_yPos		= 150;
	m_width		= 400;
	m_height	= 250;

	m_wcex.cbSize				= sizeof(WNDCLASSEX);
	m_wcex.style				= CS_CLASSDC;
	m_wcex.lpfnWndProc			= AppWindowProc;
	m_wcex.cbClsExtra			= 0;
	m_wcex.cbWndExtra			= 0;
	m_wcex.hInstance			= m_hInst;
	m_wcex.hIcon				= m_hIcon;
	m_wcex.hCursor				= m_hCursor;
	m_wcex.hbrBackground		= NULL;
	m_wcex.lpszMenuName			= NULL;
	m_wcex.lpszClassName		= m_szClassName;
	m_wcex.hIconSm				= m_hIcon;
	srand( (unsigned)time(NULL) );

	m_dwLastFrame = 0;
}

cTuringCup9ServerApp::~cTuringCup9ServerApp()
{

}

LRESULT CALLBACK AppWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) 
	{
	case WM_CLOSE:
		__End();
		return 0;
		
    default: return g_pApp->msgProc(hWnd, uMsg, wParam, lParam);
	}
}

bool cTuringCup9ServerApp::run()
{
	cServerDlg serverDlg;
	serverDlg.create( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TURINGCUP9) );
	serverDlg.run();
	
	return true;
}

bool cTuringCup9ServerApp::moveWindow( long p_xPos, long p_yPos )
{
	RECT rect;
	GetClientRect( m_hWnd, &rect );

	m_xPos = p_xPos;
	m_yPos = p_yPos;

	MoveWindow( m_hWnd, m_xPos, m_yPos, rect.right, rect.bottom, true );

	return true;
}

bool cTuringCup9ServerApp::resizeClient( long p_width, long p_height )
{
	UNREFERENCED_PARAMETER(p_height);

	RECT windowRect,
		 clientRect;

	GetWindowRect( m_hWnd, &windowRect );
	GetClientRect( m_hWnd, &clientRect );

	m_width  = ( windowRect.right - windowRect.left ) 
				- ( clientRect.right - p_width );

	m_height = ( windowRect.bottom - windowRect.top ) 
				- ( clientRect.bottom - clientRect.top );

	MoveWindow( m_hWnd, windowRect.left, windowRect.top, m_width, m_height, true );
	return true;
}

bool cTuringCup9ServerApp::showMouse( bool p_show /*= true */)
{
	ShowCursor(p_show);
	return true;
}

void cTuringCup9ServerApp::createNewGame(HWND hwndDlg)
{
	if( !RegisterClassEx(&m_wcex) )
		PostQuitMessage(0);

	RECT rc;
	rc.bottom = GAME_WINDOW_HEIGHT;
	rc.right = GAME_WINDOW_WIDTH;
	rc.left = rc.top = 0;
	AdjustWindowRect(&rc, m_style, FALSE);

	m_hWnd = CreateWindow( 
		m_szClassName, m_szCaptionName,
		m_style, 
		GAME_SCREEN_WIDTH / 2 - GAME_WINDOW_WIDTH / 2,
		(GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYCAPTION) - GAME_WINDOW_HEIGHT) / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, m_hInst, NULL);

	//assert(m_hWnd != NULL);

	if(!m_hWnd)
		PostQuitMessage(0);

	CoInitialize(NULL);

	cGame game;

	game.Initial(m_hWnd, hwndDlg);
	SendMessage(hwndDlg, WM_LOAD_DONE, 0, 0);

	if( g_GameMode1[0] == GAME_MODE_AI &&
		g_GameMode2[0] == GAME_MODE_AI )
	{
		ShowWindow(hwndDlg, SW_HIDE);
	}
	SetForegroundWindow(m_hWnd);

	MSG msg;
	if( init() ) 
	{
		ShowWindow(m_hWnd, SW_NORMAL);
		UpdateWindow(m_hWnd);
		ZeroMemory( &msg, sizeof(MSG) );
		while( !g_QuitCurrentGame )
		{
			if(PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// limit the FPS
			if( GetTickCount() - m_dwLastFrame > 10 )
			{
				game.GameLoop();
				m_dwLastFrame = GetTickCount();
			}
			else
			{
				// sleep a bit
				Sleep(1);
			}
		}
	}

	game.CleanUp();
	CoUninitialize();
	DestroyWindow( m_hWnd );
	UnregisterClass( m_szClassName, m_hInst );

	ShowWindow(hwndDlg, SW_SHOW);
	SetForegroundWindow(hwndDlg);
	SetFocus(GetDlgItem(hwndDlg, IDC_LOAD));

	g_QuitCurrentGame	= false;
	g_GameStart			= false;
	g_AfterReady		= false;
	m_hWnd				= NULL;
}


void cServerDlg::create( HINSTANCE hInstance, LPCTSTR lpTemplate )
{
	m_hwndServerDlg = CreateDialog(hInstance, lpTemplate, NULL, Dlg_Proc);
	assert(m_hwndServerDlg != NULL);

	m_linkWebSite.ConvertStaticToHyperlink(m_hwndServerDlg, IDC_WEBSITE, TEXT("http://www.turingcup.com"));
	//m_linkAuthor.ConvertStaticToHyperlink(m_hwndServerDlg, IDC_AUTHOR, TEXT("mailto:LetUsF.ckGFW@Gmail.COM"));

	show(SW_SHOW);

	//::SendMessage(m_hwndServerDlg, WM_COMMAND, MAKEWPARAM(IDC_LOAD, 0), (LPARAM)::GetDlgItem(m_hwndServerDlg, IDC_LOAD));
	//::SendMessage(m_hwndServerDlg, WM_COMMAND, MAKEWPARAM(IDC_STARTGAME, 0), (LPARAM)::GetDlgItem(m_hwndServerDlg, IDC_STARTGAME));
}

void cServerDlg::run()
{
	MSG msg;
	ZeroMemory( &msg, sizeof(MSG) );

	while( GetMessage( &msg, NULL, 0, 0) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if( g_GameStart )
		{
			SetEvent(g_hEventNewGame);
			g_GameStart = false;
		}			
	}
}

void cServerDlg::show( int nCmdShow )
{
	assert(m_hwndServerDlg);
	ShowWindow(m_hwndServerDlg, nCmdShow);
}

INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	HWND hwndProgress;
	static DWORD dwChangeClr = 0;
	static int clrIndex = 1;
	static COLORREF clrStatic[2] = {
		RGB(0x76, 0x82, 0x66),
		RGB(0x3D, 0x83, 0x2F)
	};
	TCHAR szText[32];

	if( g_szAIPath1[0] != TEXT('\0') &&
		g_szAIPath2[0] != TEXT('\0') )
	{
		EnableWindow( GetDlgItem(hwnd, IDC_SWAP), TRUE);
	}
	else EnableWindow(GetDlgItem(hwnd, IDC_SWAP), FALSE);

	switch (uMsg) 
	{
		HANDLE_DLGMSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
		HANDLE_DLGMSG(hwnd, WM_COMMAND,    Dlg_OnCommand);

	case WM_CLOSE:
		g_bExitGameThread = TRUE;
		SetEvent(g_hEventNewGame);
		break;
		
	case WM_DESTROY:
		if(g_pszMsg)
			HeapFree(GetProcessHeap(), 0, g_pszMsg);

		UnloadAllClients();

		PostQuitMessage(0);
		break;

	case WM_LOAD_IN_PROGRESS:
		hwndProgress = GetDlgItem(hwnd, IDC_PROGRESS);
		SendMessage(hwndProgress, PBM_STEPIT, 0, 0);
		break;

	case WM_LOAD_DONE:
		{
			ShowWindow(GetDlgItem(hwnd, IDC_STATIC_PROGRESS), SW_HIDE);
			ShowWindow(GetDlgItem(hwnd, IDC_PROGRESS), SW_HIDE);

			KillTimer(hwnd, 1);
			
			ResizeEditbox(hwnd, TRUE);
			break;
		}

	case WM_CTLCOLORSTATIC:
		{
			HDC hdc = (HDC)wParam;

			hwndProgress = GetDlgItem(hwnd, IDC_STATIC_PROGRESS);
			if( hwndProgress == (HWND)lParam )
			{
				clrIndex = clrIndex ? 0 : 1;

				BOOL bRet = (BOOL)DefWindowProc(hwnd, uMsg, wParam, lParam);
				SetTextColor(hdc, clrStatic[clrIndex]);

				return bRet;
			}

			if( GetDlgItem(hwnd, IDC_AUTHOR) == (HWND)lParam )
			{
				BOOL bRet = (BOOL)DefWindowProc(hwnd, uMsg, wParam, lParam);
				SetTextColor(hdc, RGB(0x5B, 0x8A, 0xB3));

				return bRet;
			}
			return FALSE;
		}

	case WM_TIMER:
		{
			UINT len = GetDlgItemText(hwnd, IDC_STATIC_PROGRESS, szText, _countof(szText)) + 1;
			if( len > 20 )
				len = 20 - 3;
			else
				szText[len - 1] = TEXT('.');

			szText[len] = TEXT('\0');
			SetDlgItemText(hwnd, IDC_STATIC_PROGRESS, szText);
			break;
		}
	}

	return(FALSE);
}

BOOL WINAPI Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hwndFocus);
	UNREFERENCED_PARAMETER(lParam);

	PCTSTR pszHeros[] = {
		TEXT("Ron Weasley"),
		TEXT("Hermione Granger"),
		TEXT("Draco Malfoy"),
		TEXT("Harry Potter"),
		TEXT("Voldemort"),
		TEXT("Ginny Weasley") };

	// Set icons
	HICON hIcon = LoadIcon((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), 
		MAKEINTRESOURCE(IDI_TC9));
	if( hIcon )
	{
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (WPARAM)hIcon);
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (WPARAM)hIcon);
	}

	// hide loading controls
	ShowWindow(GetDlgItem(hwnd, IDC_PROGRESS), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_STATIC_PROGRESS), SW_HIDE);
	GetWindowRect(GetDlgItem(hwnd, IDC_MSG), &g_rcOld);
	MapWindowRect(HWND_DESKTOP, hwnd, &g_rcOld);

	ResizeEditbox(hwnd, TRUE);

	// init combobox
	for(int i = 0; i != _countof(pszHeros); i++)
	{
		ComboBox_AddString( GetDlgItem(hwnd, IDC_KEY_LIST1), pszHeros[i] );
		ComboBox_AddString( GetDlgItem(hwnd, IDC_KEY_LIST2), pszHeros[i] );
		ComboBox_AddString( GetDlgItem(hwnd, IDC_KEY_LIST11), pszHeros[i] );
		ComboBox_AddString( GetDlgItem(hwnd, IDC_KEY_LIST22), pszHeros[i] );
	}
	ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_KEY_LIST1), 0);
	ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_KEY_LIST2), 2);
	ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_KEY_LIST11), 3);
	ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_KEY_LIST22), 3);

	// init buttons
	CheckRadioButton(hwnd, IDC_KEY1, IDC_AI1, IDC_KEY1);
	CheckRadioButton(hwnd, IDC_KEY2, IDC_AI2, IDC_KEY2);

	Button_Enable(GetDlgItem(hwnd, IDC_SELECT1), FALSE);
	Button_Enable(GetDlgItem(hwnd, IDC_SELECT2), FALSE);

	Button_Enable(GetDlgItem(hwnd, IDC_STARTGAME), FALSE);

	g_pszMsg = (PTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MSG_BUF_SIZE * sizeof(TCHAR));
	assert(g_pszMsg != NULL);

	g_szAIPath1[0] = g_szAIPath2[0] = TEXT('\0');
	g_pfnAI[0] = g_pfnAI[1] = NULL;
	g_pfnInit[0] = g_pfnInit[1] = NULL;

	// no more memory, exit
	if( !g_pszMsg )
		return FALSE;

	// the game window is created in a separate thread
	g_hEventNewGame = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hThreadNewGame = BEGINTHREADEX(NULL, 0, GameProc, hwnd, 0, NULL);
	g_bExitGameThread = FALSE;

	g_bUseOvertimeConfig = false;

	return TRUE;
}

static BOOL CheckLoadBtn(HWND hwnd)
{
	if( IsDlgButtonChecked(hwnd, IDC_KEY1) &&
		IsDlgButtonChecked(hwnd, IDC_KEY2) )
	{
		return TRUE;
	}
	else
	{
		if( IsDlgButtonChecked(hwnd, IDC_KEY1) )
		{
			return g_szAIPath2[0];
		}

		if( IsDlgButtonChecked(hwnd, IDC_KEY2) )
		{
			return g_szAIPath1[0];
		}

		return  g_szAIPath1[0] && g_szAIPath2[0];
	}
}

static void __stdcall __LoadDll(HWND hwnd, int ctrl_id, PCTSTR szPath)
{
	HMODULE hmod = NULL;

	if( szPath[0] == TEXT('\0') )
		return;

	// keyboard mode, do not load the DLL
	if( IsDlgButtonChecked(hwnd, ctrl_id) )
		return;


	// free the DLL first
	while( GetModuleHandle(szPath) )
		FreeLibrary(GetModuleHandle(szPath));

	// load the Dll into our address space
	hmod = LoadLibrary(szPath);

	if( !hmod )
	{
		DWORD dwError = GetLastError();
		HLOCAL pszErrMsg = NULL;
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwError, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			(PTSTR)&pszErrMsg,
			0,
			NULL);

		if( pszErrMsg != NULL )
		{
			MessageBox(GetForegroundWindow(),
				(PCTSTR)pszErrMsg, 
				TEXT("Oops!"),
				MB_OK | MB_ICONEXCLAMATION);

			LocalFree(pszErrMsg);
		}
	}
	
	// We will free the dll in __End
}

void WINAPI Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	UNREFERENCED_PARAMETER(codeNotify);

	PTSTR pstrTeamName = NULL;
	BOOL b_AllOK = TRUE;
	TCHAR szTmp[MAX_PATH];
	HWND hwndProgress;

	static Hero_Type iHeroTypes[] = {
		HERO_RON,
		HERO_HERMIONE,
		HERO_MALFOY,
		HERO_HARRY,
		HERO_VOLDEMORT,
		HERO_GINNY,
	};


	switch( id )
	{
	case IDCANCEL:
		DestroyWindow(hwnd);
		break;

	case IDC_STARTGAME:
		{
			EnableWindow(hwndCtl, FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_LOAD), CheckLoadBtn(hwnd) );

			g_GameStart = true;

			// show progress bar
			ShowWindow(GetDlgItem(hwnd, IDC_STATIC_PROGRESS), SW_SHOW);

			hwndProgress = GetDlgItem(hwnd, IDC_PROGRESS);
			SendMessage(hwndProgress, PBM_SETRANGE32, 0, 11);
			SendMessage(hwndProgress, PBM_SETSTEP, 1, 0);
			ShowWindow(hwndProgress, SW_SHOW);
			SendMessage(hwndProgress, PBM_SETPOS, 0, 0);
			SendMessage(hwndProgress, PBM_SETSTATE, PBST_NORMAL, 0);

			ResizeEditbox(hwnd, FALSE);

			SetTimer(hwnd, 1, 300, NULL);

			break;
		}

	case IDC_KEY1:
		Button_Enable(GetDlgItem(hwnd, IDC_KEY_LIST1), TRUE);
		Button_Enable(GetDlgItem(hwnd, IDC_KEY_LIST11), TRUE);
		Button_Enable(GetDlgItem(hwnd, IDC_SELECT1), FALSE);

		Button_Enable( GetDlgItem(hwnd, IDC_LOAD), CheckLoadBtn(hwnd) );
		Button_Enable( GetDlgItem(hwnd, IDC_STARTGAME), FALSE);
		break;

	case IDC_KEY2:
		Button_Enable(GetDlgItem(hwnd, IDC_KEY_LIST2), TRUE);
		Button_Enable(GetDlgItem(hwnd, IDC_KEY_LIST22), TRUE);
		Button_Enable(GetDlgItem(hwnd, IDC_SELECT2), FALSE);

		Button_Enable( GetDlgItem(hwnd, IDC_LOAD), CheckLoadBtn(hwnd) );
		Button_Enable( GetDlgItem(hwnd, IDC_STARTGAME), FALSE);
		break;

	case IDC_AI1:
		Button_Enable(GetDlgItem(hwnd, IDC_SELECT1), TRUE);
		Button_Enable(GetDlgItem(hwnd, IDC_KEY_LIST1), FALSE);
		Button_Enable(GetDlgItem(hwnd, IDC_KEY_LIST11), FALSE);

		Button_Enable(GetDlgItem(hwnd, IDC_LOAD), CheckLoadBtn(hwnd));
		Button_Enable( GetDlgItem(hwnd, IDC_STARTGAME), FALSE);
		break;

	case IDC_AI2:
		Button_Enable(GetDlgItem(hwnd, IDC_SELECT2), TRUE);
		Button_Enable(GetDlgItem(hwnd, IDC_KEY_LIST2), FALSE);
		Button_Enable(GetDlgItem(hwnd, IDC_KEY_LIST22), FALSE);

		Button_Enable(GetDlgItem(hwnd, IDC_LOAD), CheckLoadBtn(hwnd) );
		Button_Enable( GetDlgItem(hwnd, IDC_STARTGAME), FALSE);
		break;

	case IDC_SELECT1:
		// unload client
		UnloadClient(g_szAIPath1);

		// load new client
		if( GetClientDll(hwnd, g_szAIPath1, MAX_PATH) )
		{
			Button_Enable(GetDlgItem(hwnd, IDC_LOAD), CheckLoadBtn(hwnd));
		}
		else
		{
			// reload the client into our address space
			if(g_szAIPath1[0])
				LoadLibrary(g_szAIPath1);
		}
		break;

	case IDC_SELECT2:
		// unload client
		UnloadClient(g_szAIPath2);

		// load new client
		if( GetClientDll(hwnd, g_szAIPath2, MAX_PATH) )
		{
			Button_Enable(GetDlgItem(hwnd, IDC_LOAD), CheckLoadBtn(hwnd));
		}
		else
		{
			// reload the client
			if(g_szAIPath2[0])
				LoadLibrary(g_szAIPath2);
		}
		break;

	case IDC_SWAP:
		if( IsWindowVisible(GetDlgItem(hwnd, IDC_PROGRESS)) )
			break;

		// make sure DLLs are unloaded
		while( GetModuleHandle(g_szAIPath1) )
			FreeLibrary(GetModuleHandle(g_szAIPath1));

		while( GetModuleHandle(g_szAIPath2) )
			FreeLibrary(GetModuleHandle(g_szAIPath2));

		StringCchCopy(szTmp, _countof(szTmp), g_szAIPath1);
		StringCchCopy(g_szAIPath1, _countof(g_szAIPath1), g_szAIPath2);
		StringCchCopy(g_szAIPath2, _countof(g_szAIPath2), szTmp);

		EnableWindow(GetDlgItem(hwnd, IDC_STARTGAME), FALSE);
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_LOAD, 0), (LPARAM)GetDlgItem(hwnd, IDC_LOAD));
		EnableWindow(GetDlgItem(hwnd, IDC_STARTGAME), TRUE);

		break;

	case IDC_LOAD:
		if( IsWindowVisible(GetDlgItem(hwnd, IDC_PROGRESS)) )
			break;

		g_pfnAI[0] = g_pfnAI[1] = NULL;
		g_pfnInit[0] = g_pfnInit[1] = NULL;

		__LoadDll(hwnd, IDC_KEY1, g_szAIPath1);
		__LoadDll(hwnd, IDC_KEY2, g_szAIPath2);

		// team1
		if( IsDlgButtonChecked(hwnd, IDC_KEY1) )
		{
			// keyboard mode
			StringCchCopy(g_pszMsg, MSG_BUF_SIZE, TEXT("Team1[Keyboard]: loaded.\r\n"));

			g_GameMode1[0] = GAME_MODE_KEYBOARD;

			g_AI_HeroType[0] = iHeroTypes[ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_KEY_LIST1))];
			g_AI_HeroType[1] = iHeroTypes[ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_KEY_LIST11))];
		}
		else
		{
			//
			// AI mode
			//

			// get the addr of AI and Init
			pstrTeamName = _tcsrchr(g_szAIPath1, TEXT('\\')) + 1;
			if( !g_pfnAI[0] )
				g_pfnAI[0] = GetAI(g_szAIPath1);
			if( !g_pfnInit[0] )
				g_pfnInit[0] = GetInit(g_szAIPath1);

			if( g_pfnAI[0] && g_pfnInit[0] )
			{
				StringCchPrintf(g_pszMsg, MSG_BUF_SIZE, TEXT("%s[AI]: loaded.\r\n"),
					pstrTeamName );

				g_GameMode1[0] = GAME_MODE_AI;
			}
			else
			{
				StringCchPrintf(g_pszMsg, MSG_BUF_SIZE, TEXT("%s[AI]: error while loading.\r\n"),
					pstrTeamName );

				UnloadClient(g_szAIPath1);

				b_AllOK = FALSE;
			}		
		}

		// team2
		if( Button_GetState(GetDlgItem(hwnd, IDC_KEY2)) == BST_CHECKED )
		{
			// keyboard mode
			StringCchCat(g_pszMsg, MSG_BUF_SIZE, TEXT("Team2[Keyboard]: loaded.\r\n"));
			
			g_GameMode2[0] = GAME_MODE_KEYBOARD;
			
			g_AI_HeroType[2] = iHeroTypes[ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_KEY_LIST2))];
			g_AI_HeroType[3] = iHeroTypes[ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_KEY_LIST22))];	
		}
		else
		{
			//
			// AI mode
			//

			// load the dll and get the addr of AI
			pstrTeamName = _tcsrchr(g_szAIPath2, TEXT('\\')) + 1;
			if( !g_pfnAI[1] )
				g_pfnAI[1] = GetAI(g_szAIPath2);
			if( !g_pfnInit[1] )
				g_pfnInit[1] = GetInit(g_szAIPath2);

			if( g_pfnAI[1] && g_pfnInit[1] )
			{
				StringCchCat(g_pszMsg, MSG_BUF_SIZE, pstrTeamName);
				StringCchCat(g_pszMsg, MSG_BUF_SIZE, TEXT("[AI]: loaded.\r\n"));

				g_GameMode2[0] = GAME_MODE_AI;
			}
			else
			{
				StringCchCat(g_pszMsg, MSG_BUF_SIZE, pstrTeamName);
				StringCchCat(g_pszMsg, MSG_BUF_SIZE, TEXT("[AI]: error while loading.\r\n"));

				UnloadClient(g_szAIPath2);
				b_AllOK = FALSE;
			}
		}

		Button_Enable(GetDlgItem(hwnd, IDC_STARTGAME), b_AllOK);
		SetDlgItemText(hwnd, IDC_MSG, g_pszMsg);
		EnableWindow( GetDlgItem(hwnd, IDC_LOAD), CheckLoadBtn(hwnd) );
		break;

	case IDC_ONTOP:
		SetWindowPos(hwnd, IsDlgButtonChecked(hwnd, IDC_ONTOP) 
			? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		break;

	case IDC_CHK_OVERTIME:
		g_bUseOvertimeConfig = !g_bUseOvertimeConfig;
		break;

	}	/* end switch */
}

BOOL WINAPI GetClientDll(HWND hwndOwner, PTSTR pstrPath, DWORD dwMaxCount)
{
	//
	// If the user dismisses the dialog, nothing will be written to pstrPath
	//

	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szCurDir[MAX_PATH];       // buffer for current dir
	TCHAR szPath[MAX_PATH];
	static TCHAR szInitialDir[MAX_PATH] = {0};
	BOOL bRet = FALSE;

	if( pstrPath == NULL )
		return bRet;

	GetCurrentDirectory(MAX_PATH, szCurDir);

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwndOwner;
	ofn.lpstrFile = szPath;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = TEXT('\0');
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TEXT("All Files\0*.*\0TuringCup9 Client Module\0*.DLL\0");
	ofn.nFilterIndex = 2;		// *.dll
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = szInitialDir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 
	BOOL bOK = GetOpenFileName(&ofn);
	SetCurrentDirectory(szCurDir);

	if( bOK && szPath[0] != 0 )
	{
		// Client DLL loaded
		StringCchCopy(pstrPath, dwMaxCount, szPath);
		bRet = TRUE;

		// Store the path for future use
		*( _tcsrchr(szPath, TEXT('\\')) ) = TEXT('\0');
		StringCchCopy(szInitialDir, _countof(szInitialDir), szPath);
	}

	return bRet;
}

PFN_AI WINAPI GetAI(PCTSTR pstrDllPath)
{
	PFN_AI pfn_ai = NULL;
	HMODULE hmodDll = NULL;

	//hmodDll= LoadLibrary(pstrDllPath);
	hmodDll = GetModuleHandle(pstrDllPath);
	assert( hmodDll != NULL );

	if( hmodDll )
	{
		pfn_ai = (PFN_AI)GetProcAddress(hmodDll, "AI");
	}

	return pfn_ai;
}

PFN_AI WINAPI GetInit(PCTSTR pstrDllPath)
{
	PFN_INIT pfn_init = NULL;
	HMODULE hmodeDll = NULL;

	hmodeDll = GetModuleHandle(pstrDllPath);
	assert( hmodeDll != NULL );

	if( hmodeDll )
	{
		pfn_init = (PFN_INIT)GetProcAddress(hmodeDll, "Init");
	}

	return pfn_init;
}


VOID WINAPI UnloadClient(PTSTR pstrModule)
{
	if( !pstrModule[0] )
		return;

	// not a empty path, unload the dll
	HMODULE hDll = GetModuleHandle(pstrModule);
	//assert( hDll );

	if( hDll )
	{
		FreeLibrary(hDll);
	}
}

VOID WINAPI UnloadAllClients()
{
	/*
	if( g_pfnAI[0] )
		UnloadClient(g_szAIPath1);
	if( g_pfnAI[1] )
		UnloadClient(g_szAIPath2);
	*/

	UnloadClient(g_szAIPath1);
	UnloadClient(g_szAIPath2);
}

DWORD WINAPI GameProc(PVOID pvParam)
{
	while(!g_bExitGameThread)
	{
		WaitForSingleObject(g_hEventNewGame, INFINITE);
		if( g_bExitGameThread )
			return 0;

		g_pApp->createNewGame((HWND)pvParam);
	}

	return 0;
}

VOID ResizeEditbox(HWND hwnd, BOOL bReset)
{
	if( !bReset )
	{
		RECT rcText;
		GetWindowRect(GetDlgItem(hwnd, IDC_MSG), &rcText);
		MapWindowRect(HWND_DESKTOP, hwnd, &rcText);
		rcText.bottom = g_rcOld.bottom;
		MapWindowRect(hwnd, HWND_DESKTOP, &rcText);
		SetWindowPos(GetDlgItem(hwnd, IDC_MSG), NULL,
			0, 0,
			rcText.right - rcText.left, rcText.bottom - rcText.top,
			SWP_NOZORDER | SWP_NOMOVE);
	}
	else
	{
		RECT rcText, rcProgress;
		HWND hwndProgress, hwndText;
		hwndProgress = GetDlgItem(hwnd, IDC_PROGRESS);
		hwndText = GetDlgItem(hwnd, IDC_MSG);
		GetWindowRect(hwndProgress, &rcProgress);
		GetWindowRect(hwndText, &rcText);
		MapWindowRect(HWND_DESKTOP, hwnd, &rcProgress);
		MapWindowRect(HWND_DESKTOP, hwnd, &rcText);
		rcText.bottom = rcProgress.bottom;
		MapWindowRect(hwnd, HWND_DESKTOP, &rcText);
		SetWindowPos(hwndText, NULL,
			0, 0,
			rcText.right - rcText.left, rcText.bottom - rcText.top,
			SWP_NOMOVE | SWP_NOZORDER);
	}
}