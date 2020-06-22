
// =============================================================
// GMMem.c
// My own memory manager - Keeps list of allocations and locked
// unlocked memory blocks (segment) ... and locally allocated
// memory.
// At end can output a list of the memory used, and some stats
// =============================================================
#ifndef NO_MEM_STATS // take file out of the loop

#include	<windows.h>
// #ifdef	DIBVIEW2	// as a precompiler SWITCH
#include	"GMWin32.h"	// Some conventions
#include	"GMMemF.h"	// FAR memory diagnostics
// #endif	// DIBVIEW2
#include "Dvfile2.h"
#include "DvIni.h" // for gszCacheData

#ifndef	NDEBUG
// **************************************************************
#define	ADDMEMSTATS
// **************************************************************
#else	// NDEBUG
// **************************************************************
#undef	ADDMEMSTATS
// **************************************************************
#endif	// NDEBUG y/n

#ifndef VFH
#define  VFH(h)   ( h && ( h != INVALID_HANDLE_VALUE ) )
#endif // VHF

#ifdef	ADDMEMSTATS
// =================================================================
// Show ADVICE
#pragma message( "ADVICE: Memory Diags are *** ON ***! See TEMPMEM2.TXT..." )
#include <direct.h>     // for _getcwd

#ifndef	MXFILNAM
#define	MXFILNAM		MAX_PATH
#endif	// MXFILNAM
#ifndef	MLPTR
#define	MLPTR		*
#endif	// MLPTR

// NOWAY - use local allocated memory
//#ifndef	GetTmp
//#	ifdef	DIBVIEW2
//extern	LPSTR	GetBigBuf( void );
//#define	GetTmp		GetBigBuf
//#	else	// !DIBVIEW
//#		ifdef	DISKDB
//extern	LPSTR	GetTmp( void );
//extern	void	RelTmp( void );
//#		else	// !DISKDB
//extern	LPSTR	GetszTmpBuf2( void );
//#define	GetTmp		GetszTmpBuf2
//#		endif	// DISKDB
//#	endif	// DIBVIEW2 y/n
//#endif	// GetTmp

//#define	MXMEMS		1000	// Statistical memory block
#define	MXMEMS		5000	// Statistical memory block
#define	MXASTX		35		// Histogram in astericks
#define  MXMNAME		24		// Max. length of requesters name

#define	mm_InUse		   0x00010000
//#define	mm_Lock		0x00020000	// Now a COUNT used
#define	mm_Local		   0x00100000	// LocalAlloc memory
#define	mm_Sorted		0x80000000

#define	me_Lock			0x00000010	// Freeing with lock!
#define	me_NoLock		0x00000008	// No lock on handle!
#define	me_NoHg			0x00000004	// Handle not found!
#define	me_Reall		   0x00000001	// No more buffer space!
#define	me_NoAll		   0x00000002	// No Allocation!


extern	DWORD	DVWrite(HANDLE fh, LPBYTE pv, DWORD ul );
extern	LPSTR	GetDT4s( int );

#ifdef   EWM5  // ewm project

#else // !#ifdef   EWM5  // ewm project

extern	void	SRSetupPath( LPSTR, LPSTR, LPSTR );
extern	int		DVGetCwd( LPSTR lpb, DWORD siz );
//#ifdef	BVTEST - If LINKED with GMUtils, then use it
extern	int	GMGetFName( LPSTR lpfull, LPSTR lpname );
#define	GetFName		GMGetFName
//#else	// !BVTEST
//extern	int		GetFName( LPSTR lpfull, LPSTR lpname );
//#endif	// BVTEST y/n
// =================================================================
#endif   // #ifdef   EWM5  // ewm project y/n

#endif	// ADDMEMSTATS

void	   FreeAllocs( void );
HGLOBAL	DVGlobalAlloc( UINT, DWORD );
LPSTR	   DVGlobalLock( HGLOBAL );
void	   DVGlobalUnlock( HGLOBAL );
void	   DVGlobalFree( HGLOBAL );


#ifdef	_DVMEMF_H
JPEG_EXTMM	 ExtMM;		// Pad of addresses to pass to USER
#endif	// _DVMEMF_H

#ifdef	ADDMEMSTATS
// =================================================================

char	szDStFile[] = "TEMPMEM2.TXT";
//BOOL	fCreaNew = FALSE;
BOOL	fCreaNew = TRUE;	// Restart the file each time
char	szStFile[MXFILNAM] = { "\0" };
HANDLE	hMemFile   = 0;
BOOL	fAddHandle = TRUE;	// Add the HANDLE copy to listing
BOOL	fForceFree = FALSE;	// Do NOT correct if error???
BOOL	fSortFreq  = FALSE;	// Sort the OUTPUT
DWORD	dwSort;

DWORD	dwMax1Time = 0;
DWORD	dwMax2Time = 0;
DWORD	BgnFs, EndFs;
DWORD iTotLAllocs = 0;  // local allocs
DWORD iTotGAllocs = 0;  // global allocs

char	szTmpOutBuf[256];

typedef struct tagMEMSTR {
	HGLOBAL	mHand;
	DWORD	mSize;
	DWORD	mFlag;
	DWORD	mLocks;
	DWORD	mCount;
	DWORD	mRank;
	DWORD	mRnk;
	TCHAR	mName[MXMNAME+4]; // requesters name
}MEMSTR, * PMS;
typedef MEMSTR MLPTR LPMEMSTR;

#define	COPYMEMSTR( a, b, c, d )   memcpy( &a[b], &c[d], sizeof(MEMSTR) )
#define	ZEROMEMSTR( a, b )         ZeroMemory( &a[b],    sizeof(MEMSTR) )

typedef struct	{
	DWORD	   mer_Flag;
	LPSTR	   mer_pMsg;
}MERSTR;
typedef MERSTR FAR * LPMERSTR;

MERSTR	MerStr[] = {
	{ me_Lock,		"Free with lock!"	},
	{ me_NoLock,	"No lock on!"		},
	{ me_NoHg,		"Handle not found!" },
	{ me_Reall,		"No buffer space!"	},
	{ me_NoAll,		"No Allocation!"	},
	{ 0,             0 }
};

BOOL	fDnAInit = FALSE;

HGLOBAL	ghAlloc = 0;
#ifdef   PERMLOCK
LPMEMSTR	lpMem   = 0;
#endif   // PERMLOCK
HGLOBAL	ghSort  = 0;
LPMEMSTR	lpSort  = 0;

DWORD	AllocCnt   = 0;
DWORD	MaxMems    = MXMEMS;	// Set at about 5,000 allocations
DWORD	MissedSiz  = 0;
DWORD	MemErrs    = 0;
DWORD	iInsert    = 0;

void	chkmem( void )
{
	int	i;
	i = 0;
}

#ifdef   EWM5     // ewm project
extern   LPTSTR	GetShortName( LPTSTR lps, DWORD dwmx );
VOID  GetFName( LPSTR lpnm, LPSTR lpb )   // &szFN[0] );	// 	// GMGetFName
{
   LPTSTR lps = GetShortName( lpnm, (MXMNAME - 1) );
   strcpy(lpb,lps);
}
#endif   // #ifdef   EWM5     // ewm project

// set a memory error
void	SetMemErrs( DWORD flg )
{
	MemErrs |= flg;   // add in the ERROR TYPE flag
}

void	Add2Allocs( HGLOBAL hg, DWORD siz, DWORD dwFlg )
{
	LPMEMSTR	lpM;
	DWORD		i, j;
	HGLOBAL		gc;
	DWORD		csz;
	DWORD		cfg;

//	if( siz == 49 )
//     chkmem();

	if( !fDnAInit )
	{
#ifdef   PERMLOCK
		lpMem = 0;	// Init to ZERO
#endif   // PREMLOCK
		if( ghAlloc = GlobalAlloc( GHND,
			(sizeof(MEMSTR) * (MaxMems+4)) ) )
		{
         // Got memory allocation
			if( lpM = (LPMEMSTR) GlobalLock( ghAlloc ) )
			{
            // Initialise as desired
				for( i = 0; i < MaxMems; i++ )
				{
					ZEROMEMSTR( lpM, i );
				}
			}
#ifdef  PERMLOCK
			lpMem = lpM;
#endif   // PERMLOCK   
			lpM = 0;
		}

		fDnAInit = TRUE;
		AllocCnt = 0;

	}

#ifdef   PERMLOCK
	if( lpM = lpMem )
#else // !PERMLOCK
   if( ( ghAlloc ) &&
       ( lpM = GlobalLock(ghAlloc) ) )
#endif   // #ifdef   PERMLOCK y/n
	{
			i = 0;
			if( AllocCnt == 0 )
			{
				goto AddHere;
			}
			else	// This is the next ++ allocation
			{
				if( AllocCnt < MaxMems )
				{
DoInsert:
					for( i = 0; i < AllocCnt; i++ )
					{
						// Search for a SIZE = 0, and
						// NO INUSE Flag
						if( (lpM[i].mSize == 0) &&
							!(lpM[i].mFlag & mm_InUse) )
						{
							// We have an available slot here
							// Note i will be less than current
							break;	// so go use it.
						}
					}
					// Either found a SIZE = 0 & NO InUse flag
					// or i = current AllocCnt, so make a new
					// record of the allocation.
AddHere:
					// If it is a NEW, then
					if( i == AllocCnt )
					{
						//ZEROMEMSTR( lpM, i );
						AllocCnt++;
					}
					lpM[i].mHand  = hg;
					lpM[i].mCount = 1;
					lpM[i].mSize  = siz;
					lpM[i].mFlag  = (mm_InUse | dwFlg);
					lpM[i].mLocks = 0;	// Start LOCK count to NONE
					iInsert = i;
				}
				else	// ELSE NO CURRENT SPACE - over 1,000 rec.s
				{	// We MUST overwrite some other record.
					// Or increase the MXMEMS count!!!
					for( i = 0; i < AllocCnt; i++ )
					{
						// Find first with no INUSE flag
						if( !(lpM[i].mFlag & mm_InUse) )
						{
							// Add this to "missed"
							MissedSiz += lpM[i].mSize;
							// move all the balance up one
							for( j = i; j < AllocCnt; j++ )
							{
								gc = lpM[j+1].mHand;	// Get Next
								csz= lpM[j+1].mSize;
								cfg= lpM[j+1].mFlag;
								lpM[j].mLocks = lpM[j+1].mLocks;
								COPYMEMSTR( lpM, j, lpM, (j+1) );
								lpM[j].mHand = gc;	// Insert Current
								lpM[j].mSize = csz;
								lpM[j].mFlag = cfg;
							}
							lpM[j].mHand = 0;	// Set end as zero
							lpM[j].mSize = 0;
							lpM[j].mFlag = 0;
							lpM[j].mLocks = 0;	// End record is ZERO
							if( AllocCnt )
								AllocCnt--;	// We have DISCARDED an earlier record
							// no big deal. This is for memory stats ONLY.
							i = j;	// Use this LAST record
							break;
						}
					}
					// If we found a PLACE to put this NEW Alloc
					if( AllocCnt < MaxMems )
					{
						goto DoInsert;	// Then go ADD it
					}
					chkmem();
					SetMemErrs( me_Reall );
				}
			}
#ifndef  PERMLOCK
      if( ghAlloc )
         GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
	}
}

typedef	LPSTR	(*GTNM) (DWORD);

static GTNM	lpGetName = 0;

void	SetGetName( LPVOID lpv )
{
	lpGetName = lpv;
}

//void	WriteStats( LPMEMSTR lpM, DWORD dwCnt, DWORD dwMax )
void	WriteStats( PMS lpM, DWORD dwCnt, DWORD dwMax )
{
	LPSTR		lps;
	HANDLE		hf;
	OFSTRUCT	ofs;
	DWORD		i, j, nj;
	DWORD		fs, bfs, iErrCnt;		// Current written file size ...
	// and for SORTING the list ...
	DWORD		ii, jj, mins, maxs, ncnt, rcnt;
	DWORD		dw;
	DWORD		dwc;
	DWORD		dwtot;
	DWORD		wjj;
	LPSTR		lpsz;
	LPSTR		lptmp;
	LPSTR		lpnm;

//	chkmem();
	hMemFile = 0;		// NO FILE
	nj = 0;
	fs = 0;
	//if( !(lptmp = GetTmp() ) )	// pointer to 1K++ (static) block
	if( !(lptmp = LocalAlloc(LPTR,(1024*2))) )	// pointer to 1K block
	{
		// FAILED??????
		return;
	}
	lps = &lptmp[1024];

	if( fCreaNew )
	{
Use_Create:
		hf = DVOpenFile( &szStFile[0],
			&ofs,
			( OF_CREATE | OF_WRITE ) );
		BgnFs = 0;
	}
	else
	{
		if( ( hf = DVOpenFile( &szStFile[0], &ofs, (OF_READ | OF_WRITE) ) ) &&
			( hf !=  INVALID_HANDLE_VALUE) ) //HFILE_ERROR
		{
			fs = DVSeekEnd( hf );
			BgnFs = fs;
		}
		else
		{
			goto	Use_Create;
		}
	}
	if( VFH(hf) )
	{
		// Pass this BACK to be closed later
		
		hMemFile = hf;
		bfs = fs;		// Keep the BEGIN length.
		//lps = &lptmp[1024];
		// Put DATE/TIME Stamp
		lstrcpy( lps, GetDT4s( 1 ) );	// Get Date,Time + Cr/Lf
		if( ( j = lstrlen( lps ) ) &&
			( j == DVWrite( hf, lps, j ) ) )
		{
			fs += j;
		}

		// Either CREATED or moved to end, so Put HEADING
		wsprintf( lps,
			"Total of %u Local, %u Global allocs, %u recorded (Max.Stat=%u)\r\n",
         iTotLAllocs, // local allocs
         iTotGAllocs, // global allocs
			dwCnt,
			dwMax );
		if( j = lstrlen( lps ) )
		{
			fs += DVWrite( hf, lps, j );
		}
		
		j = 0;
		iErrCnt = 0;

		// Check for ANY errors, or apparent errors ...
		// ============================================
		for( i = 0; i < dwCnt; i++ )
		{
			if( ( lpM[i].mHand ) ||
				( lpM[i].mSize ) )
			{
				if( lpM[i].mHand )
				{
					// Handle not freed correctly = WARNING
					// ====================================
					iErrCnt++;
				}
			}
		}
		fSortFreq = FALSE;
		ghSort = 0;
		lpSort = 0;
		ncnt = 0;	// No new SORTED records ...
		if( iErrCnt == 0 )
		{
			fAddHandle = FALSE;	// No HANDLE need be added.
			lstrcpy( lps,
				"No unfreed memory errors noted!\r\n" );
			if( j = lstrlen( lps ) )
			{
				fs += DVWrite( hf, lps, j );
				ii = GlobalSize( ghAlloc );
				ii += 2 * sizeof( MEMSTR );
				if( ii < ((dwCnt+1) * sizeof(MEMSTR)) )
					ii = ((dwCnt+1) * sizeof(MEMSTR));

				if( ( ghSort = GlobalAlloc( GHND, ii ) ) &&
					( lpSort = (LPMEMSTR) GlobalLock( ghSort ) ) )
				{
					fSortFreq = TRUE;
					ii = 0;
					jj = 0;
					mins = (DWORD)-1;
					maxs = 0;
					ncnt = 0;
					rcnt = 0;
					dw = 1;
					for( i = 0; i < dwCnt; i++ )
					{
						if( lpM[i].mSize )
						{
							if( ncnt == 0 )
							{
								COPYMEMSTR( lpSort, ncnt, lpM, i );
								rcnt++;
								lpSort[ncnt].mSize = lpM[i].mSize;
								lpSort[ncnt].mCount = 1;
								lpSort[ncnt].mRank = rcnt;
								ncnt++;
							}
							else if( ncnt < dwCnt )
							{
								for( ii = 0; ii < ncnt; ii++ )
								{
									// If an allocation
									if( lpSort[ii].mSize == lpM[i].mSize )
									{
										dw = lpSort[ii].mCount;
										dw++;	// Bump the COUNT of THIS Size used
										lpSort[ii].mCount = dw;
										break;
									}
								}	// cycle through ncnt
								if( ii == ncnt )
								{	// NOT FOUND
									// Add a NEW record.
									COPYMEMSTR( lpSort, ncnt, lpM, i );
									lpSort[ncnt].mSize = lpM[i].mSize;
									lpSort[ncnt].mCount = 1;
									rcnt++;
									lpSort[ncnt].mRank = rcnt;
									ncnt++;
								}
							}	// ncnt = 0 or not
						}	// If mSize = Allocated.
					}	// for dwCnt
					if( ncnt &&
						( ncnt <= dwCnt ) )
					{
						// We have a FREQUENCY List
						// Now to ORDER IT
						DWORD	rnk;
						BOOL	chg;
						rnk = 0;
						dwtot = 0;
						for( i = 0; i < ncnt; i++ )
						{
							if( lpSort[i].mSize > maxs )
							{
								maxs = lpSort[i].mSize;
							}
							if( lpSort[i].mSize < mins )
							{
								mins = lpSort[i].mSize;
							}
							dwtot += lpSort[i].mSize * lpSort[i].mCount;
						}
						rcnt = 0;
						chg = FALSE;
						rnk = 1;
						for( ii = 0; ii < ncnt; ii++ )
						{
							lpSort[ii].mFlag &= ~mm_Sorted;
						}
						rnk = 1;
						for( ii = 0; ii < ncnt; ii++ )
						{
							dwc = (DWORD)-1;	// Init TOO LARGE
							for( i = 0; i < ncnt; i++ )
							{
								if( !(lpSort[i].mFlag & mm_Sorted) )
								{
									if( lpSort[i].mSize < dwc )
									{
										dwc = lpSort[i].mSize;
										jj = i;
									}
								}
							}
							lpSort[jj].mFlag |= mm_Sorted;
							lpSort[jj].mRnk = rnk;
							rnk++;	// Bump the rank
						}
#ifdef	ADDSORT1
						for( ii = 0; ii < ncnt; ii++ )
						{
							dwc = lpSort[ii].mSize;
							for( i = 0; i < (ncnt-1); i++ )
							{
								if( lpSort[i].mSize > lpSort[i+1].mSize )
								{
									DWORD	dwx, dwnx;
									dwx = lpSort[i].mRank;
									dwnx = lpSort[i+1].mRank;
									if( dwx < dwnx )
									{
										lpSort[i].mRank = dwnx;
										lpSort[i+1].mRank = dwx;
										if( dwx > rnk )
											rnk = dwx;
										chg = TRUE;	// We switched RANKS
									}
								//maxs = lpSort[i].mSize;
								}
								//if( lpSort[i].mSize < mins )
								//{
								//	mins = lpSort[i].mSize;
								//}
							}
							if( !chg )
							{
								break;
							}
						}
#endif	// ADDSORT1
						rnk = 1;
						// Do the OUPPUT of the LIST
						// Using the "Rank" to order the display
						// =====================================
						//lps = &buf[0];

						dwc = dwMax2Time / 1024;	// Get 'K' bytes
						if( dwc == 0 )
							wsprintf( lptmp, "%u ", dwMax2Time );
						else if( dwc < 1024 )
							wsprintf( lptmp, "%uK", dwc );
						else
							wsprintf( lptmp, "%uM", (dwc / 1024));

						dwc = dwtot / 1024;
						if( (dwtot % 1024) >= (1024 / 2) )
							dwc++;
						if( dwc < 1024 )
						{
							lpsz = "K";
						}
						else
						{
							dwc = dwc / 1024;
							lpsz = "M";
						}
                  // Output the HEADER line
                  // ===========================================================
                  if( ncnt == 1 )
                  {
						   wsprintf( lps,
							   "Only 1 size of Allocation of %u Bytes each.\r\n"
							   "Max. at a Time: %s ; G.Total: %u%s bytes - "
                        "Ordered per Size (F=Freq.)\r\n",
							   mins,
							   lptmp,	// Formatted to K or M as needed
							   dwc,
							   lpsz );
                  }
                  else
                  {
						   wsprintf( lps,
							   "Range of %u different Allocations (%u-%u Bytes)\r\n"
							   "Max. at a Time: %s ; G.Total: %u%s bytes - "
                        "Ordered per Size (F=Freq.)\r\n",
							   ncnt,
							   mins,
							   maxs,
							   lptmp,	// Formatted to K or M as needed
							   dwc,
							   lpsz );
                  }
						if( wjj = lstrlen( lps ) )
							fs += DVWrite( hf, lps, wjj );
                  // run thru the list
						for( ii = 0; ii < ncnt; ii++ )
						{	// Outer count - 1 for each record
							for( i = 0; i < ncnt; i++ )
							{	// Find the MATCH
//								if( lpSort[i].mRank == rnk )
								if( lpSort[i].mRnk == rnk )
								{
									DWORD	dww, dw2;
									wsprintf( lps,
										"%4u: %8lu %s%s",
										rnk,
										lpSort[i].mSize,
										(LPSTR)(lpSort[i].mFlag & mm_InUse ? "I" : " "),
										(LPSTR)(lpSort[i].mLocks ? "L" : " ") );
									dwc = lpSort[i].mCount;
									if( dwc > MXASTX )
										dww = MXASTX;
									else
										dww = dwc;
                           // fill out the FREQUENCY graph
									for( dw2 = 0; dw2 < dww; dw2++ )
									{
										if( (dw2 + 1) < MXASTX )
											lstrcat( lps, "*" );
										else
											lstrcat( lps, "+" );
									}
									for( ; dw2 < MXASTX; dw2++ )
									{
										lstrcat( lps, " " );
									}
									wsprintf( (lps + lstrlen(lps)),
											" (F=%3u)",
											dwc );
									if( lpSort[i].mFlag & mm_Local )
										lstrcat( lps, " Local" );
                           // ==============================================
                           // and if it has a NAME, append the NAME
                           // but what about MULTIPLE names for this size???
									if( lpSort[i].mName[0] )
									{
										lstrcat( lps, " " );
										lstrcat( lps, &lpSort[i].mName[0] );
									}
									else if( ( lpGetName ) &&
										( lpnm = (*lpGetName)(lpSort[i].mSize) ) )
									{
										lstrcat( lps, " " );
										lstrcat( lps, lpnm );
									}
                           // ==============================================

									// Add Cr/Lf end of file line
									lstrcat( lps, "\r\n" );
									// Get length
									if( wjj = lstrlen( lps ) )	// and
										fs += DVWrite( hf, lps, wjj );	// add 2 file
									lps[0] = 0;	// Done a line
									jj = 0;
									break;	// Done this record
								}
							}	// Inner search for "Rank"
							rnk++;	// Bump to NEXT rank
						}
						wsprintf( lps,
							"Done Range of %u Sizes\r\n",
							ncnt );
						if( ( wjj = lstrlen( lps ) ) &&
							( wjj == DVWrite( hf, lps, wjj ) ) )
						{
							fs += wjj;
							goto Chk_Missed;
						}
					}	// ncnt = We have a SORT of RANKED Freuency list
				}	// Got SORT memory request
			}
			j = 0;
		}	// No errors = Then SORT the list.

		j = 0;	// Ensure ZERO start
		for( i = 0; i < dwCnt; i++ )
		{
			if( lpM[i].mSize )
			{
				if( fAddHandle )
				{
					wsprintf( (lps+j), "%6lu %#08x %s%s ",
						lpM[i].mSize,
						lpM[i].mCount,
						(LPSTR)(lpM[i].mFlag & mm_InUse ? "I" : " "),
						(LPSTR)(lpM[i].mLocks ? "L" : " ") );

               // if given a requester's NAME
               // ============================================
               if( lpM[i].mName[0] )
               {
                  lstrcat(lps,"(");
                  lstrcat(lps, &lpM[i].mName[0]);
                  lstrcat(lps,") ");
               }
               // ============================================
				}
				else
				{
					wsprintf( (lps+j), "%6lu %s%s ", lpM[i].mSize,
						(LPSTR)(lpM[i].mFlag & mm_InUse ? "I" : " "),
						(LPSTR)(lpM[i].mLocks ? "L" : " ") );
				}
				// Get the NEW size of the line so far
				nj = lstrlen( lps );	// Now, minus prev = added
				if( (nj + (nj - j)) > 79 )
				{	// Oops, OVER the MAX. so MUST Cr/Lf OUT
					lstrcat( lps, "\r\n" );
					j = lstrlen( lps );
					fs += DVWrite( hf, lps, j );
					lps[0] = 0;
					j = 0;
				}
				j = lstrlen( lps );
				if( j > 65 )
				{
					lstrcat( lps, "\r\n" );
					j = lstrlen( lps );
					fs += DVWrite( hf, lps, j );
					lps[0] = 0;
					j = 0;
				}
			}
		}

		if( j )
		{
			lstrcat( lps, "\r\n" );
			j = lstrlen( lps );
			fs += DVWrite( hf, lps, j );
		}

		lstrcpy( lps, "End of Size list.\r\n" );
		if( j = lstrlen( lps ) )
			fs += DVWrite( hf, lps, j );

Chk_Missed:

		if( MissedSiz )
		{
			wsprintf( lps, "Some allocations were over written. Total %lu\r\n",
				MissedSiz );
			if( j = lstrlen( lps ) )
				fs += DVWrite( hf, lps, j );
		}

      // ANY RUNTIME ERRORS NOTED
      // ========================
		if( MemErrs )
		{
			LPMERSTR	lpme;
			wsprintf( lps,
				"Some RUNTIME memory errors were noted! (Flag = %X)\r\n",
				MemErrs );
			if( j = lstrlen( lps ) )
				fs += DVWrite( hf, lps, j );
			lpme = &MerStr[0];
			*lps = 0;
			while( lpme->mer_Flag && lpme->mer_pMsg )
			{
				if( MemErrs & lpme->mer_Flag )
				{
					if( *lps )
						lstrcat( lps, " " );
					else
						lstrcpy( lps, "Flag(s): " );
					lstrcat( lps, lpme->mer_pMsg );
				}
				lpme++;
			}
			if( *lps )
			{
				lstrcat( lps, "\r\n" );
				if( j = lstrlen(lps) )
					fs += DVWrite( hf, lps, j );
			}
		}

		EndFs = fs;
		// NOTE: Close is done back in main.
		// hMemFile = hf;
		// DVlclose( hf );
	}
//#ifdef	DISKDB
//	RelTmp();
//#endif	// DISKDB
   LocalFree( lptmp );	// pointer to 2K block

}	// WriteStats( lpMEMSTR, DWORD, DWORD )

// =================================================================
#endif	/* ADDMEMSTATS */


void	MemIni( LPSTR lpd )
{
#ifdef	ADDMEMSTATS
#ifdef   EWM5  // included in ewm project
#define  DDVGetCwd   _getcwd     // ( lpd, MAX_PATH );
   LPTSTR   lpf = szStFile;
   //strcpy( &szStFile[0], &szDStFile[0] );
   strcpy(lpf,lpd);
   if( lpf[ (strlen(lpf) - 1) ] != '\\' )
      strcat(lpf,"\\");
   strcat(lpf, szDStFile);
#else // !EWM5
	SRSetupPath( &szStFile[0], lpd, &szDStFile[0] );
#define  DDVGetCwd   DVGetCwd    // _getcwd     // ( lpd, MAX_PATH );
#endif   // #ifdef   EWM5  // included in ewm project y/n
#endif	// ADDMEMSTATS
}

void	InitAllocs( void )
{
#ifdef	ADDMEMSTATS
	char	buf[260];
	LPSTR	lpb;
	lpb = &buf[0];
	buf[0] = 0;
	if (gszCacheData[0]) {
		strcpy(lpb, gszCacheData);
	}
	else {
		DDVGetCwd(lpb, 267);
	}
	if (buf[0])
		MemIni( lpb );
#endif	// ADDMEMSTATS
}

void	FreeAllocs( void )
{
#ifdef	ADDMEMSTATS
// =================================================================
	LPMEMSTR	lpM;
	DWORD	i, j, k;
	char	buf[256];
	LPSTR	lpb;
	HGLOBAL	hg;

//	chkmem();
	k = 0;
	EndFs = 0;
	BgnFs = 0;
	if( AllocCnt )
	{
#ifdef   PERMLOCK
		if( lpM = lpMem )
#else // !PERMLOCK
      if( ( ghAlloc ) &&
          ( lpM = (LPMEMSTR) GlobalLock( ghAlloc ) ) )
#endif   // PERMLOCK y/n
		{
			WriteStats( lpM, AllocCnt, MaxMems );
			lpb = &buf[0];
			for( i = 0; i < AllocCnt; i++ )
			{
				lpb[0] = 0;
				if( hg = lpM[i].mHand )
				{
					k++;
					wsprintf( lpb, "WARNING: Handle %x not 0!",
						hg );
					if( ( lpM[i].mFlag & mm_InUse ) &&
						( !(lpM[i].mFlag & mm_Local) ) &&
						 ( lpM[i].mLocks ) )
					{
						while( lpM[i].mLocks )
						{
							GlobalUnlock( hg );
							lpM[i].mLocks--;
						}
						lstrcat( lpb, " Locked!" );
					}
					if( lpM[i].mFlag & mm_InUse )
					{
						if( fForceFree )
						{
							if( lpM[i].mFlag & mm_Local )
								LocalFree( (HLOCAL)hg );
							else
								GlobalFree( hg );
						}
						lpM[i].mHand = 0;
						lpM[i].mFlag = 0;
						lstrcat( lpb, " InUse!" );
					}
				}
				if( (j = lstrlen(lpb)) &&
					hMemFile &&
					(hMemFile != INVALID_HANDLE_VALUE) )
				{
					lstrcat( lpb, "\r\n" );
					EndFs += DVWrite( hMemFile, lpb, (j+2) );
				}
			}
//			GlobalUnlock( hAlloc );
			GlobalFree( ghAlloc );
			if( ( hMemFile ) &&
				( hMemFile != INVALID_HANDLE_VALUE) )
			{
				if( k )
				{
					wsprintf( lpb,
						"WARNING: Memory Leak Count = %u\r\n", k );
				}
				else
				{
					lstrcpy( lpb,
						"Appears NO memory leak errors.\r\n" );
				}
				if( j = lstrlen( lpb ) )
					EndFs += DVWrite( hMemFile, lpb, j );

				// FINAL ACTION before closing the FILE.
				// ====================================
				if( EndFs > BgnFs )
				{
					int	iAdding;
					// Put DATE/TIME Stamp at END also
					lstrcpy( lpb, GetDT4s( 1 ) );	// Get Date,Time + Cr/Lf
					if( iAdding = lstrlen( lpb ) )
						DVWrite( hMemFile, lpb, iAdding );
				}
				// =====================================
				DVlclose( hMemFile );
			}
			hMemFile = 0;
#ifndef   PERMLOCK
         if( ghAlloc )
            GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
		}
	}

	if( lpSort && ghSort )
		GlobalUnlock( ghSort );
	if( ghSort )
		GlobalFree( ghSort );
	lpSort = 0;
	ghSort = 0;
// =================================================================
#endif		// ADDMEMSTATS
}

HGLOBAL	DVGlobalAlloc( UINT typ, DWORD siz )
{
   HGLOBAL	hg = GlobalAlloc( typ, siz );
#ifdef	ADDMEMSTATS
   iTotGAllocs++;
// =================================================================
//	if( ( siz == 5200 ) &&
//		( hg == (HGLOBAL)0x006c005e ) )
//		chkmem();
//	if( siz == 32 )
//		chkmem();

	if( hg )
	{
		Add2Allocs( hg, siz, 0 );
		dwMax1Time += siz;		// Add this into ONE TIME size
		if( dwMax1Time > dwMax2Time )
			dwMax2Time = dwMax1Time;	// Get LARGEST at ONE TIME
	}
	else
	{
		chkmem();
		SetMemErrs( me_NoAll );	/* No allocation */
	}

// =================================================================
#endif	// ADDMEMSTATS
   return( hg );
}

// FIX980516 - Added the NEW SERVICE where a NAME can be passed
// and this NAME will be shown on close, if ADDMEMSTATS of course
HGLOBAL	DVGAlloc( LPSTR lpnm, UINT typ, DWORD siz )
{
	HGLOBAL	hg;
	int		i;

	hg = 0;
	i = 0;
	if( hg = DVGlobalAlloc( typ, siz ) )
	{
#ifdef	ADDMEMSTATS
      if( ( lpnm                ) &&
			 ( i = lstrlen( lpnm ) ) )
      {
   	   LPMEMSTR	lpM;
         TCHAR szFN[MAX_PATH];
#ifdef   PERMLOCK
		  if( lpM = lpMem  )
#else // !PERMLOCK
        if( ( ghAlloc ) &&
            ( lpM = GlobalLock(ghAlloc) ) )
#endif   // PERMLOCK y/n
        {

         // get the REQUESTORS name
         // ====================================================
			if( i < MXMNAME )
			{
            // if just a SHORT name, copy it
				lstrcpy( &lpM[iInsert].mName[0], lpnm );
			}
			else
			{
            // else try to shorten by ONLY getting the FILE NAME
				GetFName( lpnm, &szFN[0] );	// 	// GMGetFName
				// FIX980530 - Use GMGetFName
				if( i = lstrlen( &szFN[0] ) )
				{
					if( i < MXMNAME )
					{
                  // if short enough now, copy as is
						lstrcpy( &lpM[iInsert].mName[0], &szFN[0] );
					}
					else
					{
                  // else ONLY copy the ending portion
						int		j, k;
						k = 0;
						for( j = (i - MXMNAME); j < i; j++ )
						{
							lpM[iInsert].mName[k++] = szFN[j];
						}
						lpM[iInsert].mName[k] = 0;
					}
				}
			}
         // ====================================================

#ifndef  PERMLOCK
         if(ghAlloc)
            GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
        }
      }
#endif	// ADDMEMSTATS
	}
	return hg;
}

HGLOBAL	DVGFree( HGLOBAL hg )
{
   DVGlobalFree(hg);
   return 0;
}

int	DVVerifyHandle( HGLOBAL hg )
{
	int		iRet = 0;
#ifdef	ADDMEMSTATS
	LPMEMSTR	lpM;
	DWORD	i;
// =================================================================
#ifdef   PERMLOCK
	if( lpM = lpMem )
#else // !PERMLOCK
	if( ( ghAlloc ) &&
       ( lpM = GlobalLock(ghAlloc) ) )
#endif   // PERMLOCK y/n
	{
		for( i = 0; i < AllocCnt; i++ )
		{
			if( lpM[i].mHand == hg )
			{
				// Handle found
				iRet = 1;
				break;
			}
		}
#ifndef  PERMLOCK
      if( ghAlloc )
         GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
	}
	if( !iRet )
		chkmem();
#else

	// NO SERVICE
	iRet = (int)-1;

#endif	// ADDMEMSTATS
//	if( !iRet )
//		chkmem();

	return iRet;
}

LPSTR	DVGlobalLock( HGLOBAL hg )
{
	LPSTR	lps;
#ifdef	ADDMEMSTATS
	LPMEMSTR	lpM;
	DWORD	i;
//	chkmem();
#endif	// ADDMEMSTATS
	// *********************
	lps = GlobalLock( hg );
	// *********************
#ifdef	ADDMEMSTATS
// =================================================================
#ifdef   PERMLOCK
	if( lpM = lpMem )
#else // !#ifdef   PERMLOCK
   if( ( ghAlloc ) &&
       ( lpM = GlobalLock(ghAlloc) ) )
#endif   // #ifdef   PERMLOCK y/
	{
		for( i = 0; i < AllocCnt; i++ )
		{
			if( (lpM[i].mFlag & mm_InUse) &&
				(lpM[i].mHand == hg) )
			{
				// Handle found ...
					break;
			}
		}
		if( i < AllocCnt )
		{
			lpM[i].mLocks++;	// Bump our LOCK Count
		}
		else
		{
			chkmem();
			SetMemErrs( me_NoHg );	/* Global handle not found! */
		}
#ifndef  PERMLOCK
      if(ghAlloc)
         GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
	}
#endif	// ADDMEMSTATS
	return( lps );
}

void	DVGlobalUnlock( HGLOBAL hg )
{
#ifdef	ADDMEMSTATS
   LPMEMSTR	lpM;
   DWORD	i;
//	chkmem();
#endif	/* ADDMEMSTATS */
	GlobalUnlock( hg );
#ifdef	ADDMEMSTATS
#ifdef   PERMLOCK
	if( lpM = lpMem )
#else // !#ifdef   PERMLOCK
   if( ( ghAlloc ) &&
       ( lpM = GlobalLock(ghAlloc) ) )
#endif   // #ifdef   PERMLOCK y/
	{
		for( i = 0; i < AllocCnt; i++ )
		{
			if( ( lpM[i].mFlag & mm_InUse ) &&
				( !(lpM[i].mFlag & mm_Local) ) &&
				( lpM[i].mHand == hg ) )
					break;	// Got it
		}
		if( i < AllocCnt )
		{
			if( lpM[i].mLocks )
			{
				lpM[i].mLocks--;		/* Logical UNLOCK */
			}
			else
			{
				chkmem();
				SetMemErrs( me_NoLock );	/* No Lock on handle */
			}
		}
		else
		{
			chkmem();
			SetMemErrs( me_NoHg );
		}
#ifndef  PERMLOCK
      if(ghAlloc)
         GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
	}
#endif	// ADDMEMSTATS
}

void	DVGlobalFree( HGLOBAL hg )
{
#ifdef	ADDMEMSTATS
	LPMEMSTR	lpM;
	DWORD	i;
//	chkmem();
#ifdef   PERMLOCK
	if( lpM = lpMem )
#else // !#ifdef   PERMLOCK
   if( ( ghAlloc ) &&
       ( lpM = GlobalLock(ghAlloc) ) )
#endif   // #ifdef   PERMLOCK y/
	{
		for( i = 0; i < AllocCnt; i++ )
		{
			if( ( lpM[i].mFlag & mm_InUse ) &&
				( !(lpM[i].mFlag & mm_Local) ) &&
				( lpM[i].mHand == hg ) )
					break;
		}
		if( i < AllocCnt )
		{
			if( lpM[i].mLocks )
			{
				chkmem();
				SetMemErrs( me_Lock );	/* Free with lock */
				while( lpM[i].mLocks )
				{
					GlobalUnlock( lpM[i].mHand );
					lpM[i].mLocks--;
				}
				lpM[i].mLocks = 0;
			}
			lpM[i].mHand = 0;		// Clear our HANDLE
			lpM[i].mFlag &= ~mm_InUse;	// And IN USE Flag
			dwMax1Time -= lpM[i].mSize;	// Reduce by this SIZE
		}
		else
		{
			chkmem();
			SetMemErrs( me_NoHg );	/* Handle not found */
		}
#ifndef  PERMLOCK
      if(ghAlloc)
         GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
	}
#endif	// ADDMEMSTATS
	GlobalFree( hg );	// FREE it anyway
}


#ifdef	_DVMEMF_H
// ==================================================
// And some FAR Services
// =====================
#define	MXLCNT		400
typedef	struct	tagLCKM {
	DWORD		lm_Size;
	HGLOBAL		lm_Hand;
	void MLPTR	lm_Ptr;
}LCKM;
typedef LCKM MLPTR LPLCKM;

HGLOBAL	hLockMem = 0;
DWORD	dwLockCnt = 0;
DWORD	dwMaxLock = MXLCNT;	// Start

void MLPTR	FLOCALALLOC( DWORD dws )
{
	void MLPTR lp = 0;
	LPLCKM		lpLM;
	DWORD		dw;

	if( hLockMem == 0 )
		hLockMem = GlobalAlloc( GHND, ((dwMaxLock+2) * sizeof( LCKM )) );
	if( hLockMem &&
		( lpLM = (LPLCKM) GlobalLock( hLockMem ) ) )
	{
		dw = 0;
		if( dwLockCnt )
		{
			for( dw = 0; dw < dwLockCnt; dw++ )
			{
				if( lpLM->lm_Hand == 0 )
					break;
				lpLM++;
			}
		}
		if( dw == dwLockCnt )
		{
			if( dwLockCnt >= dwMaxLock )
			{

			}
		}
		lpLM->lm_Size = dws;
		lpLM->lm_Hand = 0;
		lpLM->lm_Ptr = 0;
		if( ( lpLM->lm_Hand = DVGlobalAlloc( GHND, dws ) ) &&
			( lpLM->lm_Ptr  = DVGlobalLock( lpLM->lm_Hand ) ) )
		{
			if( dw == dwLockCnt )
				dwLockCnt++;
		}
		else if( lpLM->lm_Hand )
		{
			DVGlobalFree( lpLM->lm_Hand );
			lpLM->lm_Hand = 0;
		}
		lp = lpLM->lm_Ptr;
	}
	return lp;
}

void MLPTR	FLOCALFREE( void MLPTR ptr )
{
	void MLPTR	lp;
	LPLCKM		lpLM;
	DWORD		dw;
	HGLOBAL		hg;

	lp = (void MLPTR)-1;
	if( hLockMem &&
		( lpLM = (LPLCKM) GlobalLock( hLockMem ) ) )
	{
		dw = 0;
		if( dwLockCnt )
		{
			for( dw = 0; dw < dwLockCnt; dw++ )
			{
				if( ( hg = lpLM->lm_Hand ) &&
					( lpLM->lm_Ptr == ptr ) )
					break;
				lpLM++;
			}
		}
		if( hg &&
			( dw < dwLockCnt ) )
		{
			DVGlobalUnlock( hg );
			DVGlobalFree( hg );
			lpLM->lm_Size = 0;
			lpLM->lm_Hand = 0;
			lpLM->lm_Ptr = 0;
			lp = 0;
		}
	}
	return lp;
}

HGLOBAL	FGLOBALALLOC( DWORD dws )
{
	return( DVGlobalAlloc( GHND, dws ) );
}
void MLPTR	FGLOBALLOCK( HGLOBAL hg )
{
	return( DVGlobalLock( hg ) );
}
void MLPTR	FGLOBALUNLOCK( HGLOBAL hg )
{
	void MLPTR lp = 0;
	DVGlobalUnlock( hg );
	return lp;
}
HGLOBAL	FGLOBALFREE( HGLOBAL hg )
{
	HGLOBAL rhg = 0;
	DVGlobalFree( hg );
	return rhg;
}

void	FDEBUGSTOP( int code );
LPJPEG_EXTMM	GetMemMgr( void )
{
	LPJPEG_EXTMM lpMM;

	lpMM = &ExtMM;
	lpMM->dwMM_Size = sizeof( JPEG_EXTMM );
	// Like malloc and free
	lpMM->EXTLOCALALLOC = &FLOCALALLOC;
	lpMM->EXTLOCALFREE  = &FLOCALFREE;
	// Windows DOUBLE SHIFT
	lpMM->EXTGLOBALALLOC = &FGLOBALALLOC;
	lpMM->EXTGLOBALLOCK = &FGLOBALLOCK;
	lpMM->EXTGLOBALUNLOCK = &FGLOBALUNLOCK;
	lpMM->EXTGLOBALFREE = &FGLOBALFREE;

	// Just a DEBUG tap point - nothing more
	lpMM->EXTDEBUGSTOP = &FDEBUGSTOP;	// Just a DEBUG trap

	return lpMM;
}
#endif	// _DVMEMF_H

void	debugstop( int code )
{
	int	i;
	i = code;
}
void	FDEBUGSTOP( int code )
{
	int		i;
	i = code;
	debugstop( i );
}

// ========================================
// LocalAlloc and LocalFree
HLOCAL	GMLocalAlloc( UINT Type, size_t Size )
{
	HLOCAL	hl;

//	if( Size == 40 )
//		chkmem();
	hl = LocalAlloc( Type, Size );
#ifdef	ADDMEMSTATS
   iTotLAllocs++;
	if( hl )
	{
		Add2Allocs( (HGLOBAL)hl, Size, mm_Local );
		dwMax1Time += Size;		// Add this into ONE TIME size
		if( dwMax1Time > dwMax2Time )
			dwMax2Time = dwMax1Time;	// Get LARGEST at ONE TIME
	}
	else
	{
		chkmem();
		SetMemErrs( me_NoAll );	/* No allocation */
	}
#endif	// ADDMEMSTATS
	return hl;
}

HLOCAL	GMLocalFree( HLOCAL hLoc )
{
	HLOCAL	hl;
#ifdef	ADDMEMSTATS
	LPMEMSTR	lpM;
	DWORD	i;
#endif	// ADDMEMSTATS
	hl = LocalFree( hLoc );
#ifdef	ADDMEMSTATS
//	chkmem();
#ifdef   PERMLOCK
	if( lpM = lpMem )
#else // !#ifdef   PERMLOCK
   if( ( ghAlloc ) &&
       ( lpM = GlobalLock(ghAlloc) ) )
#endif   // #ifdef   PERMLOCK y/
	{
		for( i = 0; i < AllocCnt; i++ )
		{
			if( ( lpM[i].mFlag & mm_Local ) &&
				( lpM[i].mFlag & mm_InUse ) &&
				( lpM[i].mHand == (HGLOBAL)hLoc ) )
					break;
		}
		if( i < AllocCnt )
		{
			lpM[i].mHand = 0;		// Clear our HANDLE
			lpM[i].mFlag &= ~(mm_InUse);	// And IN USE Flag
			dwMax1Time -= lpM[i].mSize;	// Reduce by this SIZE
		}
		else
		{
			chkmem();
			SetMemErrs( me_NoHg );	/* Handle not found */
		}
#ifndef  PERMLOCK
      if(ghAlloc)
         GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
	}
#endif	// ADDMEMSTATS
	return hl;
}

BOOL	GMLocalOK( HLOCAL hLoc )
{
	BOOL		flg;
#ifdef	ADDMEMSTATS
	HLOCAL		hl;
	LPMEMSTR	lpM;
	DWORD		i;
#endif	// ADDMEMSTATS

	flg = (BOOL)-1;		// Start with ERROR FLAG
#ifdef	ADDMEMSTATS
	if( hl = hLoc )
	{
#ifdef   PERMLOCK
	  if( lpM = lpMem )
#else // !#ifdef   PERMLOCK
     if( ( ghAlloc ) &&
       ( lpM = GlobalLock(ghAlloc) ) )
#endif   // #ifdef   PERMLOCK y/
     {
		flg = FALSE;	// We can do the check
		for( i = 0; i < AllocCnt; i++ )
		{
			if( lpM[i].mHand == (HGLOBAL)hLoc )
			{
				flg = TRUE;	// Found it
				break;
			}
		}
#ifndef  PERMLOCK
      if(ghAlloc)
         GlobalUnlock(ghAlloc);
#endif   // !PERMLOCK
     }
	}
#endif	// ADDMEMSTATS
	return flg;
}

#endif   // #ifndef NO_MEM_STATS // take file out of the loop
// eof - End GMMem.c
