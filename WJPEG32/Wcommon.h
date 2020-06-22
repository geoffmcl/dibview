
#ifndef  _WCommon_H
#define  _WCommon_H

/* wcommon.h - Shared routines ... */

#ifdef	WIN32
#define	gw_fstrcpy( a, b )	strcpy( a, b )
#define	gw_fstrlen(x)		strlen(x)
#else	// !WIN32
#define	gw_fstrcpy( a, b )	_fstrcpy( a, b )
#define	gw_fstrlen(x)		_fstrlen(x)
#endif	// WIN32 y/n

extern	FILETYPE in_open( LPSTR filename, LPSTR mode );
extern	FILETYPE out_open( LPSTR filename, LPSTR mode );

extern	void winp_term( void );
#ifdef	WIN32
extern	void setexitjump( void MLPTR, DWORD );
#else	// !WIN32
extern	void setexitjump( void FAR *, WORD );
#endif	// WIN32 y/n
extern	int GetErrorMsg( LPSTR, WORD );
extern	int GetWarnMsg( LPSTR );

#endif   // ifndef  _WCommon_H
/* eof - WCommon.h */
