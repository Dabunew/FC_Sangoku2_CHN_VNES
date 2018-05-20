//
// DirectSound class
//
//#include "DebugOut.h"
#include "DirectSound.h"
#include "COM.h"

CDirectSound	DirectSound;

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////
CDirectSound::SAMPLERATE CDirectSound::m_SampleRateTable[] = {
	11025, 8,
	22050, 8,
	44100, 8,
	48000, 8,
	11025, 16,
	22050, 16,
	44100, 16,
	48000, 16,
	0, 0
};

INT	CDirectSound::m_BufferSizeTable[] = {
	2, 3, 4, 5, 6, 7, 8, 9, 10, 0
};

CDirectSound::CDirectSound()
{
	m_lpDS        = NULL;
	m_lpDSPrimary = NULL;
	m_lpDSStream  = NULL;

#if	1
	m_SampleRate.Rate = 22050;
#else
	m_SampleRate.Rate = 44100;
#endif

//	m_SampleRate.Bits = 8;
	m_SampleRate.Bits = 16;
	m_BufferSize = 1;
//	m_BufferSize = 2;

	m_bStreamPlay  = FALSE;
	m_bStreamPause = FALSE;
}

CDirectSound::~CDirectSound()
{
	ReleaseDSound();
}

#define	COMUSE	TRUE

// DirectSound�̏�����
BOOL	CDirectSound::InitialDSound( HWND hWnd )
{
DSBUFFERDESC	dsbdesc;

	m_hWnd = hWnd;

	try {
		// DirectSound�I�u�W�F�N�g�̍쐬
#if	!COMUSE
		if( DirectSoundCreate( NULL, &m_lpDS, NULL ) != DS_OK ) {
			m_lpDS = NULL;
			throw	"CDirectSound:DirectSoundCreate failed.";
		}
#else
		// COM�I���p
		COM::AddRef();
		if( ::CoCreateInstance( CLSID_DirectSound, NULL, CLSCTX_ALL, IID_IDirectSound, (LPVOID*)&m_lpDS) != S_OK ) {
			m_lpDS = NULL;
			throw	"CDirectSound:CoCreateInstance failed.";
		}
		if( m_lpDS->Initialize( NULL ) != DS_OK )
			throw	"CDirectSound:IDirectSound->Initialize failed.";
#endif

		// �D�拦�����[�h�̐ݒ�
		if( m_lpDS->SetCooperativeLevel( hWnd, DSSCL_PRIORITY ) != DS_OK )
			throw	"CDirectSound:SetCooperativeLevel failed.";

		// �X�s�[�J�̐ݒ�
//		m_lpDS->SetSpeakerConfig( DSSPEAKER_COMBINED( DSSPEAKER_STEREO, DSSPEAKER_GEOMETRY_WIDE ) );

		// �v���C�}���o�b�t�@�̍쐬
		ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
		dsbdesc.dwSize = sizeof(DSBUFFERDESC);
//		dsbdesc.dwFlags       = DSBCAPS_CTRLVOLUME
//				      | DSBCAPS_PRIMARYBUFFER;
		dsbdesc.dwFlags       = DSBCAPS_PRIMARYBUFFER;
		dsbdesc.dwBufferBytes = 0;
		dsbdesc.lpwfxFormat   = NULL;
		if( m_lpDS->CreateSoundBuffer( &dsbdesc, &m_lpDSPrimary, NULL ) != DS_OK )
			throw	"CDirectSound:CreateSoundBuffer failed.";
	} catch( char *str ) {
		ReleaseDSound();
		::MessageBox( hWnd, str, "ERROR", MB_ICONERROR|MB_OK );

		return	FALSE;
	}

	return	TRUE;
}

// DirectSound�̊J��
void	CDirectSound::ReleaseDSound()
{
	ReleaseBuffer();
	// DirectSound�I�u�W�F�N�g�̊J��
	RELEASE( m_lpDSPrimary );
	if( m_lpDS ) {
		RELEASE( m_lpDS );
#if	COMUSE
		COM::Release();
#endif
	}

	m_hWnd = NULL;
}

// DirectSound�o�b�t�@�̍쐬
BOOL	CDirectSound::InitialBuffer()
{
DSBUFFERDESC	dsbdesc;
WAVEFORMATEX	pcmwf;

	try {
		if( !m_lpDSPrimary )
			throw "CDirectSound:DirectSound object uninitialized.";

		// �v���C�}���o�b�t�@��Wave�t�H�[�}�b�g��ݒ�(��Ƀ��m����)
		ZeroMemory( &pcmwf, sizeof(WAVEFORMATEX) );
		pcmwf.wFormatTag      = WAVE_FORMAT_PCM;
		pcmwf.nChannels       = 1;
		pcmwf.nSamplesPerSec  = (WORD)m_SampleRate.Rate;
		pcmwf.nBlockAlign     = (WORD)m_SampleRate.Bits/8;
		pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
		pcmwf.wBitsPerSample  = (WORD)m_SampleRate.Bits;

		if( m_lpDSPrimary->SetFormat( &pcmwf ) != DS_OK )
			throw	"CDirectSound:SetFormat failed.";

		// �X�g���[���Z�J���_���o�b�t�@�쐬
		if( m_BufferSize < 2 )
			m_BufferSize = 2;

		// �o�b�t�@�T�C�Y���̌v�Z
		m_dwDSBlockNum = m_BufferSize * 10;
		m_dwDSBlockSize = pcmwf.nAvgBytesPerSec * m_BufferSize / 60;
		m_dwDSBlockSize-= m_dwDSBlockSize % pcmwf.nBlockAlign;
		m_dwDSBufferSize = m_dwDSBlockSize * m_dwDSBlockNum;
		m_dwDSLastBlock = 0;

		ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
		dsbdesc.dwSize        = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags       = DSBCAPS_LOCSOFTWARE
				      | DSBCAPS_GETCURRENTPOSITION2
				      | DSBCAPS_GLOBALFOCUS;
		dsbdesc.dwBufferBytes = m_dwDSBufferSize;
		dsbdesc.lpwfxFormat   = &pcmwf;

		if( m_lpDS->CreateSoundBuffer( &dsbdesc, &m_lpDSStream, NULL ) != DS_OK )
			throw	"CDirectSound:CreateSoundBuffer failed.";

		LPBYTE	lpPtr;
		DWORD	dwBytes;
		if( m_lpDSStream->Lock( 0, m_dwDSBufferSize, (LPVOID*)&lpPtr, &dwBytes, NULL, NULL, 0 ) != DS_OK ) {
			throw	"CDirectSound:Lock failed.";
		} else {
			FillMemory( lpPtr, dwBytes, (BYTE)(m_SampleRate.Bits==8?128:0) );
			m_lpDSStream->Unlock( lpPtr, dwBytes, NULL, NULL );
		}
	} catch( char *str ) {
		ReleaseBuffer();

		::MessageBox( m_hWnd, str, "ERROR", MB_ICONERROR|MB_OK );

		return	FALSE;
	}

	return	TRUE;
}

// DirectSound�o�b�t�@�̊J��
void	CDirectSound::ReleaseBuffer()
{
	StreamStop();
	RELEASE( m_lpDSStream );
}

// �T���v�����O���[�g�̐ݒ�
BOOL	CDirectSound::SetSamplingRate( DWORD rate, DWORD bits )
{
INT	i;

	i = 0;
	while( m_SampleRateTable[i].Rate != 0 ) {
		if( m_SampleRateTable[i].Rate == rate
		 && m_SampleRateTable[i].Bits == bits ) {
			m_SampleRate.Rate = rate;
			m_SampleRate.Bits = bits;
			return	TRUE;
		}
		i++;
	}
	return	FALSE;
}

// �T���v�����O���[�g�̎擾
void	CDirectSound::GetSamplingRate( DWORD& rate, DWORD& bits )
{
	rate = m_SampleRate.Rate;
	bits = m_SampleRate.Bits;
}

// �X�g���[�~���O�Đ�
void	CDirectSound::StreamPlay()
{
	if( !m_lpDS || !m_lpDSStream )
		return;

	if( !m_bStreamPlay ) {
		// Buffer clear
		LPBYTE	lpPtr;
		DWORD	dwBytes;
		if( m_lpDSStream->Lock( 0, m_dwDSBufferSize, (LPVOID*)&lpPtr, &dwBytes, NULL, NULL, 0 ) != DS_OK ) {
			throw	"CDirectSound:Lock failed.";
		} else {
			FillMemory( lpPtr, dwBytes, (BYTE)(m_SampleRate.Bits==8?128:0) );
			m_lpDSStream->Unlock( lpPtr, dwBytes, NULL, NULL );
		}

		m_dwDSLastBlock = 0xFFFFFFFF;
		m_bStreamPlay  = TRUE;
		m_bStreamPause = FALSE;
		m_lpDSStream->SetCurrentPosition( 0 );
		m_lpDSStream->Play( 0, 0, DSBPLAY_LOOPING );
	}
}

// �X�g���[�~���O��~
void	CDirectSound::StreamStop()
{
	if( !m_lpDS || !m_lpDSStream )
		return;

	if( m_bStreamPlay ) {
		m_bStreamPlay  = FALSE;
		m_bStreamPause = FALSE;
		m_lpDSStream->Stop();

		// ���S��~�܂ő҂�
		DWORD	dwStatus;
		do {
			m_lpDSStream->GetStatus( &dwStatus );
		} while( dwStatus & DSBSTATUS_PLAYING );

		m_lpDSStream->SetCurrentPosition( 0 );
	}
}

// �X�g���[�~���O�|�[�Y
void	CDirectSound::StreamPause()
{
//	DEBUGOUT( "CDirectSound::StreamPause\n" );

	if( !m_lpDS || !m_lpDSStream )
		return;

	if( m_bStreamPlay ) {
		if( !m_bStreamPause ) {
			m_bStreamPause = TRUE;
			m_lpDSStream->Stop();
		}
	}
}

// �X�g���[�~���O���W���[��
void	CDirectSound::StreamResume()
{
//	DEBUGOUT( "CDirectSound::StreamResume\n" );

	if( !m_lpDS || !m_lpDSStream )
		return;

	if( m_bStreamPlay ) {
		if( m_bStreamPause ) {
			m_bStreamPause = FALSE;
			m_lpDSStream->Play( 0, 0, DSBPLAY_LOOPING );
		}
	}
}

// �X�g���[�~���O
BOOL	CDirectSound::GetStreamLockPosition( LPDWORD lpdwStart, LPDWORD lpdwSize )
{
static	BOOL	bLockHalf = FALSE;
DWORD	dwPlayPos, dwWritePos;

	if( m_lpDSStream->GetCurrentPosition( &dwPlayPos, &dwWritePos ) == DS_OK ) {
		if( (dwWritePos / m_dwDSBlockSize) != m_dwDSLastBlock ) {
			m_dwDSLastBlock = dwWritePos / m_dwDSBlockSize;
			dwWritePos = (((dwWritePos/m_dwDSBlockSize)+1)%m_dwDSBlockNum) * m_dwDSBlockSize;
			// ���b�N���ׂ��ꏊ
			*lpdwStart = dwWritePos;
			*lpdwSize = m_dwDSBlockSize;
			return	TRUE;
		}
	}

	return	FALSE;
}

BOOL	CDirectSound::StreamLock( DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID* lplpvPtr1, LPDWORD lpdwBytes1, LPVOID* lplpvPtr2, LPDWORD lpdwBytes2, DWORD dwFlags )
{
	if( m_lpDSStream->Lock( dwWriteCursor, dwWriteBytes, lplpvPtr1, lpdwBytes1, lplpvPtr2, lpdwBytes2, dwFlags ) == DS_OK )
		return	TRUE;
	return	FALSE;
}

BOOL	CDirectSound::StreamUnlock( LPVOID lpvPtr1, DWORD dwBytes1, LPVOID lpvPtr2, DWORD dwBytes2 )
{
	if( m_lpDSStream->Unlock( lpvPtr1, dwBytes1, lpvPtr2, dwBytes2 ) == DS_OK )
		return	TRUE;
	return	FALSE;
}

