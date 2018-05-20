//
// �l�b�g�v���C�N���X
//
// �����I��WinSock Lib�������N
#pragma comment(lib, "wsock32.lib")

//#include "DebugOut.h"

#include "NetPlay.h"

#define	CLOSESOCKET(soc) if((soc)!=INVALID_SOCKET){::closesocket((soc));(soc)=INVALID_SOCKET;}

CNetPlay	NetPlay;

CNetPlay::CNetPlay()
{
	m_hWnd = m_hWndMsg = NULL;
	m_bConnect = FALSE;

	m_SocketConnect = INVALID_SOCKET;
	m_SocketData = INVALID_SOCKET;
}

CNetPlay::~CNetPlay()
{
	// �Ƃ肠����
	Release();
}

BOOL	CNetPlay::Initialize( HWND hWnd )
{
	// �Ƃ肠����
	Release();

	// WinSock DLL�̏�����
	if( ::WSAStartup( MAKEWORD(1,1), &m_WSAdata ) )
		return	FALSE;

	// �o�[�W�����Ⴄ����`��
	if( m_WSAdata.wVersion != MAKEWORD(1,1) ) {
		::WSACleanup();
		return	FALSE;
	}

	m_hWnd = hWnd;
	return	TRUE;
}

void	CNetPlay::Release()
{
	Disconnect();

	if( m_hWnd ) {
		::WSACleanup();
		m_hWnd = NULL;
	}
}

BOOL	CNetPlay::Connect( BOOL bServer, const char* IP, unsigned short Port )
{
	if( !m_hWnd )
		return	FALSE;

	m_bServer = bServer;

	if( bServer ) {
	// Server
		// �ڑ��ʒm�\�P�b�g�쐬
		if( m_SocketConnect == INVALID_SOCKET ) {
			m_SocketConnect = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
			if( m_SocketConnect == INVALID_SOCKET ) {
//				DEBUGOUT( "CNetPlay:socket failed.\n" );
				return	FALSE;
			}
		}
		// �|�[�g�ƌ��т���
		struct sockaddr_in addr;
		::ZeroMemory( &addr, sizeof(addr) );
		addr.sin_family      = AF_INET;
		addr.sin_addr.s_addr = ::htonl( INADDR_ANY );
		addr.sin_port        = ::htons( Port );
		if( ::bind( m_SocketConnect, (struct sockaddr *)&addr, sizeof(addr) ) == SOCKET_ERROR ) {
//			DEBUGOUT( "CNetPlay:bind failed.\n" );
			CLOSESOCKET( m_SocketConnect );
			return	FALSE;
		}
		// �ڑ��v���C�x���g�̐ݒ�
		if( ::WSAAsyncSelect( m_SocketConnect, m_hWnd, WM_NETPLAY, FD_ACCEPT ) == SOCKET_ERROR ) {
//			DEBUGOUT( "CNetPlay:WSAAsyncSelect failed.\n" );
			CLOSESOCKET( m_SocketConnect );
			return	FALSE;
		}
		// �ڑ��v����t�J�n
		if( ::listen( m_SocketConnect, 1 ) == SOCKET_ERROR ) {
//			DEBUGOUT( "CNetPlay:listen failed.\n" );
			CLOSESOCKET( m_SocketConnect );
			return	FALSE;
		}
	} else {
	// Client
		// IP�A�h���X�H
		unsigned long IP_address = ::inet_addr( IP );
		if( IP_address == INADDR_NONE ) {
//			DEBUGOUT( "CNetPlay:IP�A�h���X���s���ł��B\"%s\"\n", IP );
			return	FALSE;
		}

		// �f�[�^�ʐM�\�P�b�g�쐬
		m_SocketData = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if( m_SocketData == INVALID_SOCKET ) {
//			DEBUGOUT( "CNetPlay:socket failed.\n" );
			return	FALSE;
		}
		// Nagle�A���S���Y���̖�����
		BOOL	bOpt = TRUE;
		if( ::setsockopt( m_SocketData, IPPROTO_TCP, TCP_NODELAY, (const char*)&bOpt, sizeof(bOpt) ) == SOCKET_ERROR ) {
//			DEBUGOUT( "CNetPlay:setsockopt failed.\n" );
			CLOSESOCKET( m_SocketData );
			return	FALSE;
		}
		// �u���b�L���O���[�h�ݒ�
		BOOL	bArg = TRUE;
		if( ::ioctlsocket( m_SocketData, FIONBIO, (unsigned long*)&bArg ) == SOCKET_ERROR ) {
//			DEBUGOUT( "CNetPlay:ioctlsocket failed.\n" );
			CLOSESOCKET( m_SocketData );
			return	FALSE;
		}
		// �ڑ������C�x���g�̐ݒ�
		if( ::WSAAsyncSelect( m_SocketData, m_hWnd, WM_NETPLAY, FD_CONNECT ) == SOCKET_ERROR ) {
//			DEBUGOUT( "CNetPlay:WSAAsyncSelect failed.\n" );
			CLOSESOCKET( m_SocketData );
			return	FALSE;
		}
		// �ڑ���v������
		struct sockaddr_in addr;
		::ZeroMemory( &addr, sizeof(addr) );
		addr.sin_family      = AF_INET;
		addr.sin_addr.s_addr = IP_address;
		addr.sin_port        = ::htons( Port );
		if( ::connect( m_SocketData, (struct sockaddr *)&addr, sizeof(addr) ) == SOCKET_ERROR ) {
			if( ::WSAGetLastError() != WSAEWOULDBLOCK ) {
//				DEBUGOUT( "CNetPlay:connect failed.\n" );
				CLOSESOCKET( m_SocketData );
				return	FALSE;
			}
		}
	}

	return	TRUE;
}

void	CNetPlay::Disconnect()
{
	// �\�P�b�g���V���b�g�_�E�����Ĕj��
	if( m_SocketConnect != INVALID_SOCKET ) {
//		::shutdown( m_SocketConnect, SD_BOTH );	// WS2�ȍ~�炵���c
		CLOSESOCKET( m_SocketConnect );
	}
	if( m_SocketData != INVALID_SOCKET ) {
//		::shutdown( m_SocketData, SD_BOTH );	// WS2�ȍ~�炵���c
		CLOSESOCKET( m_SocketData );
	}

	m_bConnect = FALSE;
}

INT	CNetPlay::Send( unsigned char* pBuf, int size )
{
	if( !m_hWnd || !m_bConnect || m_SocketData == INVALID_SOCKET )
		return	-1L;

	if( ::send( m_SocketData, (char*)pBuf, size, 0 ) == SOCKET_ERROR ) {
//		DEBUGOUT( "CNetPlay:send failed. code=%d\n", ::WSAGetLastError() );
		Disconnect();
		return	-1L;
	}

	return	0L;
}

INT	CNetPlay::Recv( unsigned char* pBuf, int size )
{
	if( !m_hWnd || !m_bConnect || m_SocketData == INVALID_SOCKET )
		return	-1L;

	// �f�[�^���͂��Ă��邩���m�F
	unsigned long	len = 0;
	if( ::ioctlsocket( m_SocketData, FIONREAD, (unsigned long*)&len ) == SOCKET_ERROR ) {
//		DEBUGOUT( "CNetPlay:ioctlsocket failed.\n" );
		Disconnect();
		return	-1L;
	}

	if( !len ) {
		// �f�[�^���͂��Ă��Ȃ�
		return	0L;
	} else {
		// �v���f�[�^�T�C�Y���ǂݍ���
		if( ::recv( m_SocketData, (char*)pBuf, size, 0 ) == SOCKET_ERROR ) {
//			DEBUGOUT( "CNetPlay:recv failed.\n" );
			Disconnect();
			return	-1L;
		}
	}

	return	size;
}

INT	CNetPlay::RecvTime( unsigned char* pBuf, int size, unsigned long timeout )
{
	INT	ret;
	unsigned long dwTimeOut;
	dwTimeOut = ::timeGetTime();
	while( (ret = NetPlay.Recv( pBuf, size )) == 0 ) {
		// �ł܂�Ȃ��[�u
		::Sleep( 0 );
		// �^�C���A�E�g�̃`�F�b�N
		if( (::timeGetTime()-dwTimeOut) > timeout ) {
			return	-1;
		}
	}
	return	ret;
}

HRESULT	CNetPlay::WndProc( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	// �G���[�H
	if( WSAGETSELECTERROR(lParam) ) {
#if	0
		switch( WSAGETSELECTERROR(lParam) ) {
			case	WSAEINTR:
				MessageBox( hWnd, "�֐��Ăяo�������f���ꂽ�B\n", "ERROR", MB_OK );
				break;
			case	WSAEBADF:
				MessageBox( hWnd, "�����ȃt�@�C���n���h���B\n", "ERROR", MB_OK );
				break;
			case	WSAEACCES:
				MessageBox( hWnd, "�A�N�Z�X�����ۂ��ꂽ�B\n", "ERROR", MB_OK );
				break;
			case	WSAEFAULT:
				MessageBox( hWnd, "�����ȃo�b�t�@�A�h���X�ł��B\n", "ERROR", MB_OK );
				break;
			case	WSAEINVAL:
				MessageBox( hWnd, "�����Ȉ������n���ꂽ�B\n", "ERROR", MB_OK );
				break;
			case	WSAEMFILE:
				MessageBox( hWnd, "�g�p���̃\�P�b�g�̐�����������B\n", "ERROR", MB_OK );
				break;
			case	WSAEWOULDBLOCK:
				MessageBox( hWnd, "����̓u���b�L���O�����B\n", "ERROR", MB_OK );
				break;
			case	WSAEINPROGRESS:
				MessageBox( hWnd, "���Ƀu���b�L���O�葱�������s����Ă���B\n", "ERROR", MB_OK );
				break;
			case	WSAEALREADY:
				MessageBox( hWnd, "�v�����ꂽ����͊��Ɏ��s���A�܂��͎��s�ς݁B\n", "ERROR", MB_OK );
				break;
			case	WSAENOTSOCK:
				MessageBox( hWnd, "�w�肳�ꂽ�\�P�b�g�������ł���B\n", "ERROR", MB_OK );
				break;
//			case	WSAEDESTADDREQ:
//				MessageBox( hWnd, "����̎��s�ɑ��M��A�h���X���K�v�B\n", "ERROR", MB_OK );
//				break;
			case	WSAEMSGSIZE:
				MessageBox( hWnd, "���b�Z�[�W�T�C�Y���傫������B\n", "ERROR", MB_OK );
				break;
			case	WSAEPROTOTYPE:
				MessageBox( hWnd, "�\�P�b�g�͗v�����ꂽ�v���g�R���ɓK�����Ă��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAENOPROTOOPT:
				MessageBox( hWnd, "�s���ȃv���g�R���I�v�V�����B\n", "ERROR", MB_OK );
				break;
			case	WSAEPROTONOSUPPORT:
				MessageBox( hWnd, "�v���g�R�����T�|�[�g����Ă��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAESOCKTNOSUPPORT:
				MessageBox( hWnd, "�w�肳�ꂽ�\�P�b�g�^�C�v�̓T�|�[�g����Ă��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAEOPNOTSUPP:
				MessageBox( hWnd, "�v�����ꂽ����̓T�|�[�g����Ă��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAEPFNOSUPPORT:
				MessageBox( hWnd, "�v���g�R���t�@�~�����T�|�[�g����Ă��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAEAFNOSUPPORT:
				MessageBox( hWnd, "�A�h���X�t�@�~�����T�|�[�g����Ă��Ȃ��B\n", "ERROR", MB_OK );
				break;

			case	WSAEADDRINUSE:
				MessageBox( hWnd, "�A�h���X�͊��Ɏg�p���ł���B\n", "ERROR", MB_OK );
				break;
			case	WSAEADDRNOTAVAIL:
				MessageBox( hWnd, "�����ȃl�b�g���[�N�A�h���X�B\n", "ERROR", MB_OK );
				break;
			case	WSAENETDOWN:
				MessageBox( hWnd, "�l�b�g���[�N���_�E�����Ă���B\n", "ERROR", MB_OK );
				break;
			case	WSAENETUNREACH:
				MessageBox( hWnd, "�w�肳�ꂽ�l�b�g���[�N�z�X�g�ɓ��B�ł��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAENETRESET:
				MessageBox( hWnd, "�l�b�g���[�N�ڑ����j�����ꂽ�B\n", "ERROR", MB_OK );
				break;

			case	WSAECONNRESET:
				MessageBox( hWnd, "�l�b�g���[�N�ڑ�������ɂ���Ĕj�����ꂽ�B\n", "ERROR", MB_OK );
				break;
			case	WSAENOBUFS:
				MessageBox( hWnd, "�o�b�t�@���s�����Ă���B\n", "ERROR", MB_OK );
				break;
			case	WSAEISCONN:
				MessageBox( hWnd, "�\�P�b�g�͊��ɐڑ�����Ă���B\n", "ERROR", MB_OK );
				break;
			case	WSAENOTCONN:
				MessageBox( hWnd, "�\�P�b�g�͐ڑ�����Ă��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAESHUTDOWN:
				MessageBox( hWnd, "�\�P�b�g�̓V���b�g�_�E������Ă���B\n", "ERROR", MB_OK );
				break;
			case	WSAETOOMANYREFS:
				MessageBox( hWnd, "�Q�Ƃ̐�����������B\n", "ERROR", MB_OK );
				break;

			case	WSAETIMEDOUT:
				MessageBox( hWnd, "�ڑ��v�����^�C���A�E�g�����B\n", "ERROR", MB_OK );
				break;
			case	WSAECONNREFUSED:
				MessageBox( hWnd, "�ڑ������ۂ��ꂽ�B\n", "ERROR", MB_OK );
				break;
			case	WSAELOOP:
				MessageBox( hWnd, "���[�v�B\n", "ERROR", MB_OK );
				break;
			case	WSAENAMETOOLONG:
				MessageBox( hWnd, "���O����������B\n", "ERROR", MB_OK );
				break;
			case	WSAEHOSTDOWN:
				MessageBox( hWnd, "�z�X�g���_�E�����Ă���B\n", "ERROR", MB_OK );
				break;
			case	WSAEHOSTUNREACH:
				MessageBox( hWnd, "�z�X�g�ւ̌o�H���Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAENOTEMPTY:
				MessageBox( hWnd, "�f�B���N�g������ł͂Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAEPROCLIM:
				MessageBox( hWnd, "���[�U�[�̐�����������B\n", "ERROR", MB_OK );
				break;
			case	WSAEDQUOT:
				MessageBox( hWnd, "�f�B�X�N�N�H�[�^�B\n", "ERROR", MB_OK );
				break;
			case	WSAESTALE:
				MessageBox( hWnd, "���s���悤�Ƃ�������͔p�~����Ă���B\n", "ERROR", MB_OK );
				break;


			case	WSAEREMOTE:
				MessageBox( hWnd, "�����[�g�B\n", "ERROR", MB_OK );
				break;
			case	WSASYSNOTREADY:
				MessageBox( hWnd, "�l�b�g���[�N�T�u�V�X�e�������p�ł��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAVERNOTSUPPORTED:
				MessageBox( hWnd, "Winsock.dll�̃o�[�W�������͈͊O�ł���B\n", "ERROR", MB_OK );
				break;
			case	WSANOTINITIALISED:
				MessageBox( hWnd, "WinSock�V�X�e��������������Ă��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAEDISCON:
				MessageBox( hWnd, "�V���b�g�_�E���������B\n", "ERROR", MB_OK );
				break;
			case	WSAENOMORE:
				MessageBox( hWnd, "�f�[�^�͂���ȏ㑶�݂��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSAECANCELLED:
				MessageBox( hWnd, "����͎������ꂽ�B\n", "ERROR", MB_OK );
				break;

			case	WSAEINVALIDPROCTABLE:
				MessageBox( hWnd, "�T�[�r�X�v���o�C�_�̊֐��e�[�u���������B\n", "ERROR", MB_OK );
				break;
			case	WSAEINVALIDPROVIDER:
				MessageBox( hWnd, "�T�[�r�X�v���o�C�_�������B\n", "ERROR", MB_OK );
				break;
			case	WSAEPROVIDERFAILEDINIT:
				MessageBox( hWnd, "�T�[�r�X�v���o�C�_�̏������Ɏ��s�����B\n", "ERROR", MB_OK );
				break;
			case	WSASYSCALLFAILURE:
				MessageBox( hWnd, "�V�X�e���R�[���Ɏ��s�����B\n", "ERROR", MB_OK );
				break;
			case	WSASERVICE_NOT_FOUND:
				MessageBox( hWnd, "�T�[�r�X��������Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSATYPE_NOT_FOUND:
				MessageBox( hWnd, "�^�C�v��������Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSA_E_NO_MORE:
				MessageBox( hWnd, "�f�[�^�͂���ȏ㑶�݂��Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSA_E_CANCELLED:
				MessageBox( hWnd, "�������L�����Z�����ꂽ�B\n", "ERROR", MB_OK );
				break;
			case	WSAEREFUSED:
				MessageBox( hWnd, "����͋��ۂ��ꂽ�B\n", "ERROR", MB_OK );
				break;
			case	WSAHOST_NOT_FOUND:
				MessageBox( hWnd, "�z�X�g��������Ȃ��B\n", "ERROR", MB_OK );
				break;
			case	WSATRY_AGAIN:
				MessageBox( hWnd, "�w�肳�ꂽ�z�X�g��������Ȃ��A�܂��̓T�[�r�X�ُ̈�B\n", "ERROR", MB_OK );
				break;
			case	WSANO_RECOVERY:
				MessageBox( hWnd, "�񕜕s�\�ȃG���[�����������B\n", "ERROR", MB_OK );
				break;
			case	WSANO_DATA:
				MessageBox( hWnd, "�v�����ꂽ�^�C�v�̃f�[�^���R�[�h��������Ȃ��B\n", "ERROR", MB_OK );
				break;

		}
#endif
		Disconnect();
		if( m_hWndMsg ) {
			::PostMessage( m_hWndMsg, WM_NETPLAY_CLOSE, 0, 0 );
		}
		return	0L;
	}

	BOOL	bOpt;
	BOOL	bArg;

	switch( WSAGETSELECTEVENT(lParam) ) {
		case	FD_ACCEPT:
//			DEBUGOUT( "Accepting...." );
			sockaddr_in	addr;
			int	len;
			len = sizeof(addr);
			m_SocketData = ::accept( m_SocketConnect, (sockaddr*)&addr, &len );
			if( m_SocketData == INVALID_SOCKET ) {
//				DEBUGOUT( "failed.\n" );
				CLOSESOCKET( m_SocketConnect );
				if( m_hWndMsg ) {
					::PostMessage( m_hWndMsg, WM_NETPLAY_ERROR, 0, 0 );
				}
				return	0L;
			}
//			DEBUGOUT( "done.\n" );

			// Nagle�A���S���Y���̖�����
			bOpt = TRUE;
			if( ::setsockopt( m_SocketData, IPPROTO_TCP, TCP_NODELAY, (const char*)&bOpt, sizeof(bOpt) ) == SOCKET_ERROR ) {
//				DEBUGOUT( "CNetPlay:setsockopt failed.\n" );
				CLOSESOCKET( m_SocketData );
				if( m_hWndMsg ) {
					::PostMessage( m_hWndMsg, WM_NETPLAY_ERROR, 0, 0 );
				}
				return	0L;
			}
			// �u���b�L���O���[�h�ݒ�
			bArg = TRUE;
			if( ::ioctlsocket( m_SocketData, FIONBIO, (unsigned long*)&bArg ) == SOCKET_ERROR ) {
//				DEBUGOUT( "CNetPlay:ioctlsocket failed.\n" );
				CLOSESOCKET( m_SocketData );
				if( m_hWndMsg ) {
					::PostMessage( m_hWndMsg, WM_NETPLAY_ERROR, 0, 0 );
				}
				return	0L;
			}
			::WSAAsyncSelect( m_SocketData, m_hWnd, WM_NETPLAY, FD_CLOSE );
			m_bConnect = TRUE;
			if( m_hWndMsg ) {
				::PostMessage( m_hWndMsg, WM_NETPLAY_ACCEPT, 0, 0 );
			}
			break;
		case	FD_CONNECT:
//			DEBUGOUT( "Connection done.\n" );
			::WSAAsyncSelect( m_SocketData, m_hWnd, WM_NETPLAY, FD_CLOSE );
			m_bConnect = TRUE;
			if( m_hWndMsg ) {
				::PostMessage( m_hWndMsg, WM_NETPLAY_CONNECT, 0, 0 );
			}
			break;
		case	FD_CLOSE:
//			DEBUGOUT( "Connection close.\n" );
			Disconnect();
			if( m_hWndMsg ) {
				::PostMessage( m_hWndMsg, WM_NETPLAY_CLOSE, 0, 0 );
			}
			break;
	}

	return	0L;
}

