

// GMDrives.c
/* ==============================================
 *
 *      File:  DVDrives.C
 *
 *   History:   Date     Reason
 *             Sep99    Created
 *
 * ============================================= */
#include	"dv.h"
#include    <direct.h>
#define		ADDGRMLIB

extern	char	UpIt( char c );
#ifdef	ADDGRMLIB

extern	void    Dbl2Stg( LPSTR lps, double factor, int prec );

#endif	/* ADDGRMLIB */

#define     MXVOLB      256
#define     MXNAMB      128

//#define       MXTXT1          128
//#define       MXTXT2          32
#define     MXTXT1          256
#define     MXTXT2          64
#define     uc          unsigned char
#define     puc         uc *
#define     MXDRVS      26
//#define       MXDBUF      ((MXDRVS+6) * 4)
#define     MXDBUF      288
#define     MXDRVB		((MXDRVS + 6 ) * 160)

typedef struct tagDRLIST {  /* dl */
    BOOL    dl_ok;
    HGLOBAL dl_gh;
    int     dl_cnt;
    DWORD   dl_sz;
    int     dl_sel;
    DWORD   dl_soff;
    int     dl_drv;
    DWORD   dl_drives;
    DWORD   dl_count;
    char    dl_szList[MXDBUF];
}DRLIST;

typedef DRLIST * PDRLIST;

typedef struct tagDRTYPE {  /* dr */
    UINT    dr_type;
    puc     dr_stg;
}DRTYPE;
typedef DRTYPE * PDRTYPE;

// Values in dv_Flag
#define		dvf_ErrDrvLst		0x00000001
#define		dvf_NoDisk			0x00000002
#define		dvf_NoVolInfo		0x00000004
#define		dvf_NoFreeSp		0x00000008

typedef struct tagDRV1 {		/* dv */

	DWORD	dv_Flag;
	UINT	dv_Letter;
	UINT	dv_Type;
	DWORD	dv_Serial;
	DWORD	dv_SysFlg;
	DWORD	dv_SecPerClus;
	DWORD	dv_BytPerSec;
	DWORD	dv_NumFreeClus;
	DWORD	dv_TotClust;
//	DWORDLONG	dv_dwlTotSize;
//	DWORDLONG	dv_dwlTotFree;

	double	dv_dTotSize;
	double	dv_dTotFree;

    BOOL            dv_bGotDF;

	/* 64-bit DRIVE SIZE */
//    ULARGE_INTEGER  dv_ulFreeToCall;
//    ULARGE_INTEGER  dv_ulTotalBytes;
//    ULARGE_INTEGER  dv_ulFreeBytes;
    DWORDLONG  dv_ulFreeToCall;
    DWORDLONG  dv_ulTotalBytes;
    DWORDLONG  dv_ulFreeBytes;
	/* ================= */

	uc	dv_Label[MXVOLB+8];
	uc	dv_FSName[MXNAMB+8];

}DRV1;

typedef DRV1 FAR * LPDRV1;

typedef struct tagCPieGraph {

    DWORD       m_StrucSize;

    /* Member variables  */
    POINT       m_BSize;
    DWORD       m_dwNum;

    COLORREF    m_crBackground; /* Background color */
    COLORREF    m_crLines;      /* Line color       */

    COLORREF    m_crUsed;       /* Used slice color */
    COLORREF    m_crFree;       /* Free slice color */

    HBITMAP     m_hBitmap;      /* painted graphic */
    float       m_fFree;        /* Percentage FREE */

    COLORREF    m_crCurBack;    /* current background of BITMAP */
    RECT        m_rCurBack;

}PIEGRAPH;

typedef PIEGRAPH *  LPPIEGRAPH;

typedef	struct {

	RECT	sd_rWin;
	int		sd_iWidf;	/* if there is space FREE */
	RECT	sd_rMax;
	RECT	sd_rAll;
	RECT	sd_rFree;
	POINT	sd_pTxt1;
	POINT	sd_pTxt2;
	POINT	sd_pTxtE1;
	POINT	sd_pTxtE2;
	int		sd_iTxt1;
	int		sd_iTxt2;
	RECT	sd_rTxt1;
	RECT	sd_rTxt2;
	uc		sd_cTxt1[MXTXT1];
	uc		sd_cTxt2[MXTXT2];

	PIEGRAPH sd_Pie;

	DRV1	sd_Drv1;

}SDRV1;

typedef SDRV1 FAR * LPSDRV1;

typedef struct {

	DWORD	hd_Count;
	double	hd_dMin;
	UINT	hd_uMax;
	double	hd_dMax;
	UINT	hd_uMin;

	SDRV1	hd_Data;    /* extra 1 for summary (at end) */
    /* others drives can be attached here          */
}HDRVS;

typedef HDRVS FAR * LPHDRVS;


DWORD	gdwDrvLst;
DWORD	gdwDrvCnt = 0;

/* this will contain the active list of drives */
/* =========================================== */
DRV1    sDrives[MXDRVS + 2];
/* =========================================== */
/* the LAST should be the summary of ALL 26 drives */
// Value    Meaning
DRTYPE DrType[] = {
//  { 0,            "Unknown" },    // The drive type cannot be determined.
//  { 1,            "NotExist" },   // The root directory does not exist.
    { DRIVE_REMOVABLE, "Floppy" },  // The drive can be removed from the drive.
    { DRIVE_FIXED,     "Fixed" },   // The disk cannot be removed from the driv
    { DRIVE_REMOTE,    "Remote" },  // The drive is a remote (network) drive.
    { DRIVE_CDROM,     "CD-ROM" },  // The drive is a CD-ROM drive.
    { DRIVE_RAMDISK,   "RAM" },     // The drive is a RAM disk.
    { (DWORD)-1,    "Unknown" }
};

typedef BOOL (*GDFSEX) ( LPCTSTR, PULARGE_INTEGER,
                        PULARGE_INTEGER, PULARGE_INTEGER );

//typedef struct _OSVERSIONINFO{     DWORD dwOSVersionInfoSize;
//    DWORD dwMajorVersion;     DWORD dwMinorVersion;     DWORD dwBuildNumber;
//    DWORD dwPlatformId;     TCHAR szCSDVersion[ 128 ]; } OSVERSIONINFO
int             giDnVers = 0;
OSVERSIONINFO   gsOSVers;
GDFSEX          gfGDFSEx = 0;

#ifdef  UNICODE
#define DEF_DISKEX      "GetDiskFreeSpaceExW"
#else
#define DEF_DISKEX      "GetDiskFreeSpaceExA"
#endif

TCHAR   gszDiskEx[] = DEF_DISKEX;

uc      szTerm[] = "|*|";
DWORD	dwDrvCnt = 0;
DRLIST  gsDrList = { 0 };   /* = drl */

double  DW32Dbl( DWORD dwspc, DWORD dwbps, DWORD dwcnt )
{
    double  d1, d2, d3, dres;
    d1 = (double)dwspc;
    d2 = (double)dwbps;
    d3 = (double)dwcnt;

    dres = d1 * d2 * d3;

    return dres;
}

// #ifndef		ADDGRMLIB
// ===========================================================
// void Buffer2Stg( LPSTR lps, LPSTR lpb, int decimal,
//				 int sign, int precision )
//
// Purpose: Convert the string of digits from the _ecvt
//			function to a nice human readbale form.
//
// 1999 Sept 7 - Case of removing ?.?0000 the zeros
//
// ===========================================================
void Buffer2Stg( LPSTR lps, LPSTR lpb, int decimal,
				 int sign, int precision )
{
	int		i, j, k, l, m, sig, cad;
	char	c;

	k = 0;					// Start at output beginning
	cad = 0;				// Count AFTER the decimal
	j = lstrlen( lpb );		// Get LENGTH of buffer digits

	if( sign )				// if the SIGN flag is ON
		lps[k++] = '-';		// Fill in the negative

	l = decimal;
	if( l < 0 )
	{
		// A NEGATIVE decimal position
		lps[k++] = '0';
		lps[k++] = '.';
		cad++;
		while( l < 0 )
		{
			lps[k++] = '0';
			l++;
			cad++;
		}
	}
	else if( ( decimal >= 0 ) &&
		( decimal < precision ) )
	{
		// Possible shortened use of the digit string
		// ie possible LOSS of DIGITS to fit the precision requested.
		if( decimal == 0 )
		{
			if( ( precision - 1 ) < j )
			{
				//chkme( "NOTE: Precision less than j" );
				j = precision - 1;
			}
		}
		else
		{
			if( precision < j )
			{
//				chkme( "NOTE: Precision less than j" );
				j = precision;
			}
		}
	}

	sig = 0;	// Significant character counter
	// Process each digit of the digit list in the buffer
	// or LESS than the list if precision is LESS!!!
	for( i = 0; i < j; i++ )
	{
		c = lpb[i];		// Get a DIGIT
		if( i == decimal )	// Have we reached the DECIMAL POINT?
		{
			// At the DECIMAL point
			if( i == 0 )	
			{
				// if no other digits BEFORE the decimal
				lps[k++] = '0';	// then plonk in a zero now
			}
			lps[k++] = '.';	// and ADD the decimal point
			cad++;
		}
		// Check for adding a comma for the THOUSANDS
		if( ( decimal > 0 ) &&
			( sig ) &&
			( i < decimal ) )
		{
			m = decimal - i;
			if( (m % 3) == 0 )
				lps[k++] = ',';	// Add in a comma
		}
		lps[k++] = c;	// Add this digit to the output
		if( sig )		// If we have HAD a significant char
		{
			sig++;		// Then just count another, and another etc
		}
		else if( c > '0' )
		{
			sig++;	// First SIGNIFICANT character
		}
		if( cad )
			cad++;
	}	// while processing the digit list

	// FIX980509 - If digit length is LESS than decimal position
	// =========================================================
	if( ( decimal > 0 ) &&
		( i < decimal ) )
	{
		c = '0';
		while( i < decimal )
		{
			if( ( decimal > 0 ) &&
				( sig ) &&
				( i < decimal ) )
			{
				m = decimal - i;
				if( (m % 3) == 0 )
					lps[k++] = ',';	// Add in a comma
			}
			lps[k++] = c;	// Add this digit to the output
			i++;
		}
	}
	// =========================================================
	if( cad )
		cad--;
	lps[k] = 0;		// zero terminate the output
	// FIX990907 - Remove unsightly ZEROs after decimal point
    for( i = 0; i < k; i++ )
    {
        if( lps[i] == '.' )
            break;
    }
    if( ( i < k ) &&
        ( lps[i] == '.' ) )
    {
        i++;
        if( lps[i] == '0' )
        {
            while( k > i )
            {
                k--;
                if( lps[k] == '0' )
                    lps[k] = 0;
                else
                    break;
            }
            if( k > i )
            {
                // we have backed to a not '0' value so STOP
            }
            else
            {
                // we backed all the way, so remove the DECIMAL also
                i--;
                lps[i] = 0;
            }
        }
        else
        {
            while( k > i )
            {
                k--;
                if( lps[k] == '0' )
                    lps[k] = 0;
                else
                    break;
            }
        }
    }

}


void    Dbl2Stg( LPSTR lps, double factor, int prec )
{
    int             decimal, sign, precision;
    char *  buffer;

    if( prec )
        precision = prec;
    else
        precision = 16;

    buffer = _ecvt( factor, precision, &decimal, &sign );

    Buffer2Stg( lps, buffer, decimal, sign, precision );
}

// #endif	/* ADDGRMLIB */

void    GetDSFunction( void )
{
    HINSTANCE   hInst;
    LPTSTR      lps;

    if( !giDnVers )
    {
        memset( &gsOSVers, 0, sizeof(OSVERSIONINFO) );
        gsOSVers.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if( GetVersionEx( &gsOSVers ) ) // function to determine that a system is running OSR 2 or a later release of the Windows 95 operating system.
        {
            // The GetVersionEx function fills in the members
            // of an OSVERSIONINFO data structure.
            if( ( gsOSVers.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) &&
                ( LOWORD( gsOSVers.dwBuildNumber ) > 1000 ) )
            {
                // the system is running OSR 2 or a later release. 
                // Once you have determined that a system is running OSR 2,
                // call the LoadLibrary or LoadLibraryEx function
                // to load the KERNEL32.DLL file, then call the
                // GetProcAddress function to obtain an address 
                // for the GetDiskFreeSpaceEx function.
                if( hInst = LoadLibraryEx( "KERNEL32.DLL",  // points to name of executable module 
                    NULL,                   // reserved, must be NULL
                    0 ) )   // DWORD dwFlags = entry-point execution flag 
                {
                    lps = &gszDiskEx[0];
                    gfGDFSEx = (GDFSEX)GetProcAddress( hInst,
                        lps );   // "GetDiskFreeSpaceExA"
                    FreeLibrary( hInst );
                }
            }
        }
        giDnVers = 1;

    }
}



void    GetFreeEx( LPTSTR lpd, 	LPDRV1	lpdrv )
{
    TCHAR   buf[264];
    GDFSEX  gx;

    if( ( gx = gfGDFSEx ) &&
        ( lpd ) &&
        ( lpdrv ) )
    {
        lstrcpy( buf, lpd );
        lpdrv->dv_bGotDF = (*gx) ( buf,
            (PULARGE_INTEGER) &lpdrv->dv_ulFreeToCall,
            (PULARGE_INTEGER) &lpdrv->dv_ulTotalBytes,
            (PULARGE_INTEGER) &lpdrv->dv_ulFreeBytes );
    }
}


DWORD	DoAll26( LPSTR lpStg, DWORD dwLen,
				DWORD dwLst,
				LPSTR lpTxt )
{
	int	drive;
	LPSTR	lpd = lpStg;
	DWORD	dln, dlen;
	DWORD	dstg = dwLen;
	UINT	ui;
	PDRTYPE	pdr;
	LPSTR	lpt;
	char	c, d;
	DWORD	drnum, dmask;
	DWORD	dlist;
	int		ch = 0;
	LPDRV1	lpdrv;
	DWORD	boff;
	LPSTR	lptxt = lpTxt;
	LPSTR	lpfm;

	lpdrv = &sDrives[0];
	dln   = dlen = 0;
	dlist = dwLst;
    /* ========================================== */
    /* process 26 logical drive letters           */
    /* ========================================== */
	for( drive = 0; drive < 26; drive++ )
	{

		for(dln = 0; dlen < dstg; dlen++, dln++ )
		{
			if( lpd[dln] == 0 )
				break;
		}

		ui = GetDriveType( lpd );	// address of root path
		pdr = &DrType[0];
		lpt = 0;
		while( pdr->dr_type != (UINT)-1 )
		{
			if( pdr->dr_type == ui )
			{
				break;
			}
			pdr++;
		}
		lpt = pdr->dr_stg;
		if( lpt )
		{
			DWORD	sernum, maxlen;

			/* we have a TYPE string */
			/* get 1st char and convert to drive number */
			c = *lpd;
			d = UpIt( c );
			drnum = (int)( d - ('A' - 1) );
			/* establish a MASK for this drive */
			dmask = 1 << (drnum - 1);
			/* ensure it is IN THE BIT LIST */
			if( dlist & dmask )
			{
				LPSTR	lpvn, lpfsn;
				LPDWORD	lpspc, lpbps, lpnfc, lptc, lpsf;

				/* remove this bit - as done */
				dlist = (dlist & ~dmask);
				ch++;	// Count a VALID Drive
				memset( lpdrv, 0, sizeof(DRV1) );
				lpdrv->dv_Letter = drnum;
				lpdrv->dv_Type   = ui;
				lpvn = &lpdrv->dv_Label[0];
				*lpvn = 0;	// None presently
				lpfsn = &lpdrv->dv_FSName[0];
				*lpfsn = 0;
//	LPDWORD	lpspc, lpbps, lpnfc, lptc;
				lpspc = &lpdrv->dv_SecPerClus;
				lpbps = &lpdrv->dv_BytPerSec;
				lpnfc = &lpdrv->dv_NumFreeClus;
				lptc  = &lpdrv->dv_TotClust;
				lpsf  = &lpdrv->dv_SysFlg;
				if( lptxt )
				{
					LPSTR lpb;
					lpb = lptxt;
					lstrcpy( &lpb[boff], lpd );
					boff = lstrlen( lpb );
					wsprintf( &lpb[boff], " [%s] ", lpt );
					boff = lstrlen( lpb );
				}
				if( _chdrive( drnum ) == 0 )
				{
//					vbuf[0] = 0;
					sernum  = 0;
//					nbuf[0] = 0;
                    lpdrv->dv_bGotDF = 0;
                    if( gfGDFSEx )
                    {
//                        lpdrv->dv_bGotDF =
//                            (*gfGDFSEx) ( lpd,
//                                &lpdrv->dv_ulFreeToCall,
//                                &lpdrv->dv_ulTotalBytes,
//                                &lpdrv->dv_ulFreeBytes );
                        GetFreeEx( lpd, lpdrv );
                    }

					if( GetVolumeInformation( lpd, // address of root directory of the file system 
						lpvn,	//&vbuf[0],	// address of name of the volume
						MXVOLB,		// length of lpVolumeNameBuffer
						&sernum,	// address of volume serial number 
						&maxlen,	// address of system's maximum filename length
						lpsf, //&sysflg,	// address of file system flags 
						lpfsn,	//&nbuf[0],	// address of name of file system 
						MXNAMB ) )	// length of lpFileSystemNameBuffer 
					{
//						if( vbuf[0] )
						lpdrv->dv_Serial = sernum;
						if( lptxt )
						{
//							LPSTR	lpb;
//							LPSTR	lpfm;
//							lpb = lptxt;

							if( *lpvn )
								lpfm = " [%s] ";	//(LPSTR)&vbuf[0] );
							else
								lpfm = " [*No label*] ";
							wsprintf( EndBuf(lptxt),
								lpfm,
								lpvn );

//							if( *lpvn )
//								wsprintf( &lpb[boff], " [%s] ", lpvn );	//(LPSTR)&vbuf[0] );
//							else
//								lstrcat( lpb, " [*No label*] " );
//							boff = lstrlen( lpb );
							if( sernum )
								lpfm = " [%lu]";
							else
								lpfm = " [*No Serial*] ";
							wsprintf( EndBuf(lptxt),
								lpfm,
								sernum );

//							if( sernum )
//								wsprintf( &lpb[boff], " [%lu]", sernum );
//							else
//								lstrcat( lpb, " [*No Serial*] " );
//							boff = lstrlen( lpb );
//							if( nbuf[0] )
							if( *lpfsn )
								lpfm = " [%s] ";	//(LPSTR)&nbuf[0] );
							else
								lpfm = " [*?*] ";
							wsprintf( EndBuf(lptxt),
								lpfm,
								lpfsn );	//(LPSTR)&nbuf[0] );

//							if( *lpfsn )
//								wsprintf( &lpb[boff], " [%s] ", lpfsn );	//(LPSTR)&nbuf[0] );
//							else
//								lstrcat( lpb, " [*?*] " );
//							boff = lstrlen( lpb );
							boff = lstrlen( lptxt );
						}

//						if( GetDiskFreeSpace( lpd, // address of root path
//							&SecPerClus,	// address of sectors per cluster
//							&BytPerSec,	// address of bytes per sector
//							&NumFreeClus,	// address of number of free clusters
//							&TotClust ) ) 	// address of total number of clusters
						if( GetDiskFreeSpace( lpd, // address of root path
							lpspc, //&SecPerClus,	// address of sectors per cluster
							lpbps, //&BytPerSec,	// address of bytes per sector
							lpnfc, //&NumFreeClus,	// address of number of free clusters
							lptc ) ) //&TotClust ) ) 	// address of total number of clusters
						{
//							boff = lstrlen( lpb );
//							wsprintf( &lpb[boff], " [%lu] [%lu] ",
//								(BytPerSec * SecPerClus * TotClust),
//								(BytPerSec * SecPerClus * NumFreeClus) );
							//lpdrv->dv_dwlTotSize =
							//	( *lpbps * *lpspc * *lptc );

							//lpdrv->dv_dwlTotFree =
							//	( *lpbps * *lpspc * *lpnfc );
                            if( ( gfGDFSEx ) &&
                                ( lpdrv->dv_bGotDF ) )
                            {
                                // lpdrv->dv_ulFreeToCall,
                                lpdrv->dv_dTotSize = (double)(__int64)lpdrv->dv_ulTotalBytes;
                                lpdrv->dv_dTotFree = (double)(__int64)lpdrv->dv_ulFreeBytes;
                            }
                            else
                            {
            				    lpdrv->dv_dTotSize =
			            		    DW32Dbl( lpdrv->dv_SecPerClus,
						                lpdrv->dv_BytPerSec,
						                lpdrv->dv_TotClust );
				                lpdrv->dv_dTotFree =
					                DW32Dbl( lpdrv->dv_SecPerClus,
						                lpdrv->dv_BytPerSec,
						                lpdrv->dv_NumFreeClus );
                            }

//							wsprintf( &lpb[boff], " [%lu] [%lu] ",
//								lpdrv->dv_dwlTotSize,
//								lpdrv->dv_dwlTotFree );
							if( lptxt )
							{
								LPSTR	lpb;
								lpb = lptxt;
								lstrcat( lpb, " [" );
		                        Dbl2Stg( EndBuf(lpb), lpdrv->dv_dTotSize, 16 );
			                    lstrcat( lpb, "] [" );
				                Dbl2Stg( EndBuf(lpb), lpdrv->dv_dTotFree, 16 );
					            lstrcat( lpb, "] " );
//								( *lpbps * *lpspc * *lptc),
//								( *lpbps * *lpspc * *lpnfc ) );
							}
						}
						else
						{
							lpdrv->dv_Flag |= dvf_NoFreeSp;
							if( lptxt )
							{
								lstrcat( lptxt, " [?] [?] " );
								boff = lstrlen( lptxt );
							}
						}
 					}
					else
					{
						lpdrv->dv_Flag |= dvf_NoVolInfo;
						if( lptxt )
						{
							lstrcat( lptxt, " [*No Label*] [*No Serial*] [*No ?*] [?] [?] " );
							boff = lstrlen( lptxt );
						}
					}
				}
				else	// NO Disk in DRIVE ...
				{
					lpdrv->dv_Flag |= dvf_NoDisk;
					if( lptxt )
					{
						lstrcat( lptxt, " [*EMPTY*] [*] [*] [?] [?] ");
						boff = lstrlen( lptxt );
					}
				}
				if( lptxt )
				{
					lstrcat( lptxt, szTerm );
					boff = lstrlen( lptxt );
				}

				/* we can only store up to 26 here */
				if( dwDrvCnt < MXDRVS )
				{
					lpdrv++;	/* to next sDrives[] struct */
					dwDrvCnt++;	/* inc our count */
					/* and clear next to zero */
					memset( lpdrv, 0, sizeof(DRV1) );
				}
			}
		}

		dln++;		// Bump past the ZERO
		lpd = lpd + dln;
		if( lpd[0] == 0 )
			break;
		if( (boff + MXNAMB + MXVOLB) > MXDRVB )
			break;
		if( dlen >= dstg )
			break;
//		DBWaitINC;

	}	// for 26 drives

	return dlist;

}

DWORD   GetDriveCount( DWORD dlist )
{
    int     i = 32;
    DWORD   mask  = 1;
    DWORD   dwCnt = 0;

    while( i-- )
    {
        if( dlist & mask )
        {
            dwCnt++;
        }
        mask = mask << 1;
    }
    if( dwCnt )
    {
        gdwDrvLst = dlist;
        gdwDrvCnt = dwCnt;
    }

    return dwCnt;
}

/* =======================================================================
    PDRLIST GetDriveList( HWND hwnd, LPVOID lpv, PDRLIST lpPrev,
						BOOL bRetTxt )
    was PDRLIST ChooseDrive( HWND hwnd, LPVOID lpv )

    INPUT:
        HWND    hwnd;   owner window
        LPVOID  lpv;    equals =
            -> typedef struct {
	            DWORD	hd_Count;
	            double	hd_dMin;
				UINT	hd_uMin;
				double	hd_dMax;
				UINT	hd_uMax;	// //	UINT	dv_Letter;
	            SDRV1	hd_Data;    -- declare the first drive --
                -- others drives can be attached here          --
                }HDRVS;
                typedef HDRVS FAR * LPHDRVS;

    ======================================================================  */
//PDRLIST ChooseDrive( HWND hwnd, LPVOID lpv )

PDRLIST GetDriveList( HWND hwnd, LPVOID lpv, PDRLIST lpPrev,
					 BOOL bRetTxt )
{
	PDRLIST	pdrl = 0;	/* allocated memory list */
	BOOL flg;
	int		curdrive;
	DWORD	dlist;
	DWORD	dstg;
	LPSTR	lpd;
	LPSTR	lpTxt;
	LPDRV1	lpdrv;
	UINT	OldMode;
	LPHDRVS lphdin;
	DWORD	dcount;
	PDRLIST	lpdr;
	HGLOBAL	hg;

	lpdr = &gsDrList;
	lphdin = (LPHDRVS)lpv;

	flg = FALSE;
	pdrl = 0;	/* No DRIVE LIST = no allocated memory */

	gsDrList.dl_ok = flg;
//	hgDrv = 0;
	hg = 0;
	lpTxt = 0;
	if( bRetTxt )
	{
		/* allocate a text buffer for the text output */
//		if( (hgDrv = DVGlobalAlloc( GHND, MXDRVB )) == 0 )
//		if( (hg = DVGlobalAlloc( GHND, MXDRVB )) == 0 )
		if( (hg = DVGAlloc( "DRVLIST", GHND, MXDRVB )) == 0 )
			return( pdrl );


//		if( (lpTxt = DVGlobalLock( hgDrv )) == 0 )
		if( (lpTxt = DVGlobalLock( hg )) == 0 )
		{
			DVGlobalFree( hg );
//			DVGlobalFree( hgDrv );
//			hgDrv = 0;
			return( pdrl );
		}
	}

	dlist  = GetLogicalDrives();
	dcount = GetDriveCount( dlist );

	if( dcount == 0 )
	{
		if( hg )
			DVGlobalFree( hg );
		hg = 0;
		return( pdrl );
	}

	// SET WAIT
	// ========
	//DBWaitON;
	// ========

    GetDSFunction();

//	ch = 0;
//	lpd = &dbuf[ch];
//	lpd = &dbuf[0];
	lpd = &lpdr->dl_szList[0];
//	dlen = 0;
	dstg = GetLogicalDriveStrings( MXDBUF, // size of buffer
							 lpd ); // address of buffer for drive strings 

	lpdrv = &sDrives[0];
	dwDrvCnt = 0;
    memset( lpdrv, 0, sizeof(DRV1) );

	OldMode = SetErrorMode( SEM_FAILCRITICALERRORS );
	/* ========================================== */
	/* return to here when done                   */
	curdrive = _getdrive();
    /* ========================================== */
    /* process 26 logical drive letters           */
    /* ========================================== */
	dlist = DoAll26( lpd, dstg, dlist, lpTxt );
	/* ========================================== */
	/* Switch BACK to original DRIVE ============ */
	_chdrive( curdrive );
	/* ========================================== */
	OldMode = SetErrorMode( OldMode );

//	lpd = &dbuf[ch];
//	lpd = EndBuf( &dbuf[0] );
	if( ( hg    ) &&
		( lpTxt ) )
	{
//		DVGlobalUnlock( hgDrv );
		DVGlobalUnlock( hg );
		lpTxt = 0;
	}

	if( dlist == 0 )
	{
//		PDRLIST	lpdr;
//		lpdr = &gsDrList;
		/* set as an ok list of drives in system */
		lpdr->dl_ok = TRUE;
		lpdr->dl_gh = hg;	/* handle to memory */
		lpdr->dl_cnt = dcount;
//		lpdr->dl_cnt = ch;
//		lpdr->dl_sz = boff;
		lpd = &lpdr->dl_szList[0];
		while( curdrive = lstrlen(lpd) )
		{
			lpd += curdrive;
			*lpd = ' ';
			lpd++;
		}
		lpd = &lpdr->dl_szList[0];
		lpdr->dl_sz = lstrlen(lpd);
//		lpdr->dl_sel = ch + 1;	// Put this OUT OF RANGE
		lpdr->dl_sel = dcount + 1;	// Put this OUT OF RANGE
		lpdr->dl_soff = MXDRVB;	// and this too ...
		lpdr->dl_drv = MXDRVS + 1;
		/* ================================================== */
		lpdr->dl_drives = gdwDrvLst;	/* bit LIST of DRIVES */
		lpdr->dl_count  = gdwDrvCnt;	/* count of bits      */
		pdrl = lpdr;
	}

	//DBWaitOFF;

	/* actually just return a global pointer to an instance */
	return( pdrl );

}

PDRLIST SetStaticDL( void )
{
	PDRLIST	pdrl = 0;	/* static list */
	BOOL flg;
	int		curdrive;
	DWORD	dlist;
	DWORD	dstg;
	LPSTR	lpd;
//	LPSTR	lpTxt;
	LPDRV1	lpdrv;
	UINT	OldMode;
//	LPHDRVS lphdin;
	DWORD	dcount;
	PDRLIST	lpdr;
//	HGLOBAL	hg;

	lpdr = &gsDrList;
//	lphdin = (LPHDRVS)lpv;

	flg = FALSE;
	pdrl = 0;	/* No DRIVE LIST */

	gsDrList.dl_ok = flg;
//	hgDrv = 0;
//	hg = 0;
//	lpTxt = 0;
//	if( bRetTxt )
//	{
		/* allocate a text buffer for the text output */
//		if( (hgDrv = DVGlobalAlloc( GHND, MXDRVB )) == 0 )
//		if( (hg = DVGlobalAlloc( GHND, MXDRVB )) == 0 )
//			return( pdrl );


//		if( (lpTxt = DVGlobalLock( hgDrv )) == 0 )
//		if( (lpTxt = DVGlobalLock( hg )) == 0 )
//		{
//			DVGlobalFree( hg );
//			DVGlobalFree( hgDrv );
//			hgDrv = 0;
//			return( pdrl );
//		}
//	}

	dlist  = GetLogicalDrives();
	dcount = GetDriveCount( dlist );

	if( dcount == 0 )
	{
//		if( hg )
//			DVGlobalFree( hg );
//		hg = 0;
		return( pdrl );
	}

	// SET WAIT
	// ========
	//DBWaitON;
	// ========

    GetDSFunction();

//	ch = 0;
//	lpd = &dbuf[ch];
//	lpd = &dbuf[0];
	lpd = &lpdr->dl_szList[0];
//	dlen = 0;
	dstg = GetLogicalDriveStrings( MXDBUF, // size of buffer
							 lpd ); // address of buffer for drive strings 

	lpdrv = &sDrives[0];
	dwDrvCnt = 0;
    memset( lpdrv, 0, sizeof(DRV1) );

	OldMode = SetErrorMode( SEM_FAILCRITICALERRORS );
	/* ========================================== */
	/* return to here when done                   */
	curdrive = _getdrive();
    /* ========================================== */
    /* process 26 logical drive letters           */
    /* ========================================== */
	dlist = DoAll26( lpd, dstg, dlist, 0 );
	/* ========================================== */
	/* Switch BACK to original DRIVE ============ */
	_chdrive( curdrive );
	/* ========================================== */
	OldMode = SetErrorMode( OldMode );

//	lpd = &dbuf[ch];
//	lpd = EndBuf( &dbuf[0] );
//	if( ( hg    ) &&
//		( lpTxt ) )
//	{
//		DVGlobalUnlock( hgDrv );
//		DVGlobalUnlock( hg );
//		lpTxt = 0;
//	}

	if( dlist == 0 )
	{
//		PDRLIST	lpdr;
//		lpdr = &gsDrList;
		/* set as an ok list of drives in system */
		lpdr->dl_ok = TRUE;
		lpdr->dl_gh = 0;	// hg;	handle to memory
		lpdr->dl_cnt = dcount;
//		lpdr->dl_cnt = ch;
//		lpdr->dl_sz = boff;
		lpd = &lpdr->dl_szList[0];
		while( curdrive = lstrlen(lpd) )
		{
			lpd += curdrive;
			*lpd = ' ';
			lpd++;
		}
		lpd = &lpdr->dl_szList[0];
		lpdr->dl_sz = lstrlen(lpd);
//		lpdr->dl_sel = ch + 1;	// Put this OUT OF RANGE
		lpdr->dl_sel = dcount + 1;	// Put this OUT OF RANGE
		lpdr->dl_soff = MXDRVB;	// and this too ...
		lpdr->dl_drv = MXDRVS + 1;
		/* ================================================== */
		lpdr->dl_drives = gdwDrvLst;	/* bit LIST of DRIVES */
		lpdr->dl_count  = gdwDrvCnt;	/* count of bits      */
		pdrl = lpdr;
	}

	//DBWaitOFF;

	/* actually just return a global pointer to an instance */
	return( pdrl );

}

int	GMGetDL( PDRLIST pdrl )
{
	int		i = 0;
	PDRLIST	psdrl;
	if( psdrl = SetStaticDL() )
	{
		memcpy( pdrl, psdrl, sizeof(DRLIST) );
		if( pdrl->dl_ok )
			i++;
	}
	return i;
}

// eof - DvDrives.c
