//
// �l�b�g�v���C�N���X
//
#ifndef	__CNETPLAY_INCLUDED__
#define	__CNETPLAY_INCLUDED__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <winsock.h>

#include <string>
using namespace std;

#define	WM_NETPLAY		(WM_APP+100)

#define	WM_NETPLAY_ACCEPT	(WM_APP+110)
#define	WM_NETPLAY_CONNECT	(WM_APP+111)
#define	WM_NETPLAY_CLOSE	(WM_APP+112)
#define	WM_NETPLAY_ERROR	(WM_APP+113)

class	CNetPlay
{
public:
	CNetPlay();
	~CNetPlay();

	// ������/�J��
	BOOL	Initialize( HWND hWnd );
	void	Release();

	// �l�b�g�v���C�\�H
	BOOL	IsNetPlay() { return m_hWnd?TRUE:FALSE; }
	// �ڑ����H
	BOOL	IsConnect() { return m_hWnd?m_bConnect:FALSE; }
	// �ڑ����H
	BOOL	IsServer() { return m_bServer; }

	// �ʐM���C�e���V
	void	SetLatency( INT nLatency ) { m_nLatency = nLatency; }
	INT	GetLatency() { return m_nLatency; }

	// �񓯊��������b�Z�[�W�ԑ��E�C���h�E�̐ݒ�
	void	SetMsgWnd( HWND hWnd ) { m_hWndMsg = hWnd; }

	// �ڑ��Ɛؒf
	BOOL	Connect( BOOL bServer, const char* IP, unsigned short Port );
	void	Disconnect();

	// �f�[�^���M 0:��M�f�[�^�҂� 1�ȏ�:��M�f�[�^���� 0����:�ڑ��؂��G���[
	INT	Send( unsigned char* pBuf, int size );
	// �f�[�^��M
	// 0:��M�f�[�^�҂� 1�ȏ�:��M�f�[�^���� 0����:�ڑ��؂��G���[
	// �^�C���A�E�g����
	INT	Recv( unsigned char* pBuf, int size );
	// �^�C���A�E�g�L��
	INT	RecvTime( unsigned char* pBuf, int size, unsigned long timeout );

	// Windows���b�Z�[�W�v���V�[�W��
	HRESULT	WndProc( HWND hWnd, WPARAM wParam, LPARAM lParam );
protected:
	// �����o�ϐ�
	HWND	m_hWnd;
	HWND	m_hWndMsg;

	BOOL	m_bServer;
	BOOL	m_bConnect;	// �ڑ����H
	INT	m_nLatency;

	// WINSOCK
	WSADATA	m_WSAdata;
	SOCKET	m_SocketConnect;
	SOCKET	m_SocketData;
private:
};

extern	CNetPlay	NetPlay;

#endif	// !__CNETPLAY_INCLUDED__
