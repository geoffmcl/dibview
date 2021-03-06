
//
// PROJECT: WJPG32_2.DLL
//
// MODULE: WMemDiag.h
//
// This is a COPY of DvMemF.h used in the application,
//	thus MUST be kept the SAME, or COMPATIBLE.
//

// DvMemF.h
#ifndef	_DVMEMF_H
#define	_DVMEMF_H

#ifdef  __cplusplus
extern  "C"
{
#endif  // __cplusplus

#define		mf_TurnOn		0x10000000	// ON, else OFF

	// LIST OF FAR PASCAL FUNCTIONS
	// ============================
typedef struct tagJPEG_EXTMM {
	DWORD		dwMM_Size;
	void MLPTR	(*EXTLOCALALLOC)	( DWORD dwSz );
	HGLOBAL		(*EXTLOCALFREE)		( void MLPTR );
	HGLOBAL		(*EXTGLOBALALLOC)	( DWORD dwSz );
	void MLPTR	(*EXTGLOBALLOCK)	( HGLOBAL hg );
	void MLPTR	(*EXTGLOBALUNLOCK)	( HGLOBAL hg );
	HGLOBAL		(*EXTGLOBALFREE)	( HGLOBAL hg );
	void		(*EXTDEBUGSTOP)		( int code );
	void		(*EXTTRACEOUT)		( LPSTR lpt );
}JPEG_EXTMM;

typedef JPEG_EXTMM MLPTR LPJPEG_EXTMM;

EXPORT32
BOOL MLIBCALL WEXTMMGR( LPJPEG_EXTMM lpMM, DWORD dwFlg );
typedef BOOL (MLIBCONV *LPWEXTMMGR) ( LPJPEG_EXTMM, DWORD );

// Close the CPP Block
#ifdef  __cplusplus
}
#endif  // __cplusplus
#endif	// _DVMEMF_H

// eof - DvMemF.h
// eof - WMemDiag.h
