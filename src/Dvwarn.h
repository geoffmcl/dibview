

// DvWarn.h
#ifndef	_DvWarn_h
#define	_DvWarn_h

// for the IDD_DVERROR dialog template
typedef struct tagWARNSTR {

	LPTSTR	lpText;     // pointer to the TEXT *** MUST BE GIVEN ***
	LPTSTR	lpTitle;    // pointer to the TITLE (if any)
	BOOL	   bJustOK;    // just single OK button
	BOOL	   bCheck;
	BOOL	   bChgCheck;
	BOOL	   bAddCheck;  // to add a check item
   BOOL     bAddBrowse; // add a BROWSE button
   LPTSTR   pszFile;    // file name pointer - if needed
   LPTSTR   pszPath;    // path pointer - if needed

}WARNSTR, * PWARNSTR;
typedef WARNSTR FAR * LPWARNSTR;

#define		NULWARN(a)  ZeroMemory( &a, sizeof(WARNSTR) )

#define     ATOM_INFO      0x117
#define		ATOM_SAVEAS		0x116
#define     ATOM_HDI       0x115
#define     ATOM_LPWN      0x114
#define     ATOM_LPFL      0x113

#define SET_PROP( x, y, z )  SetProp( x, MAKEINTATOM( y ), (HANDLE)z )
#define GET_PROP( x, y )     GetProp( x, MAKEINTATOM( y ) )
#define REMOVE_PROP( x, y )  RemoveProp( x, MAKEINTATOM( y ) )

// DvWarn.c
extern	UINT ShowWarning( HWND hWnd, LPWARNSTR lpWS );

#endif	// _DvWarn_h
// eof - DvWarn.h
