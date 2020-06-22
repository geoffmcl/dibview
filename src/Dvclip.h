
// DvClip.h
#ifndef	_DvClip_h
#define	_DvClip_h

// Macros to display/remove hourglass cursor for lengthy operations
#define StartWait() hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT))
#define EndWait()   SetCursor(hcurSave)

HANDLE     CopyHandle           (HANDLE h);
HBITMAP    CopyBitmap           (HBITMAP hbm);
HBITMAP    CropBitmap           (HBITMAP hbm, HPALETTE hPal, LPRECT lpRect, LPPOINT lpptSize);
HANDLE     RenderFormat         (HWND hWndClip, int cf, POINT ptDIBSize);
HANDLE     RealizeDibFormat     (DWORD biStyle, WORD biBits);

extern HWND  HandlePasteClipboard( VOID );
extern HWND HandleCopyClipboard( VOID );
extern void ChildWndLeftButton( HWND hWnd, int x, int y );
extern void DrawSelect( HDC hDC, LPRECT lprcClip );
extern RECT	rcPrevClip; // = { 0 };
extern VOID ChildWndMouseMove( HWND hWnd, WPARAM wParam, LPARAM lParam );
extern BOOL InitCursors ( VOID );

#endif	// _DvClip_h
// eof - DvClip.h
