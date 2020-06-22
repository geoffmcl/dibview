
// DvAbout.h
#ifndef	_DvAbout_h
#define	_DvAbout_h

#define szAboutDLG "AboutDlg"          // About Dlg box template in .RC file.

INT_PTR CALLBACK ABOUTDLG(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

// BOOL MLIBCONV ABOUTDLG(HWND hDlg, unsigned message, WORD wParam, LONG lParam);

#endif	// _DvAbout_h
// eof - DvAbout.h
