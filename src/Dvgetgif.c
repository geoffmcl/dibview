
// =========================================================================
//
//	File:  DVGETGIF.C
//
//	Purpose:  Contains the handlers for the "SELECT GIF" Dialog box
//
//	Comments:
//
// History:
//	Date     Reason
//	19 April 1996	Created                           Geoff R. McLane
//  29 April 1997	Added DumpGIF - Dump the lpGHE/lpGIE structures
//			to a TEMPGIF.TXT file, IFF NDEBUG is not defined.
//	ie the WJPEG4 release!
// ======================================================================== */
#include	"dv.h"	/* Single, all inclusing include ... except 'specials' */

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
//#include "wjpeglib.h"	/* ONLY module to include this ... ALL calls from here */
//#include	"WJPEG32\WJPEGLIB.h"
#include	"DvGif.h"

extern	BOOL	fNetScape;

#ifdef	WJPEG4
extern	BOOL	GIFCountExt( HGLOBAL hgG, DWORD ddSz, HGLOBAL hGHE );
#endif	// WJPEG4

#ifdef	ADDTIMER1
extern	UINT	GotTimer;
#else
BOOL	GotTimer = FALSE;
#endif	/* ADDTIMER1 y/n */

extern	BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
extern	DWORD DVWrite(HANDLE fh, PINT8 pv, DWORD ul );
extern	BOOL	FindNewName( LPSTR );
extern	LPSTR	GetDT4( int );

INT_PTR CALLBACK GETGIFDLG(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);
// BOOL MEXPORTS GETGIFDLG( HWND, UINT, WORD, LONG );
BOOL	GetGifInit( HWND, LPARAM );
BOOL	GetGifNum( HWND );
BOOL	GetGifAll( HWND );
BOOL	GetGifNone( HWND );
BOOL	GetGifAnim( HWND );

//char	szGetGif[] = "CHOOSEGIF";  // NOT_USED!
char	szGetGif2[] = "CHOOSEGIF2";
LPGIFSTR	lpGifStr;
WORD	wMaxCnt;

/* ================================================================
 *
 * Function:   ChooseGIF( )
 * Purpose:    Brings up "CHOOSEGIF2" dialog box
 * Parms:      hWnd    == Handle to options dialog box's parent.
 * History:
 *   Date      Reason
 *  5 March 96 Created
 *
 * ================================================================ */

void ChooseGIF( LPGIFSTR lpG )
{
#ifndef	WIN32
	FARPROC lpProcOpt;
#endif
	LPCSTR	lpcs;

	if( (fNetScape == 2) && GotTimer )
		lpcs = szGetGif2; // = char "CHOOSEGIF2";
	else
		lpcs = MAKEINTRESOURCE(IDD_CHOOSEGIF2);

	//		lpcs = szGetGif;
#ifdef	WIN32
	DialogBoxParam( ghDvInst,
		lpcs,
		GetFocus(),
		GETGIFDLG,	// lpProcOpt,
		(LPARAM) lpG );
#else
	lpProcOpt = MakeProcInstance( GETGIFDLG, ghDvInst );
	DialogBoxParam( ghDvInst,
		lpcs,
		GetFocus(),
		lpProcOpt,
		(LPARAM) lpG );
	FreeProcInstance( lpProcOpt );
#endif

}

/* =====================================================================
 * GETGIFDLG( ... ) using "CHOOSEGIF2" or IDD_CHOOSEGIF2
 * ===================================================================== */
//BOOL MEXPORTS GETGIFDLG( HWND hDlg, UINT message, 
//	WORD wParam, LONG lParam )
INT_PTR CALLBACK GETGIFDLG(
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
   DWORD cmd;
	switch( message )
	{
		case WM_INITDIALOG:
			return GetGifInit( hDlg, lParam );

		case WM_COMMAND:
         cmd = LOWORD(wParam);
			switch( cmd )
			{
				case IDD_FBAL:    // this the "All" button
					GetGifAll( hDlg );
					EndDialog(hDlg, TRUE);
					return( TRUE );

				case IDD_COMBINED:
#ifdef	TRANSGIF
					lpGifStr->fGAll = TRUE;
					lpGifStr->fGNone = FALSE;
					lpGifStr->fGComb = TRUE;
					EndDialog(hDlg, TRUE);
#else	// !TRANSGIF
					MessageBox( hDlg, "ERROR: Not yet implmented", "NOT IMPLMENTED", MB_OK | MB_ICONINFORMATION );
#endif	// TRANSGIF y/n
					break;

				case IDM_PALANIMATE:    // this is the Animate button
					GetGifAnim( hDlg );
					EndDialog(hDlg, TRUE);
					return( TRUE );

				case IDD_CANCEL:        // this is the "Cancel" button
				case IDCANCEL:        // this is the system "Cancel" button
					GetGifNone( hDlg );
					EndDialog(hDlg, FALSE);
					return( TRUE );

            case IDD_OK:            // this is the "Ok" button
					if( GetGifNum( hDlg ) )	/* Get values, and fall through CANCEL */
						break;	/* If TRUE, then stay in Dialog to try fix */
					EndDialog(hDlg, TRUE);
					return( TRUE );

			}
	}
   return( FALSE );
}

DWORD	GetSizeCSL( void )
{
	return( 2248 );
}

void	CheckStringLen( HWND hDlg, UINT id, LPSTR lps, int len, int max )
{
	HWND	hText;
	HDC	hdc, hdcm;
	RECT	rc;
	DWORD	ext1, ext2;
	int	i, j, k, l;
	char	c;
	HGLOBAL	hG;
	LPSTR	lpG, lpG2;
#ifdef	WIN32
	SIZE	tSize;	// size structure pointer
#endif	// WIN32

	hdcm = 0;
	hdc = 0;
	hG = 0;
	lpG = 0;
	if( (i = len) &&
		( hG = DVGlobalAlloc( GHND, GetSizeCSL() ) ) &&
		( lpG = DVGlobalLock( hG ) ) )
	{
			lpG2 = lpG + 1024;
			if( ( hText = GetDlgItem( hDlg, id ) ) &&
				( hdc = GetDC( hText ) ) &&
				( GetDeviceCaps( hdc, RASTERCAPS ) & RC_BITBLT ) &&
				( hdcm = CreateCompatibleDC( hdc ) ) )
			{
				GetClientRect( hText, &rc );
#ifdef	WIN32
//BOOL GetTextExtentPoint(
//    HDC hdc,	// handle of device context 
//    LPCTSTR lpString,	// address of text string 
//    int cbString,	// number of characters in string 
//    LPSIZE lpSize 	// address of structure for string size 
////);	
				if( GetTextExtentPoint( hdc, lps, i, &tSize ) )
				{
					ext1 =  (((WORD)tSize.cy << 16) | ((WORD)tSize.cx) );
				}
#else	// !WIN32
				ext1 = GetTextExtent( hdcm, lps, i );
#endif	// WIN32 y/n
				if( ( rc.right > 50 ) &&
					( LOWORD( ext1 ) > (WORD) rc.right ) )
				{
					l = k = 0;
					for( j = 0; j < i; j++ )
					{
						c = lps[j];
						lpG[j] = c;
						if( c == '\\' )
							k = j;
					}
NxtLen:
					l = k;
					if( k &&( (k + 1) < i ) )
					{	/* We found an directory ... */
						k++;
						lpG[k] = 0;	/* Put after DIRECTORY ... */
#ifdef	WIN32
//BOOL GetTextExtentPoint(
//    HDC hdc,	// handle of device context 
//    LPCTSTR lpString,	// address of text string 
//    int cbString,	// number of characters in string 
//    LPSIZE lpSize 	// address of structure for string size 
////);	
				if( GetTextExtentPoint( hdc, lps, i, &tSize ) )
				{
					ext2 =  (((WORD)tSize.cy << 16) | ((WORD)tSize.cx) );
				}
#else	// !WIN32
				ext2 = GetTextExtent( hdcm, lps, i );
#endif	// WIN32 y/n
						if( LOWORD( ext2 ) < LOWORD( ext1 ) )
						{
							if( LOWORD( ext2 ) > (WORD) rc.right )
							{
								k = 0;
								for( j = 0; j < i; j++ )
								{
									c = lpG[j];
									lpG2[j] = c;
									if( c == '\\' )
										k = j;
								}
								if( k && ( (k + 1) < i ) )
								{
									lpG2[k+1] = 0;
									lstrcpy( lpG, lpG2 );
									goto NxtLen;
								}
							}						
							else
							{	/* Success ... */
								lstrcat( lpG, MEOR );	/* Add a line break ... */
								l = lstrlen( lpG );	/* Get its NEW length ... */
								for( ; k < i; k++ )	/* and move BALANCE ... */
								{
									c = lps[k];
									lpG[l++] = c;
								}
								lpG[l] = 0;	/* Zero terminate ... */
								if( l < max )	/* and IF size permits ... */
									lstrcpy( lps, lpG );	/* Put in the new string */
							}
						}
					}
				}
			}
			if( hdcm )
				DeleteDC( hdcm );
			if( hdc )
				ReleaseDC( hText, hdc );
	}
	if( hG && lpG )
		DVGlobalUnlock( hG );
	if( hG )
		DVGlobalFree( hG );
}

DWORD	GetSizeGHE( void )
{
	return( sizeof( GIFHDREXT ) );
}

/* =====================================================================
 * GetGifInit( ... )
 * ===================================================================== */
BOOL	GetGifInit( HWND hDlg, LPARAM lParam )
{
	BOOL	flg;
	HGLOBAL	hg;
	LPGIFHDR	lpgh;
	WORD	icnt;
	LPSTR	lps1, lps2, lpf;
	int	i;
	HWND	hOwnr;
	flg = FALSE;
	lpGifStr = 0;
	hg = 0;
	lpgh = 0;
	lps1 = GetTmp1();
	lps2 = GetTmp2();
	if( hOwnr = GetWindow( hDlg, GW_OWNER ) )
	{
		CenterWindow( hDlg, hOwnr );
	}
	if( lpGifStr = (LPGIFSTR) lParam )
	{
		lpGifStr->fGNone = TRUE;	/* Start with NONE requested ... */
		if( lpGifStr->wGNum == 0 )
			lpGifStr->wGNum = 1;
		if( ( hg = lpGifStr->hgGifs ) &&
			( lpgh = (LPGIFHDR) DVGlobalLock( hg ) ) &&
			( icnt = lpgh->ghImgCount ) )
		{
			wMaxCnt = icnt;
			if( (lpf = lpGifStr->lpGFile) &&
				(GetDlgItemText( hDlg, IDD_FNAME, lps1, MXTSTG )) )
				wsprintf( lps2, lps1, lpf );
			else
				lps2[0] = 0;
			if( i = lstrlen( lps2 ) )
				CheckStringLen( hDlg, IDD_FNAME, lps2, i, MXTSTG );
			SetDlgItemText( hDlg, IDD_FNAME, lps2 );

			if( GetDlgItemText( hDlg, IDD_FSIZE, lps1, MXTSTG ))
			{
				wsprintf( lps2, lps1, icnt );
#ifdef	WJPEG4
				if( lpGifStr->hgFile && lpGifStr->dwFSize )
				{
					HGLOBAL	hgGHE;
					LPGIFHDREXT	lpGHE;
					if( hgGHE = DVGlobalAlloc( GHND, GetSizeGHE() ) )
					{
						if( lpGHE = (LPGIFHDREXT)DVGlobalLock( hgGHE ) )
						{
							lpGHE->gheMaxCount = gie_Flag;	// This is in the ghMaxCount
							if( GIFCountExt( lpGifStr->hgFile, lpGifStr->dwFSize, hgGHE ) == 0 )
							{
								if( lpGHE->gheImgCount > icnt )
								{
									wsprintf( (lps2 + lstrlen( lps2 )),
										" (+ %u Text)",
										(lpGHE->gheImgCount - icnt) );
								}
							}
							DVGlobalUnlock( hgGHE );
						}
						DVGlobalFree( hgGHE );
					}
				}
#endif	// WJPEG4
			}
			else
			{
				lps2[0] = 0;
			}

			SetDlgItemText( hDlg, IDD_FSIZE, lps2 );

			SetDlgItemInt( hDlg, IDD_DEFAULT, lpGifStr->wGNum, FALSE );

			flg = TRUE;	/* Default to all OK ... */
		}
	}
	if( hg && lpgh )
		DVGlobalUnlock( hg );

	return( flg );

}

BOOL	GetGifNum( HWND hDlg )
{
BOOL	flg;
WORD	gnum;
int	i;
	gnum = GetDlgItemInt( hDlg, IDD_DEFAULT, &i, FALSE );
	if( ( i == 0 ) || (gnum == 0) || (gnum > wMaxCnt) )
	{
		flg = TRUE;
	}
	else
	{
		lpGifStr->fGAll = FALSE;
		lpGifStr->fGNone = FALSE;
		lpGifStr->wGNum = gnum;
		flg = FALSE;
	}
return( flg );
}

BOOL	GetGifAll( HWND hDlg )
{
BOOL	flg;
		lpGifStr->fGAll = TRUE;
		lpGifStr->fGNone = FALSE;
		flg = FALSE;
return( flg );
}

BOOL	GetGifNone( HWND hDlg )
{
BOOL	flg;
		lpGifStr->fGAll = FALSE;
		lpGifStr->fGNone = TRUE;
		flg = FALSE;
return( flg );
}

BOOL	GetGifAnim( HWND hDlg )
{
BOOL	flg;
		lpGifStr->fGAll = TRUE;
		lpGifStr->fGNone = TRUE;
		flg = FALSE;
return( flg );
}


#ifdef	GIFDIAGS
//
// Ouput to TEMPGIF.TXT, the contents of the lpGHE/lpGIE structs
// =============================================================
#pragma message( "See TEMPG???.TXT - ADVICE: GIF Diagnostic is ***ON***!" )

#define	ADDDC

#define	wrtit		lstrcat( lpb, MEOR );\
					DVWrite( IntToPtr(hf), lpb, lstrlen( lpb ))

#define	sprt( a, b )\
{\
	wsprintf( lpb, a, b );\
	wrtit;\
}

char	szDefGif[] = "TEMPGIF.TXT";
// Current and Next NAME of FILE
//char	szTmpGif[MAX_PATH+8] = "\0";
//HFILE	hGifFile = 0;
//int		iTwipsPerInch = 1440;
//extern	int		iHPPI;	//  = GetDeviceCaps( hDC, LOGPIXELSX );
//extern	int		iVPPI;	//  = GetDeviceCaps( hDC, LOGPIXELSY );

// GetDC( hMainWnd );

#ifdef	ADDDC

typedef struct	DCATTRS {	/* dc */
	TEXTMETRIC	dctm;
	int		c_MapMode;
	int		n_MapMode;
	HFONT	hOldFont;
	HFONT	hNewFont;
	int		c_TextAlign;
	int		n_TextAlign;
	int		xChar;
	int		yChar;
	UINT	c_BlockMode;
	UINT	n_BlockMode;
	BOOL		f_SetBG;
	COLORREF	c_BG_Colr;
	COLORREF	n_BG_Colr;
	BOOL		f_SetFG;
	COLORREF	c_FG_Colr;
	COLORREF	n_FG_Colr;
}DCATTRS;

typedef DCATTRS MLPTR LPDCATTRS;
	
void	DoDCAttrs( HDC hDC, LPDCATTRS lpDCAttrs )
{
	lpDCAttrs->hOldFont = 0;
	if( lpDCAttrs->hNewFont )
		lpDCAttrs->hOldFont = SelectObject( hDC, lpDCAttrs->hNewFont );

	lpDCAttrs->c_MapMode = GetMapMode( hDC );
	if( lpDCAttrs->c_MapMode != lpDCAttrs->n_MapMode )
		SetMapMode( hDC, lpDCAttrs->n_MapMode );
	lpDCAttrs->c_TextAlign = GetTextAlign( hDC );
	if( lpDCAttrs->c_TextAlign != lpDCAttrs->n_TextAlign )
		SetTextAlign( hDC, lpDCAttrs->n_TextAlign );
	lpDCAttrs->c_BlockMode = GetBkMode( hDC );
	if( lpDCAttrs->c_BlockMode != lpDCAttrs->n_BlockMode )
		SetBkMode( hDC, lpDCAttrs->n_BlockMode );

	if( lpDCAttrs->f_SetBG )
		lpDCAttrs->c_BG_Colr = SetBkColor( hDC, lpDCAttrs->n_BG_Colr );

	if( lpDCAttrs->f_SetFG )
		lpDCAttrs->c_FG_Colr = SetTextColor( hDC, lpDCAttrs->n_FG_Colr );

	GetTextMetrics( hDC, &lpDCAttrs->dctm );
	lpDCAttrs->xChar = lpDCAttrs->dctm.tmAveCharWidth;
	lpDCAttrs->yChar = (lpDCAttrs->dctm.tmHeight +
		lpDCAttrs->dctm.tmExternalLeading);

}

void	ResetDCAttrs( HDC hDC, LPDCATTRS lpDCAttrs )
{
	if( lpDCAttrs->hOldFont )
		SelectObject( hDC, lpDCAttrs->hOldFont );

	if( lpDCAttrs->c_MapMode != lpDCAttrs->n_MapMode )
		SetMapMode( hDC, lpDCAttrs->c_MapMode );

	if( lpDCAttrs->c_TextAlign != lpDCAttrs->n_TextAlign )
		SetTextAlign( hDC, lpDCAttrs->c_TextAlign );

	if( lpDCAttrs->c_BlockMode != lpDCAttrs->n_BlockMode )
		SetBkMode( hDC, lpDCAttrs->c_BlockMode );

	if( lpDCAttrs->f_SetBG )
		SetBkColor( hDC, lpDCAttrs->c_BG_Colr );

	if( lpDCAttrs->f_SetFG )
		SetTextColor( hDC, lpDCAttrs->c_FG_Colr );

}

/* ===================
								if( !fta )
								{
									ta = GetTextAlign( hDC );
									ffta = ChkTextAlign( hDC );
									fta = TRUE;
								}
								if( lpbmh = DVGlobalLock( hDIB ) )
								{
									int	index;
									lpq = (LPRGBQUAD)(lpbmh + *(LPDWORD)lpbmh);
									index = lpGIE->gceRes2;
									fgcolr = RGB( lpq[index].rgbRed,
										lpq[index].rgbGreen,
										lpq[index].rgbBlue );
//			lpGIE->gceRes2 = (DWORD)lpb[10];	// Foreground index
									index = lpGIE->gceIndex;
//			lpGIE->gceIndex = lpb[11];	// Backgound INDEX
//	DWORD	gceColr;	// COLORREF (if SET)
									bgcolr = RGB( lpq[index].rgbRed,
										lpq[index].rgbGreen,
										lpq[index].rgbBlue );
									DVGlobalUnlock( hDIB );
								}
								BkMode = GetBkMode( hDC );
								//opts = ETO_CLIPPED;
								opts = 0;
								if( lpGIE->gceBits & gce_TransColr )	// Transparent Color Flag 1 Bit
								{
									SetBkMode( hDC, TRANSPARENT );
								}
								else
								{
									SetBkMode( hDC, OPAQUE ); 
									SetBkColor( hDC, bgcolr );
									opts |= ETO_OPAQUE;
								}
								SetTextColor( hDC, fgcolr );
								crect.left = (lpGIE->giLeft & 0xffff);
								crect.top  = (lpGIE->giTop & 0xffff);
								crect.right = crect.left + GetHPixels( lpGIE ); //(lpGIE->giGI.giWidth & 0xffff);
								crect.bottom = crect.top + GetVPixels( lpGIE ); //(lpGIE->giGI.giHeight & 0xffff);
								GetTextMetrics( hDC, &tm );
								xChar = tm.tmAveCharWidth;
								yChar = tm.tmHeight + tm.tmExternalLeading;
 ============= */
#define	MXTPL		256
#define	MXLINES		52

extern	int	GetHPixels( LPGIFIMGEXT ); //(lpGIE->giGI.giWidth & 0xffff);
extern	int GetVPixels( LPGIFIMGEXT ); //(lpGIE->giGI.giHeight & 0xffff);

void	ShowTxtInfo( LPGIFHDREXT lpGHE,	// Header
					LPGIFIMGEXT lpGIE,	// Image
					LPSTR lptt,			// Text
					int bbl,			// Position of LONGEST
					HFILE	hf )
{
	LOGFONT	cursfont;
	HANDLE	hnewfont;
	HDC	hDC;
	DCATTRS	DCAttrs;
	char	buf1[MXTPL+4];
	char	buf2[MXTPL+4];
	LPSTR	lps, lpb;
	RECT	crect;
	BYTE	cnt;
	LPBYTE	lpt;
	SIZE	sz[MXLINES+2];
	SIZE	sz2;

	lps = &buf1[0];
	lpb = &buf2[0];
	if( hDC = GetDC( ghMainWnd ) )
	{
		hnewfont = 0;
		// Build fixed screen font.
		if( ( cursfont.lfHeight = HIBYTE(LOWORD(lpGIE->gceRes1)) ) &&
			( cursfont.lfWidth  = LOBYTE(LOWORD(lpGIE->gceRes1)) ) )
		{
			cursfont.lfEscapement     =  0;
			cursfont.lfOrientation    =  0;
			cursfont.lfWeight         =  FW_NORMAL;
			cursfont.lfItalic         =  FALSE;
			cursfont.lfUnderline      =  FALSE;
			cursfont.lfStrikeOut      =  FALSE;
			cursfont.lfCharSet        =  ANSI_CHARSET;
			cursfont.lfOutPrecision   =  OUT_DEFAULT_PRECIS;
			cursfont.lfClipPrecision  =  CLIP_DEFAULT_PRECIS;
			cursfont.lfQuality        =  DEFAULT_QUALITY;
			//cursfont.lfPitchAndFamily =  FIXED_PITCH | FF_DONTCARE;
			cursfont.lfPitchAndFamily =  DEFAULT_PITCH | FF_DONTCARE;
			//strcpy(cursfont.lfFaceName, "System");
			cursfont.lfFaceName[0] = 0;
			if( hnewfont = CreateFontIndirect( &cursfont ) )
			{
				//==========================
				DCAttrs.n_MapMode = MM_TEXT;
				DCAttrs.hNewFont  = hnewfont;
				if( lpGIE->gceBits & gce_TransColr )	// Transparent Color Flag 1 Bit
					DCAttrs.n_BlockMode = TRANSPARENT;
				else
					DCAttrs.n_BlockMode = OPAQUE;
				DCAttrs.n_TextAlign = TA_TOP | TA_LEFT | TA_NOUPDATECP;
				DCAttrs.f_SetBG = FALSE;
				DCAttrs.f_SetFG = FALSE;
				DoDCAttrs( hDC, &DCAttrs );
				crect.left = (lpGIE->giLeft & 0xffff);
				crect.top  = (lpGIE->giTop & 0xffff);
				crect.right = crect.left + GetHPixels( lpGIE ); //(lpGIE->giGI.giWidth & 0xffff);
				crect.bottom = crect.top + GetVPixels( lpGIE ); //(lpGIE->giGI.giHeight & 0xffff);
				if( lpt = lptt )
				{
					int		i, j, k, pk;
					char	c;
					int		cumht;
					int		lc;

					k = 0;
					pk = 0;
					lc = 0;
					cumht = 0;
					sz[0].cx = 0;
					sz[0].cy = 0;
					while( cnt = (*lpt & 0xff) )
					{
						lpt++;
						j = (int)(cnt & 0xff);
						for( i = 0; i < j; i++ )
						{
							c = lpt[i];	// Get char
							if( c >= ' ' )
							{
								if( k < MXTPL )
								{
									lps[k++] = c;
									lps[k] = 0;
								}
							}
							else
							{
								if( c == 0x0d )
								{
									cumht += DCAttrs.yChar;
									lc++;
								}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								if( k &&
									(lc < MXLINES) )
								{
#ifdef	WIN32
									if( !GetTextExtentPoint32( hDC, // handle of device context
										lps,	// address of text string
										k,		// number of characters in string
										&sz[lc] ) )	// address of structure for string size
									{
										sz[lc].cx = 0;
										sz[lc].cy = 0;
									}
#else	// !WIN32
									if( !GetTextExtentPoint( hDC, // handle of device context
										lps,	// address of text string
										k,		// number of characters in string
										&sz[lc] ) )	// address of structure for string size
									{
										sz[lc].cx = 0;
										sz[lc].cy = 0;
									}
#endif	// WIN32 y/n
									if( sz[lc].cx > sz[0].cx )
										sz[0].cx = sz[lc].cx;
									if( sz[lc].cy > sz[0].cy )
										sz[0].cy = sz[lc].cy;
								}
								else if( k )
								{
#ifdef	WIN32
									if( !GetTextExtentPoint32( hDC, // handle of device context
										lps,	// address of text string
										k,		// number of characters in string
										&sz2 ) )	// address of structure for string size
									{
										sz2.cx = 0;
										sz2.cy = 0;
									}
#else	// !WIN32
									if( !GetTextExtentPoint( hDC, // handle of device context
										lps,	// address of text string
										k,		// number of characters in string
										&sz2 ) )	// address of structure for string size
									{
										sz2.cx = 0;
										sz2.cy = 0;
									}
#endif	// WIN32 y/n
									if( sz2.cx > sz[0].cx )
										sz[0].cx = sz2.cx;
									if( sz2.cy > sz[0].cy )
										sz[0].cy = sz2.cy;
								}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								k = 0;
							}
						}
						lpt += cnt;
					}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								if( k &&
									((lc+1) < MXLINES) )
								{
									cumht += DCAttrs.yChar;
									lc++;	// Count the LAST line
#ifdef	WIN32
									if( !GetTextExtentPoint32( hDC, // handle of device context
										lps,	// address of text string
										k,		// number of characters in string
										&sz[lc] ) )	// address of structure for string size
									{
										sz[lc].cx = 0;
										sz[lc].cy = 0;
									}
#else	// !WIN32
									if( !GetTextExtentPoint( hDC, // handle of device context
										lps,	// address of text string
										k,		// number of characters in string
										&sz[lc] ) )	// address of structure for string size
									{
										sz[lc].cx = 0;
										sz[lc].cy = 0;
									}
#endif	// WIN32 y/n
									if( sz[lc].cx > sz[0].cx )
										sz[0].cx = sz[lc].cx;
									if( sz[lc].cy > sz[0].cy )
										sz[0].cy = sz[lc].cy;
								}
								else if( k )
								{
#ifdef	WIN32
									if( !GetTextExtentPoint32( hDC, // handle of device context
										lps,	// address of text string
										k,		// number of characters in string
										&sz2 ) )	// address of structure for string size
									{
										sz2.cx = 0;
										sz2.cy = 0;
									}
#else	// !WIN32
									if( !GetTextExtentPoint( hDC, // handle of device context
										lps,	// address of text string
										k,		// number of characters in string
										&sz2 ) )	// address of structure for string size
									{
										sz2.cx = 0;
										sz2.cy = 0;
									}
#endif	// WIN32 y/n
									if( sz2.cx > sz[0].cx )
										sz[0].cx = sz2.cx;
									if( sz2.cy > sz[0].cy )
										sz[0].cy = sz2.cy;
								}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					// We have accumulated all the INFO
					wsprintf( lpb, "\tUsing FIXED FONT of %u H x %u W (%ux%u)",
						cursfont.lfHeight,	// = HIBYTE(LOWORD(lpGIE->gceRes1))
						cursfont.lfWidth,	//  = LOBYTE(LOWORD(lpGIE->gceRes1))
						(HIBYTE(LOWORD(lpGIE->gceRes1)) & 0xff),
						(LOBYTE(LOWORD(lpGIE->gceRes1)) & 0xff) );
					//(LPSTR)	&cursfont.lfFaceName[0]

					wrtit;						
					wsprintf( lpb, "\tFits rectangle %u H x %u W pixels!",
						cumht, sz[0].cx );
					wrtit;
						
				}
				ResetDCAttrs( hDC, &DCAttrs );
				//==========================
				DeleteObject( hnewfont );
			}
		}
		ReleaseDC( ghMainWnd, hDC );
	}
}

#endif	// ADDDC

typedef	struct	tagHDRDIB {
	LPSTR		            hd_lpBI;    // pointer when handle LOCKED, else clear
	LPRGBQUAD	         hd_lpRGBQ;
	LPBITMAPINFOHEADER	hd_lpBIH;
	HGLOBAL		         hd_hDIB;
}HDRDIB;
typedef HDRDIB MLPTR LPHDRDIB;
	
BOOL ShowHColor( HFILE hf, LPSTR lpb, LPHDRDIB lpHD )
{
	BOOL rFlg = 0;
	int	br, bg, bb;
	int	i, j;
	BOOL	fZ;
	int		bz, bc;

	lpHD->hd_lpBIH = (LPBITMAPINFOHEADER)lpHD->hd_lpBI;
	lstrcpy( lpb, "GLOBAL COLOR TABLE" );
	wrtit;
	lpHD->hd_lpRGBQ = (LPRGBQUAD)( lpHD->hd_lpBI + *(LPDWORD)lpHD->hd_lpBI );
	switch( lpHD->hd_lpBIH->biBitCount )
	{
	case 1:
		j = 2;
		break;
	case 4:
		j = 16;
		break;
	case 8:
		j = 256;
		break;
	default:
		j = 0;
		break;
	}
	if( j )
	{
		rFlg = TRUE;
		sprt( "\tTotal %u Colors ...", j );
		fZ = FALSE;	// NO Zeros YET
		bz = 0;
		for( i = 0; i < j; i++ )
		{
			br = (lpHD->hd_lpRGBQ[i].rgbRed & 0xff);
			bg = (lpHD->hd_lpRGBQ[i].rgbGreen & 0xff);
			bb = (lpHD->hd_lpRGBQ[i].rgbBlue & 0xff);
			if( (br == 0) &&
				(bg == 0) &&
				(bb == 0) )
			{
				if( fZ )
				{
					bc++;		// Just count another ZERO
				}
				else
				{
					fZ = TRUE;	// Well we have one now
					bz = i;		// Keep BEGIN Index
					bc = 1;		// Start COUNT
				}
			}
			else	// They are NOT all ZERO
			{
				if( fZ )	// Did we have 1 or more ZEROS?
				{
					if( bc == 1 )
					{	// Bah, just ONE - Put it out like it is
						wsprintf( lpb, "\tColor %3u: R=%3u G=%3u B=%3u",
							bz, 0, 0, 0 );
					}
					else
					{	// Else SHOW a RANGE of ZEROS
						wsprintf( lpb, "\tColors %u - %u: %u Zeros ie (0,0,0)",
							bz,		// Begin index
							i - 1,	// End Index
							bc );	// Count
					}
					wrtit;
					fZ = FALSE;		// And signal DONE Zeros
				}
				// Now we can put this index
				wsprintf( lpb, "\tColor %3u: R=%3u G=%3u B=%3u",
					i,
					br,
					bg,
					bb );
				wrtit;
			}
		}
		// Check if we HAD some ZEROS, and show them ...
		if( fZ )
		{
			if( bc == 1 )
			{	// BAH, just one - Put it normally
				wsprintf( lpb, "\tColor %3u: R=%3u G=%3u B=%3u",
					bz, 0, 0, 0 );
			}
			else
			{	// Else SHOW THE RANGE ONLY - Shorten the DISP.
				wsprintf( lpb, "\tColors %u - %u: %u Zeros ie (0,0,0)",
					bz,		// Begin index
					i - 1,	// End Index
					bc );	// Count
			}
			wrtit;
			fZ = FALSE;
		}
	}
	else
	{
		sprt( "\tERROR: BitCount error", 1 );
	}

	return rFlg;

}

BOOL ShowLSD( HFILE hf, LPSTR lpb, LPGIFHDREXT lpGHE,
			 LPHDRDIB lpHD )
{
	BOOL	rFlg;
	int		i;

	rFlg = FALSE;

	lstrcpy( lpb, "LOGICAL SCREEN DESCRIPTOR (GIFHDREXT)" );
	wrtit;
//	lpGHE->gheWidth = GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	sprt( "\tWidth   = %u pixels", lpGHE->gheWidth );
//	lpGHE->gheHeight = GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);
	sprt( "\tHeight  = %u pixels", lpGHE->gheHeight );
	// Add the EXTENDED Information
//	lpGHE->gheBits = hdrbuf[4];		// packed field
	sprt( "\tBits   = %#x", lpGHE->gheBits );
//#define		ghe_ColrMap		0x80	// A Global Color Map
//#define		ghe_ColrRes		0x70	// Colour Resolution
//#define		ghe_Sort		0x08	// Sorted Colour Map
//#define		ghe_ColrSize	0x07	// Size of Colour Table ((n+1)^2)
	lstrcpy( lpb, "\t\t" );
	if( lpGHE->gheBits & ghe_ColrMap )
		lstrcat( lpb, "GlobalColorMap " );
	if( lpGHE->gheBits & ghe_Sort )
		lstrcat( lpb, "Sorted " );
	wsprintf( (lpb + lstrlen(lpb)),
		"Size=%u ", ( 2 << (lpGHE->gheBits & ghe_ColrSize) ) );
	wsprintf( (lpb + lstrlen(lpb)),
		"Res=%u ", ((lpGHE->gheBits & ghe_ColrRes) >> 4) );
	wrtit;
//	lpGHE->gheIndex = hdrbuf[5];	// Background Colour Index
	sprt( "\tBgIndex = %u", (lpGHE->gheIndex & 0xff));
//	lpGHE->ghePAR   = hdrbuf[6];	// Pixel Aspect Ration
	sprt( "\tAspect  = %u", (lpGHE->gheIndex & 0xff));
	if( ( lpHD->hd_hDIB = lpGHE->hDIB                   ) &&
		 ( lpHD->hd_lpBI = DVGlobalLock( lpHD->hd_hDIB ) ) )  // LOCK DIB HANDLE
	{
		rFlg = ShowHColor( hf, lpb, lpHD );
		i = (lpGHE->gheIndex & 0xff);
		if( lpHD->hd_lpRGBQ )
		{
			wsprintf( lpb, "\tBackground Index %3u = RGB(%3u,%3u,%3u)",
				i,
				(lpHD->hd_lpRGBQ[i].rgbRed & 0xff),
				(lpHD->hd_lpRGBQ[i].rgbGreen & 0xff),
				(lpHD->hd_lpRGBQ[i].rgbBlue & 0xff) );
		}
		else
		{
			wsprintf( lpb, "\tBackground Index %3u",
				i );
		}
		wrtit;
		DVGlobalUnlock( lpHD->hd_hDIB );  // UNLOCK DIB HANDLE - Add FIX20001201
      lpHD->hd_lpBI = 0;   // and MAKE SURE this is also ZERO after UNLOCK
	}

	return rFlg;

}

void ShowGCE( HFILE hf, LPSTR lpb, LPGIFHDREXT lpGHE,
			 LPGIFIMGEXT lpGIE, LPHDRDIB lpHD )
{
	int		i;

	if( ( lpHD->hd_hDIB == 0 ) &&
		( lpGIE->hDIB ) )
	{
		if( ( lpHD->hd_hDIB = lpGIE->hDIB                   ) &&
			 ( lpHD->hd_lpBI = DVGlobalLock( lpHD->hd_hDIB ) ) )   // LOCK DIB HANDLE
		{
			ShowHColor( hf, lpb, lpHD );
			i = (lpGHE->gheIndex & 0xff);
			if( lpHD->hd_lpRGBQ )
			{
				wsprintf( lpb, "\tBackground Index %3u = RGB(%3u,%3u,%3u)",
					i,
					(lpHD->hd_lpRGBQ[i].rgbRed & 0xff),
					(lpHD->hd_lpRGBQ[i].rgbGreen & 0xff),
					(lpHD->hd_lpRGBQ[i].rgbBlue & 0xff) );
			}
			else
			{
				wsprintf( lpb, "\tBackground Index %3u",
					i );
			}
			wrtit;
			DVGlobalUnlock( lpHD->hd_hDIB ); // UNLOCK DIB HANDLE - FIX20001201 missed unlock
         lpHD->hd_lpBI = 0;   // and make sure this is ZEROED when unlocked

		}
	}

	lstrcpy( lpb, "GRAPHIC CONTROL EXTENSION (GIFIMGEXT)" );
	wrtit;
//			lpGIE->gceBits = lpb[0];	// packed field
//			lpGIE->gceDelay = LM_to_uint(lpb[1],lpb[2]);	// 1/100 secs to wait
//			lpGIE->gceIndex = lpb[3];	// Transparency Index (if Bit set)
//#define		gce_Reserved	0xe0	// Reserved 3 Bits
	sprt( "\tgce_Bits = %#x", (lpGIE->gceBits & 0xff) );
//#define		gce_Disposal	0x1c	// Disposal Method 3 Bits
//#define		gce_UserInput	0x02	// User Input Flag 1 Bit
//#define		gce_TransColr	0x01	// Transparent Color Flag 1 Bit
	sprt( "\t\tgce_Reserved = %u",
		((lpGIE->gceBits & gce_Reserved) >> 5) );
//Values :    0 -   No disposal specified. The decoder is
//                  not required to take any action.
//            1 -   Do not dispose. The graphic is to be left
//                  in place.
//            2 -   Restore to background color. The area used by the
//                  graphic must be restored to the background color.
//            3 -   Restore to previous. The decoder is required to
//                  restore the area overwritten by the graphic with
//                  what was there prior to rendering the graphic.
//         4-7 -    To be defined.
	i = ((lpGIE->gceBits & gce_Disposal) >> 2);
	wsprintf( lpb, "\t\tgce_Disposal = %u ", i );
	switch( i )
	{
	case 0:
		lstrcat( lpb, "No disposal!" );
		break;
	case 1:
		lstrcat( lpb, "Left in place!" );
		break;
	case 2:
		lstrcat( lpb, "Restore Background!" );
		break;
	case 3:
		lstrcat( lpb, "Restore Previous!" );
		break;
	default:
		lstrcat( lpb, "UNDEFINED Disposal!!!" );
		break;
	}
	wrtit;
	wsprintf( lpb, "\t\tUser Input is %s ",
		((LPSTR)((lpGIE->gceBits & gce_UserInput) ? "On" : "Off" ) ) );
	wrtit;
	wsprintf( lpb, "\t\tTranparency is %s ",
		((LPSTR)((lpGIE->gceBits & gce_TransColr) ? "On" : "Off" ) ) );
	wrtit;
	sprt( "\tDelay = %u", (lpGIE->gceDelay & 0xff) );
	sprt( "\tIndex = %u", (lpGIE->gceIndex & 0xff) );
	if( lpGIE->gceBits & gce_TransColr )
	{
		i = (lpGIE->gceIndex & 0xff);
		if( lpHD->hd_lpRGBQ )
		{
			wsprintf( lpb, "\t\tTransparency Index %3u = RGB(%3u,%3u,%3u)",
				i,
				(lpHD->hd_lpRGBQ[i].rgbRed & 0xff),
				(lpHD->hd_lpRGBQ[i].rgbGreen & 0xff),
				(lpHD->hd_lpRGBQ[i].rgbBlue & 0xff) );
		}
		else
		{
			wsprintf( lpb, "\t\tTransparency Index %3u",
				i );
		}
		wrtit;
	}
}

void ShowGID( HFILE hf, LPSTR lpb, LPGIFIMGEXT lpGIE, LPHDRDIB lpHD )
{
	lstrcpy( lpb, "IMAGE DESCRIPTOR" );
	wrtit;
//			lpGIE->gceFlag |= gie_GID;	// Is Image Descriptor
//			lpGIE->giLeft = LM_to_uint(hdrbuf[0],hdrbuf[1]);// Left (logical) column of TEXT
//			lpGIE->giTop  = LM_to_uint(hdrbuf[2],hdrbuf[3]);// Top (logical) row
//			lpGIE->giGI.giWidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
//			lpGIE->giGI.giHeight = LM_to_uint(hdrbuf[6],hdrbuf[7]);
//			lpGIE->giBits = hdrbuf[8];	// packed field
	sprt( "\tWidth   = %3u pixels", (lpGIE->giGI.giWidth & 0xffff) );
	sprt( "\tHeight  = %3u pixels", (lpGIE->giGI.giHeight & 0xffff) );
	sprt( "\tLeft    = %3u", (lpGIE->giLeft & 0xffff) );
	sprt( "\tTop     = %3u", (lpGIE->giTop  & 0xffff) );
	sprt( "\tBits    = %#x", (lpGIE->giBits & 0xff) );
//#define		gie_ColrLoc		0x80	// Local Colour Table
//#define		gie_Interlace	0x40	// Interlaced Scan lines
//#define		gie_SortFlag	0x20	// Sorted Color Table3
//#define		gie_Reserved	0x18	// 2 reserved bits
//#define		gie_ColrSize	0x07	// Colr Table Size ((n+1)^2)
	lstrcpy( lpb, "\t\t" );
	if( lpGIE->giBits & gie_ColrLoc )
	{
		wsprintf( (lpb+lstrlen(lpb)),
			"LocalColor=%u ",
			( 2 << (lpGIE->giBits & gie_ColrSize) ) );
	}
	if( lpGIE->giBits & gie_Interlace )
		lstrcat( lpb, "Interlaced " );
	if( lpGIE->giBits & gie_SortFlag )
		lstrcat( lpb, "Sorted " );
	wsprintf( (lpb + lstrlen( lpb )),
		"ColSize=%u ",
		( 2 << (lpGIE->giBits & gie_ColrSize) ) );
	wrtit;
	if( lpGIE->hDIB )
	{
		if( lpGIE->gceFlag & gie_PTE )
		{
			lstrcpy( lpb, "\tPlain Text returned by Library..." );
			wrtit;
		}
		else
		{
			LPBITMAPINFOHEADER lpibmi;
			lstrcpy( lpb, "\tBitmap converted by Library..." );
			wrtit;
			lpibmi = 0;
			if( lpibmi = (LPBITMAPINFOHEADER) DVGlobalLock( lpGIE->hDIB ) )   // LOCK DIB HANDLE   
			{
				sprt( "\t\tWidth   = %u",
					lpibmi->biWidth );
				sprt( "\t\tHeight  = %u",
					lpibmi->biHeight );
			}
//	LPSTR		hd_lpBI;
//	LPRGBQUAD	hd_lpRGBQ;
//	LPBITMAPINFOHEADER	hd_lpBIH;
//	HGLOBAL		hd_hDIB;
			if( ( lpibmi             ) &&
				 ( lpHD->hd_hDIB == 0 ) )
			{
				lpHD->hd_hDIB = lpGIE->hDIB;
				lpHD->hd_lpBIH = lpibmi;
				lpHD->hd_lpBI = (LPSTR)lpibmi;
				lpHD->hd_lpRGBQ = (LPRGBQUAD)( lpHD->hd_lpBI + *(LPDWORD)lpHD->hd_lpBI );
				ShowHColor( hf, lpb, lpHD );
            lpHD->hd_lpBI = 0;   // clear this
			}
			else if( lpibmi )
			{
				DVGlobalUnlock( lpGIE->hDIB );   // UNLOCK DIB HANDLE
            lpHD->hd_lpBI = 0;   // clear this
			}
		}
	}
}

void ShowPTE( HFILE hf, LPSTR lpb, LPGIFHDREXT lpGHE,
			 LPGIFIMGEXT lpGIE, LPHDRDIB lpHD )
{
	int		i, j, k;
	LPPTEHDR	lpph;
	LPSTR	lpt;

	lstrcpy( lpb, "PLAIN TEXT EXTENSION" );
	wrtit;
//		else if( ( count == 12 ) &&
//			( extlabel == GIF_PTxtExt ) )
//		{
//			lpGIE->gceFlag |=	gie_PTE;	// Plain Text Extension
//     +---------------+
//  1  |               | 0 Text Grid Left Position Unsigned
//     +-             -+
//  2  |               | 1
//     +---------------+
//  3  |               | 2 Text Grid Top Position        Unsigned
//     +-             -+
//  4  |               | 3
//     +---------------+
//  5  |               | 4 Text Grid Width               Unsigned
//     +-             -+
//  6  |               | 5
//     +---------------+
//  7  |               | 6 Text Grid Height              Unsigned
//     +-             -+
//  8  |               | 7
//     +---------------+
//  9  |               | 8 Character Cell Width          Byte
//     +---------------+
// 10  |               | 9 Character Cell Height         Byte
//     +---------------+
// 11  |               | 10 Text Foreground Color Index   Byte
//     +---------------+
// 12  |               | 11 Text Background Color Index   Byte
////     +---------------+
//			lpGIE->giGI.giWidth = LM_to_uint(lpb[4],lpb[5]);
	i = (lpGIE->giGI.giWidth & 0xffff);
	sprt( "\tWidth   = %u units", i );
//			lpGIE->giGI.giHeight = LM_to_uint(lpb[6],lpb[7]);
	j = (lpGIE->giGI.giHeight & 0xffff);
	sprt( "\tHeight  = %u units", j );
	//if( ghMainWnd &&
	//	( hDC == 0 ) )
	//{
	//	hDC = GetDC( ghMainWnd );
	//	if( hDC )
	//	{
	//		giHPPI = GetDeviceCaps( hDC, LOGPIXELSX );
	//		iVPPI = GetDeviceCaps( hDC, LOGPIXELSY );
	//	}
	//}
	if( giHPPI )
	{
		wsprintf( lpb, "\t\tWidth  = %u pixels??? (giHPPI=%u)",
			(((i * giHPPI) / giTwipsPerInch) * i),
			giHPPI );
		wrtit;
	}
	if( giVPPI )
	{
		wsprintf( lpb, "\t\tHeight = %u pixels??? (giVPPI=%u)",
			(((j * giVPPI) / giTwipsPerInch) * j),
			giVPPI );
		wrtit;
	}
//			lpGIE->giLeft = LM_to_uint(lpb[0],lpb[1]);// Left (logical) column of TEXT
	sprt( "\tLeft    = %u", (lpGIE->giLeft & 0xffff) );
//			lpGIE->giTop  = LM_to_uint(lpb[2],lpb[3]);// Top (logical) row
	sprt( "\tTop     = %u", (lpGIE->giTop & 0xffff) );
//			lpGIE->gceRes1 = (DWORD)(LM_to_uint(lpb[8],lpb[9]) & 0xffff); // Char CELL
	wsprintf( lpb, "\tCell    = %u W x %u H",
		(LOBYTE(LOWORD(lpGIE->gceRes1)) & 0xff),
		(HIBYTE(LOWORD(lpGIE->gceRes1)) & 0xff) );
	wrtit;
//			lpGIE->gceRes2 = (DWORD)(lpb[10] & 0xff);	// Foreground index
	i = (int)(lpGIE->gceRes2 & 0xff);
	sprt( "\tFG Index= %u", i );
	if( lpHD->hd_lpRGBQ )
	{
		wsprintf( lpb, "\t\tForeground Index %3u = RGB(%3u,%3u,%3u)",
			i,
			(lpHD->hd_lpRGBQ[i].rgbRed & 0xff),
			(lpHD->hd_lpRGBQ[i].rgbGreen & 0xff),
			(lpHD->hd_lpRGBQ[i].rgbBlue & 0xff) );
	}
	else
	{
		wsprintf( lpb, "\t\tForeground Index %3u",
			i );
	}
	wrtit;
//			lpGIE->gceIndex = lpb[11];	// Background INDEX
	i = (int)(lpGIE->gceIndex & 0xff);
	sprt( "\tBG Index= %u", i );
	if( lpHD->hd_lpRGBQ )
	{
		wsprintf( lpb, "\t\tBackground Index %3u = RGB(%3u,%3u,%3u)",
			i,
			(lpHD->hd_lpRGBQ[i].rgbRed & 0xff),
			(lpHD->hd_lpRGBQ[i].rgbGreen & 0xff),
			(lpHD->hd_lpRGBQ[i].rgbBlue & 0xff) );
	}
	else
	{
		wsprintf( lpb, "\t\tBackground Index %3u",
			i );
	}
	wrtit;
//			lpGIE->gceSize += sizeof( PTEHDR );	// Room for 2 x DWORD header
	if( lpGIE->hDIB )
	{
		if( lpph = (LPPTEHDR) DVGlobalLock( lpGIE->hDIB ) )   // LOCK DIB HANDLE
		{
			DWORD	dwSize;
			int		cnt;
			char	c;
			int		tc, tl, ll, kk, bl, bbl;
			LPSTR	lptt;
			sprt( "\tApprox Block = %u bytes (inc.ext.block)",
				lpph->pt_Total );
			lpt = (LPSTR)((LPPTEHDR)lpph + 1);
			if( ( dwSize = lpph->pt_Total ) &&
				( lpph->pt_Missed == 0 ) &&
				( (*lpt & 0xff) == 0x0c ) )
			{
				// lfHeight = -MulDiv( PointSize,
				// LOGPIXELSY, 72 )
				lpt += (*lpt & 0xff);
				lpt++;
				k = 0;
				kk = 0;
				tc = 0;
				tl = 0;
				ll = 0;
				bl = 1;
				bbl = bl;
				lptt = lpt;
				while( cnt = (*lpt & 0xff) )
				{
					lpt++;
					for( i = 0; i < cnt; i++ )
					{
						if( k == 0 )
							lpb[k++] = '\t';
						c = lpt[i];
						lpb[k++] = c;
						if( c >= ' ' )
							kk++;
						lpb[k] = 0;
						if( c == 0x0a )
						{
							if( k >= 2 )
							{
								k--;
								lpb[k] = 0;
								k--;
								lpb[k] = '*';
							}
							tc += kk; // Total CHARACTERS
							if( kk > ll )
							{
								ll = kk;
								bbl = bl;
							}
							tl++;	// Bump LINES
							wrtit;
							bl = i + 1;	// Start of NEXT line
							k = 0;
							kk = 0;
						}
						if( k > 60 )
						{
							if( c == ' ' )
							{
								wrtit;
								k = 0;
							}
							else if( k > 70 )
							{
								wrtit;
								k = 0;
							}
						}
					}
					lpt += cnt;
				}
				if( k )
				{
					tc += kk; // Total CHARACTERS
					if( kk > ll )
					{
						ll = kk;
						bbl = bl;
					}
					tl++;	// Bump LINE count
					wrtit;
				}
				wsprintf( lpb, "\tTotal %u characters in %u lines",
					tc, tl );
				wrtit;
				sprt( "\tLongest Line = %u characters!", ll );
#ifdef	ADDDC
				if( hf && (hf != HFILE_ERROR) )
					ShowTxtInfo( lpGHE, lpGIE, lptt, bbl, hf );
#endif	// ADDDC
			}
			else
			{
				lstrcpy( lpb, "\tERROR: Block error!" );
				wrtit;
			}
			DVGlobalUnlock( lpGIE->hDIB );   // UNLOCK DIB HANDLE
		}
		else
		{
			lstrcpy( lpb, "\tERROR: Memory lock failed!" );
			wrtit;
		}
	}
	else
	{
		lstrcpy( lpb, "\tWARNING: No block returned from Library" );
		wrtit;
	}
}

BOOL	fOneTmpGif = TRUE;

void	DumpGIF( LPGIFHDREXT lpGHE, LPSTR lpf )
{
	LPGIFIMGEXT	lpGIE;
	DWORD	dw, max;
	HFILE	hf;
	OFSTRUCT	ofs;
	char		buf[260];
	LPSTR		lpb;
	HDC			hDC;
	BOOL		fDnDIB;
	HDRDIB		hd;
	LPHDRDIB	lpHD;
	LPSTR		lpo;
	DWORD		fs;

	lpHD = &hd;
	lpHD->hd_lpBI = 0;
	lpHD->hd_lpRGBQ = 0;
	lpHD->hd_lpBIH = 0;
	lpHD->hd_hDIB = 0;

	//lpbi = 0;
	hDC = 0;
	//lpq = 0;
	hf = 0;
	fs = 0;
	fDnDIB = FALSE;
	lpb = &buf[0];	// Setup buffer pointer.
	if( lpGHE )
	{
		lpo = &gszTmpGif[0]; // MAX_PATH
		if( ( max = lpGHE->gheImgCount ) &&
			 ( ( hf = PtrToInt(ghGifFile) ) == 0  ) )
		{
			if (lpo[0] == 0) {
				if (gszAppData[0]) // FIX20200602 - move outputs to %APPDATA%
					strcpy(lpo, gszAppData);
				strcat(lpo, &szDefGif[0]);
			}

			if( fOneTmpGif )
			{
				if( ( ghGifFile = DVOpenFile( lpo, &ofs, OF_READWRITE ) ) &&
					 ( ghGifFile != INVALID_HANDLE_VALUE ) )
				{
					// move to END OF FILE
					fs = DVSeekEnd( ghGifFile );
					wsprintf( lpb,
						"=== At %u Bytes === plus ===>"MEOR,
						fs );
					wrtit;
				}
				else
				{
					ghGifFile = DVOpenFile( lpo, &ofs, OF_CREATE | OF_WRITE );
				}
			}
			else
			{
				if( !FindNewName( lpo ) )	// and get a unique name
				{
					//DIBError( ERR_INTERROR );
					return;
				}
				ghGifFile = DVOpenFile( lpo, &ofs, OF_CREATE | OF_WRITE );
			}
		}

		if( ( max = lpGHE->gheImgCount ) &&
			 ( hf = PtrToInt(ghGifFile)  ) &&
			 ( hf != HFILE_ERROR        ) )
		{

			wsprintf( lpb, "Processing file [%s]\r\nOn %s -",
				lpf,
				GetDT4( 0 ) );
			wrtit;

			for( dw = 0; dw <= max; dw++ )
			{
				if( dw == 0 )
				{
					fDnDIB = ShowLSD( hf, lpb, lpGHE, lpHD );
				}
				else
				{
					lpGIE = &lpGHE->gheGIE[dw-1];
//			// NOTE: Offsets a MINUS 1, since we have read the count
//			lpGIE->gceFlag |=	gie_GCE;	// Had Graphic Control Extension
					if( lpGIE->gceFlag & gie_GCE )	// Had Graphic Control Extension
					{
						ShowGCE( hf, lpb, lpGHE, lpGIE, lpHD );
					}

//		else if( ( count == 11 ) &&
//			( extlabel == GIF_AppExt ) )
//		{
//#define		gie_APP			0x00000010	// Application Extension
//			if( IsNetscape( lpb, 0 ) )
//			{
//				lpGIE->gceRes1 = (DWORD)LM_to_uint(lpb[13],lpb[14]);	// Loop count
//				lpGIE->gceFlag |= gie_Netscape;	// FLAG Netscape
//			}
//		}
					if( lpGIE->gceFlag & gie_APP )
					{
						lstrcpy( lpb, "APPLICATION EXTENSION" );
						wrtit;
//				lpGIE->gceFlag |= gie_Netscape;	// FLAG Netscape
						if( lpGIE->gceFlag & gie_Netscape )
						{
							lstrcpy( lpb, "\tNetscape Extension" );
							wrtit;
//				lpGIE->gceRes1 = (DWORD)LM_to_uint(lpb[13],lpb[14]);	// Loop count
							sprt( "\t\tLoop Count = %u", (lpGIE->gceRes1 & 0xffff) );
						}
						else
						{
							lstrcpy( lpb, "\tUndefined Application!" );
							wrtit;
						}
					}

					if( lpGIE->gceFlag & gie_GID )
					{
						ShowGID( hf, lpb, lpGIE, lpHD );
					}

					if( lpGIE->gceFlag & gie_PTE )
					{
						ShowPTE( hf, lpb, lpGHE, lpGIE, lpHD );
					}

//		}
//		else if( GIF_CommExt )
//		{
//#define		gie_COM			0x00000020	// Comment Extension
//			if( count > 8 )
//				count = 8;
//			lps = (LPSTR) &lpGIE->gceRes1;
//			for( i = 0; i < count; i++ )
//			{
//				lps[i] = lpb[i];
//			}
//		}
//		else
//		{
//#define		gie_UNK			0x80000000	// Undefined Extension
//			if( count > 8 )
//				count = 8;
//			lps = (LPSTR) &lpGIE->gceRes1;
//			for( i = 0; i < count; i++ )
//			{
//				lps[i] = lpb[i];
//			}
//		}
				}
			}
		}
	}

	if( lpHD->hd_hDIB && lpHD->hd_lpBI )
		DVGlobalUnlock( lpHD->hd_hDIB );

	if( hf && (hf != HFILE_ERROR) )
		DVlclose( IntToPtr(hf) );
	//if( hDC )
	//	ReleaseDC( ghMainWnd, hDC );

	ghGifFile = 0;
}

// =============================================================
#endif	// GIFDIAGS
// OFF for release of course.
// =========================

#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF


// eof - DvGetGif.c

