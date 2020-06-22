
// DvInit.h
#ifndef	_DvInit_h
#define	_DvInit_h

// GLOBAL variables
extern char szPalClass[];        // Class name of palette windows.
//extern	HWND ghMainWnd;
//extern	HINSTANCE	hDvInst;

BOOL InitMyDIB( HANDLE, LPSTR );
BOOL InitInstance( HANDLE, int );

void HelpIni( LPSTR );

#define  CMD_LINE    1
typedef struct tagMYREC {
   DWORD n;
   char  cmd[1];
} MYREC, * PMYREC;

#endif	// _DvInit_h
// eof - DvInit.h
