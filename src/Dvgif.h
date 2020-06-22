
// DvGif.h - ChooseGIF in GetGif.C
#ifndef	_DvGif_h
#define	_DvGif_h

typedef	struct tagGIFSTR {
	HGLOBAL	hgGifs;
	LPSTR		lpGFile;	   // Name of FILE
	BOOL		fGAll;		// Load them ALL
	BOOL		fGNone;		// Load NONE = NOTE: both TRUE = ANIMATE
	BOOL		fGComb;		// Combine GIFs into ONE Window
	WORD		wGNum;		// Number
	HGLOBAL	hgFile;		// GIF File
	DWORD		dwFSize;	   // FILE Size
   TCHAR    szFile[264];   // pad for file name (if used)
   DWORD    dwMS;       // ms count for animation
}GIFSTR, * PGIFSTR;
typedef GIFSTR MLPTR LPGIFSTR;

extern	void ChooseGIF( LPGIFSTR );

#define	MXGIFIMG		100

#endif	// _DvGif_h
// eof - DvGif.h
