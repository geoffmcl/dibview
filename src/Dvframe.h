
// DvFrame.h
#ifndef	_DvFrame_h
#define	_DvFrame_h

//long MLIBCONV FRAMEWNDPROC(HWND hWnd, 
//			     UINT message,
//			     WPARAM wParam,
//			     LPARAM lParam);
LRESULT CALLBACK FRAMEWNDPROC(HWND hWnd,      // handle to window
                              UINT uMsg,      // message identifier
                              WPARAM wParam,  // first message parameter
                              LPARAM lParam   // second message parameter
);

extern	HWND OpenDIBWindow2( PRDIB );
extern	HWND OpenDIBWindow( HANDLE, LPSTR, DWORD );

void EnableWindowAndOptionsMenus (BOOL bEnable);

#endif	// _DvFrame_h
// eof - DvFrame.h

