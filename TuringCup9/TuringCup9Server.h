#ifndef		__APPLICATION_H__
#define		__APPLICATION_H__

#include <windows.h>
#include "Engine/Core_Globaldef.h"
#include "GlobalDef.h"
#include "hyperlink.h"

extern enum Hero_Type;

class cTuringCup9ServerApp{
public:
	cTuringCup9ServerApp();
	virtual ~cTuringCup9ServerApp();

	bool		run();
	bool		moveWindow(long p_xPos, long p_yPos);
	bool		resizeClient(long p_width, long p_height);
	bool		showMouse(bool p_show = true);
	
	HWND		gethWnd() { return m_hWnd; }
	HINSTANCE	gethInst() { return m_hInst; }

	virtual LRESULT CALLBACK 
		msgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{ return DefWindowProc( hwnd, uMsg, wParam, lParam ); }

	virtual	bool	init()		 { return true; }
	virtual	bool	cleanUp()	 { return true; }

	virtual void	gameLoop()	 { return ;		}

	void		createNewGame(HWND hwndDlg);
	
protected:
	TCHAR		m_szClassName[MAX_PATH];
	TCHAR		m_szCaptionName[MAX_PATH];

	WNDCLASSEX	m_wcex;
	
	unsigned	m_style;
	unsigned	m_width;
	unsigned	m_height;
	
	long		m_xPos;
	long		m_yPos;
	
	HICON		m_hIcon;
	HCURSOR		m_hCursor;


private:
	HWND		m_hWnd;
	HINSTANCE	m_hInst;
	DWORD		m_dwLastFrame;
};

static cTuringCup9ServerApp* g_pApp = NULL;
static LRESULT CALLBACK AppWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


class cServerDlg
{
public:
	cServerDlg():m_hwndServerDlg(NULL)
	{

	}

	~cServerDlg()
	{

	}

	void create(HINSTANCE hInstance,  LPCTSTR lpTemplate);
	void run();

private:
	void show( int nCmdShow );

private:
	HWND		m_hwndServerDlg;
	CHyperLink	m_linkWebSite;
	//CHyperLink	m_linkAuthor;
};

INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void WINAPI Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL WINAPI GetClientDll(HWND hwndOwner, PTSTR pstrPath, DWORD dwMaxCount);
VOID WINAPI UnloadClient(PTSTR pstrModule);
VOID WINAPI UnloadAllClients();
VOID ResizeEditbox(HWND hwnd, BOOL bReset);
DWORD WINAPI GameProc(PVOID pvParam);


PFN_AI WINAPI GetAI(PCTSTR pstrDllPath);
PFN_AI WINAPI GetInit(PCTSTR pstrDllPath);

// message cracker
#define HANDLE_DLGMSG(hWnd, message, fn)                 \
   case (message): return (SetDlgMsgResult(hWnd, uMsg,     \
   HANDLE_##message((hWnd), (wParam), (lParam), (fn))))

#endif