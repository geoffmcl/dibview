

// DvFile2.h
// Commenced May, 1997

#ifndef	_DvFile2_h
#define	_DvFile2_h

extern DWORD	DVSeekEnd( HANDLE hf );
extern HANDLE	DVOpenFile( LPSTR lpf, LPOFSTRUCT pof, UINT uStyle );
extern HANDLE	DVlclose( HANDLE hf );

// IDM_OPTION4
// Added Nov25, 1997
// FIX980430 - Changed and extended
// FIX20001129 - Use LINKED LIST instead of double zero terminated file list
typedef struct {
	BOOL		   bFLChg;     // if CHANGED - ie items deleted, reset MRU menu list
	DWORD		   dwFLCnt;	   // Count 1
//#ifdef   USELLIST
   PLIST_ENTRY pHead;      // original MRU list
   TCHAR       szFilBuf[264];
   TCHAR       szDirBuf[264];
   TCHAR       szTmpBuf[264];
	DWORD		   dwFL;
	DWORD		   dwFLC2;		// Count 2
	DWORD		   dwFL2;
	LPBOOL		lpFLGChg;	// Pointer to active change flag
	LPDWORD		lpdwGCnt;	// Pointer to active count
	LPDWORD		lpdwGOff;	// Pointer to active offset
} FLST;
typedef FLST FAR * LPFLST;

#endif	// _DvFile2_h
// eof - DvFile2.h

