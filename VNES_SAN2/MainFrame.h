//
// ���C���E�C���h�E�N���X
//
#ifndef	__CMAINFRAME_INCLUDED__
#define	__CMAINFRAME_INCLUDED__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
using namespace std;

#include "Wnd.h"
#include "EmuThread.h"
#include "CheatDlg.h"


class	CMainFrame : public CWnd
{
public:
	// Override from CWnd
	BOOL	Create( HWND hWndParent );
	void	Destroy();

	// Override from CWnd
	BOOL	PreTranslateMessage( MSG* pMsg );
protected:
	void	OnEmulationStart( LPCSTR szFile, BOOL bChecked );

	void	OnFullScreenGDI( BOOL bFlag );
	void	OnShowCursor( BOOL bFlag );
	void	OnChangeMenu( BOOL bFlag );
	void	OnSetWindowSize();
	void	OnRebuildMenu();
	void	OnUpdateMenu( HMENU hMenu, UINT uID );

	void	OnKeyControl();

	// Message map
	WND_MESSAGE_MAP()
	WNDMSG		OnCreate( WNDMSGPARAM );
	WNDMSG		OnClose( WNDMSGPARAM );
	WNDMSG		OnDestroy( WNDMSGPARAM );
	WNDMSG		OnActivate( WNDMSGPARAM );
	WNDMSG		OnActivateApp( WNDMSGPARAM );
	WNDMSG		OnShortCutEnable( WNDMSGPARAM );	// Private message
	WNDMSG		OnEnable( WNDMSGPARAM );
	WNDMSG		OnEnterMenuLoop( WNDMSGPARAM );
	WNDMSG		OnExitMenuLoop( WNDMSGPARAM );
	WNDMSG		OnSetFocus( WNDMSGPARAM );
	WNDMSG		OnKillFocus( WNDMSGPARAM );
	WNDMSG		OnInitMenu( WNDMSGPARAM );
	WNDMSG		OnInitMenuPopup( WNDMSGPARAM );
	WNDMSG		OnGetMinMaxInfo( WNDMSGPARAM );
	WNDMSG		OnPaint( WNDMSGPARAM );
	WNDMSG		OnDisplayChange( WNDMSGPARAM );
	WNDMSG		OnPaletteChanged( WNDMSGPARAM );
	WNDMSG		OnQueryNewPalette( WNDMSGPARAM );
	WNDMSG		OnMenuChar( WNDMSGPARAM );
	WNDMSG		OnKeyDown( WNDMSGPARAM );
	WNDMSG		OnSize( WNDMSGPARAM );
	WNDMSG		OnSysCommand( WNDMSGPARAM );
	WNDMSG		OnExitSizeMove( WNDMSGPARAM );

	WNDMSG		OnSetCursor( WNDMSGPARAM );
	WNDMSG		OnTimer( WNDMSGPARAM );

	WNDMSG		OnCopyData( WNDMSGPARAM );

	WNDMSG		OnErrorMessage( WNDMSGPARAM );		// Private message

	WNDMSG		OnCommandLine( WNDMSGPARAM );		// Private message

	WNDMSG		OnNetPlay( WNDMSGPARAM );
	WNDMSG		OnNetPlayClose( WNDMSGPARAM );
	WNDMSG		OnNetPlayError( WNDMSGPARAM );

	WNDCMD		OnExit( WNDCMDPARAM );
	WNDCMD		OnHelp( WNDCMDPARAM );
	WNDCMD		OnAbout( WNDCMDPARAM );

	WNDCMD		OnSearch( WNDCMDPARAM );
	WNDCMD		OnCheat( WNDCMDPARAM );
	WNDCMD		OnGenie( WNDCMDPARAM );
	WNDCMD		OnCheatCommand( WNDCMDPARAM );

	WNDCMD		OnNetPlayConnect( WNDCMDPARAM );
	WNDCMD		OnNetPlayDisconnect( WNDCMDPARAM );

	WNDCMD		OnEmulatorCfg( WNDCMDPARAM );
	WNDCMD		OnGraphicsCfg( WNDCMDPARAM );
	WNDCMD		OnSoundCfg( WNDCMDPARAM );
	WNDCMD		OnControllerCfg( WNDCMDPARAM );
	WNDCMD		OnShortcutCfg( WNDCMDPARAM );
	WNDCMD		OnMovieCfg( WNDCMDPARAM );
	WNDCMD		OnGameOptionCfg( WNDCMDPARAM );
	WNDCMD		OnFullScreen( WNDCMDPARAM );
	WNDCMD		OnOnTop( WNDCMDPARAM );
	WNDCMD		OnZoom( WNDCMDPARAM );

	WNDCMD		OnEmuCommand( WNDCMDPARAM );

	WNDCMD		OnStateCommand( WNDCMDPARAM );
	WNDCMD		OnMovieCommand( WNDCMDPARAM );
	WNDCMD		OnMovieInfo( WNDCMDPARAM );

	// Command line
	CHAR	m_szCommandLine[_MAX_PATH];

	//�ö�
	BOOL	m_OnTop;

	// Window activate flags
	BOOL	m_bActivate;
	BOOL	m_bActivateApp;
	BOOL	m_bForcus;
	BOOL	m_bKeyEnable;

	// Cursor
	BOOL	m_bCursor;
	UINT	m_uTimerID;
	DWORD	m_LastMovedTime;

	// Menu flags
	BOOL	m_bMenu;

	// Window Position
	BOOL	m_bZoomed;
	RECT	m_WindowRect;
	WINDOWPLACEMENT	m_WindowPlacement;

	// Window Style
	LONG	m_ExStyleBackup;
	LONG	m_StyleBackup;

	// Short Cut
	UINT	m_uKeyTimerID;
	volatile BOOL	m_bKeyChecking;
	BYTE	m_KeyBuf[256*64*8];
	BYTE	m_KeyCnt[256*64*8];

	// Emulator
	NES*		Nes;
//	CEmuThread	Emu;

	// State slot
	INT	m_nStateSlot;

	// Cheat
	CSearchDlg	m_SearchDlg;

private:
};

#endif	// !__CMAINFRAME_INCLUDED__

