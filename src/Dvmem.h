
// DvMem.h
#ifndef	_DvMem_h
#define	_DvMem_h

extern	void	   FreeAllocs( void );

extern	HGLOBAL	DVGAlloc( LPTSTR, UINT, DWORD );
extern	HGLOBAL  DVGFree( HGLOBAL );

extern	HGLOBAL	DVGlobalAlloc( UINT, DWORD );
extern	LPSTR	   DVGlobalLock( HGLOBAL );
extern	void	   DVGlobalUnlock( HGLOBAL );
extern	void	   DVGlobalFree( HGLOBAL );

#endif	// _DvMem_h
// eof - DvMem.h

