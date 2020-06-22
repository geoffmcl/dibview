

// DvTbl32.h


#include	"dv.h"
#include	"DvPal24.h"

#ifdef	DIAGDT
#define		MXOUTBUF		1024
char	szTempTbl[] = "TEMPTBL.TXT";
char	outbuf[MXOUTBUF];

HFILE	hOutFilT = 0;
#define	closeit			CloseOutT()

HFILE OpenFileT( LPSTR lpFileName,	// pointer to filename
				LPOFSTRUCT lpO,	// pointer to buffer for file information
				UINT uStyle	) // action and attributes
{
	HFILE hf;
	hf = OpenFile( lpFileName, lpO, uStyle );
	return hf;
}

void	OpenOutT( void )
{
	OFSTRUCT	of;
	LPSTR		lpf;

	lpf = &szTempTbl[0];
	hOutFilT = OpenFileT( lpf, &of, OF_CREATE | OF_WRITE );
}

// MAKE SURE the standard DOS Cr/Lf is used
// ========================================
int	OutFileT( HFILE hf, LPSTR lpb, int len )
{
	int		i, j, k, wtn;
	char	c, d;

	wtn = 0;
	if( ( hf ) &&
		( hf != HFILE_ERROR ) &&
		( lpb ) &&
		( len ) &&
		(i = lstrlen( lpb ) ) )
	{
		if( i > MXOUTBUF )
		{
			i = MXOUTBUF;
		}
		k = 0;
		d = 0;

		for( j = 0; j < i; j++ )
		{
			c = lpb[j];
			if( c == '\r' )
			{
				if( (j+1) < i )
				{
					if( lpb[j+1] != '\n' )
					{
						outbuf[k++] = c;
						c = '\n';
					}
				}
				else
				{
					outbuf[k++] = c;
					c = '\n';
				}
			}
			else if( c == '\n' )
			{
				if( d != '\r' )
					outbuf[k++] = '\r';
			}
			outbuf[k++] = c;
			d = c;
		}
		if( c != '\n' )
		{
			outbuf[k++] = '\r';
			outbuf[k++] = '\n';
		}
		if( k )
			wtn = _lwrite( hf, &outbuf[0], k );
	}
	return wtn;
}

void	CloseOutT( void )
{
	if( hOutFilT && (hOutFilT != HFILE_ERROR) )
		_lclose( hOutFilT );
	hOutFilT = 0;
}

#define	twrtit( a ) \
{\
	if( hOutFilT == 0 )\
		OpenOutT();\
	if( a && lstrlen(a) && hOutFilT && (hOutFilT != HFILE_ERROR) )\
		OutFileT( hOutFilT, a, lstrlen( a ) );\
}


void	DumpTable( LPCOLRNG2 lpCR, DWORD dwCnt, DWORD dwCnt2 )
{
	LPSTR	lpb;
	DWORD	iOff;
	if( lpCR && dwCnt2 )
	{
		lpb = GetTmp2();
		wsprintf( lpb, "Dumping Table of %u of %u length...\r\n",
			dwCnt2,
			dwCnt );
		twrtit( lpb );
		for( iOff = 0; iOff < dwCnt2; iOff++ )
		{
			//wsprintf( lpb, "{{%3u,%3u,%3u},{%3u,%3u,%3u},{%3u,%3u,%3u}}\r\n",
			wsprintf( lpb, "{{%u,%u,%u},{%u,%u,%u},{%u,%u,%u}}\r\n",
				lpCR[iOff].bl.cMin,
				lpCR[iOff].bl.cMax,
				lpCR[iOff].bl.cVal,
				lpCR[iOff].gr.cMin,
				lpCR[iOff].gr.cMax,
				lpCR[iOff].gr.cVal,
				lpCR[iOff].re.cMin,
				lpCR[iOff].re.cMax,
				lpCR[iOff].re.cVal );
			twrtit( lpb );
		}
		wsprintf( lpb, "END Dumping Table of %u of %u length...\r\n",
			dwCnt2,
			dwCnt );
		twrtit( lpb );
		closeit;
	}
}
#endif	// DIAGDT
//=====================================================
// BOOL	MakeTable( LPHPTBL lpHTbl, int Percent )
//
// Purpose: Allocate memory and build a comparison table
//		using the percentage passed.
//
//=====================================================
BOOL	MakeTable( LPHPTBL lpHTbl, int Percent )
{
	BOOL	flg = FALSE;
	HGLOBAL	hg;
	int		iInc, iRem, iCur, iDec, iCnt, iNxt, iIns, iAve;
	LPCOLRNG2	lpCR, lpc;
	DWORD		dwSz, dwCnt, dwCnt2;
	int			i, j, k;
	int			iIns2, iIns3, iNxt2, iNxt3, iAve2, iAve3;
	int			iDec2, iDec3, iCur2, iCur3;

	hg = 0;
	lpCR = 0;
	iInc = ( Percent * 256 ) / 100;	// Each INCREMENT
	iRem = ( Percent * 256 ) % 100;	// and REMAINDER
	iCur = 0;
	iDec = 0;
	if( lpHTbl &&
		( iInc || iRem ) )
	{
		lpHTbl->iCnt = 0;
		lpHTbl->hTbl = hg;
		iCnt = 0;
		while( iCur < 256 )
		{
			iCnt++;
			iCur += iInc;
			iDec += iRem;
			if( iDec >= 100 )
			{
				iDec -= 100;
				iCur++;
			}
		}
		dwCnt = iCnt * iCnt * iCnt;
		dwSz =  dwCnt * sizeof( COLRNG2 );
		if( ( hg = DVGlobalAlloc( GHND, dwSz ) ) &&
			( lpCR = (LPCOLRNG2) DVGlobalLock( hg ) ) )
		{
			lpc = lpCR;
			dwCnt2 = 0;
			iCur = 0;
			iDec = 0;
			for( i = 0; i < iCnt; i++ )
			{
				iNxt = iCur + iInc;
				iDec += iRem;
				if( iDec >= 100 )
				{
					iDec -= 100;
					iNxt++;
				}
				if( iNxt < 255 )
				{
					if( iCur )
					{
						iIns = iCur + 1;
						//iAve = (iCur + iNxt) / 2;
						iAve = (iIns + iNxt) / 2;
					}
					else
					{
						iIns = iCur;
						iAve = 0;
					}
				}
				else
				{
					iIns = iCur + 1;
					iNxt = 255;
					iAve = 255;
				}
				// Got color 1
				iDec2 = 0;
				iCur2 = 0;
				for( j = 0; j < iCnt; j++ )
				{
					iNxt2 = iCur2 + iInc;
					iDec2 += iRem;
					if( iDec2 >= 100 )
					{
						iDec2 -= 100;
						iNxt2++;
					}
					if( iNxt2 < 255 )
					{
						if( iCur2 )
						{
							iIns2 = iCur2 + 1;
							//iAve2 = (iCur2 + iNxt2) / 2;
							iAve2 = (iIns2 + iNxt2) / 2;
						}
						else
						{
							iIns2 = iCur2;
							iAve2 = 0;
						}
					}
					else
					{
						iIns2 = iCur2 + 1;
						iNxt2 = 255;
						iAve2 = 255;
					}
					// Got color 2
					iDec3 = 0;
					iCur3 = 0;
					for( k = 0; k < iCnt; k++ )
					{
						iNxt3 = iCur3 + iInc;
						iDec3 += iRem;
						if( iDec3 >= 100 )
						{
							iDec3 -= 100;
							iNxt3++;
						}
						if( iNxt3 < 255 )
						{
							if( iCur3 )
							{
								iIns3 = iCur3 + 1;
								//iAve3 = (iCur3 + iNxt3) / 2;
								iAve3 = (iIns3 + iNxt3) / 2;
							}
							else
							{
								iIns3 = iCur3;
								iAve3 = 0;
							}
						}
						else
						{
							iIns3 = iCur3 + 1;
							iNxt3 = 255;
							iAve3 = 255;
						}
						// Got color 3

						// Insert this
						//=======================
						lpc->bl.cMin = (BYTE)iIns;
						lpc->bl.cMax = (BYTE)iNxt;
						lpc->bl.cVal = (BYTE)iAve;

						lpc->gr.cMin = (BYTE)iIns2;
						lpc->gr.cMax = (BYTE)iNxt2;
						lpc->gr.cVal = (BYTE)iAve2;

						lpc->re.cMin = (BYTE)iIns3;
						lpc->re.cMax = (BYTE)iNxt3;
						lpc->re.cVal = (BYTE)iAve3;
						lpc++;
						dwCnt2++;
						//========================

						iCur3 = iNxt3;
						if( iCur3 == 255 )
							break;
					}	// For iCnt of 3rd colors
					iCur2 = iNxt2;
					if( iCur2 == 255 )
						break;
				}	// For iCnt of 2nd colors
				iCur = iNxt;
				if( iCur == 255 )
					break;
			}	// For iCnt of 1st colors
#ifdef	DIAGDT
			DumpTable( lpCR, dwCnt, dwCnt2 );
#endif	// DIAGDT
			DVGlobalUnlock( hg );
			lpHTbl->iCnt = dwCnt2;
			lpHTbl->hTbl = hg;
			flg = TRUE;
		}
		else
		{
			if( hg )
				DVGlobalFree( hg );
		}
	}
	return flg;
}

BOOL	MakeTable2( LPHPTBL lpHTbl, int Percent )
{
	BOOL	flg = FALSE;
	HGLOBAL	hg;
	int		iInc, iRem, iCur, iDec, iCnt, iNxt, iIns, iAve;
	LPCOLRNG2	lpCR, lpc;
	DWORD		dwSz, dwCnt, dwCnt2;
	int			i, j, k;
	int			iIns2, iIns3, iNxt2, iNxt3, iAve2, iAve3;
	int			iDec2, iDec3;

	hg = 0;
	lpCR = 0;
	iInc = ( Percent * 256 ) / 100;	// Each INCREMENT
	iRem = ( Percent * 256 ) % 100;	// and REMAINDER
	iCur = 0;
	iDec = 0;
	if( lpHTbl &&
		( iInc || iRem ) )
	{
		lpHTbl->iCnt = 0;
		lpHTbl->hTbl = hg;
		iCnt = 0;
		while( iCur < 256 )
		{
			iCnt++;
			iCur += iInc;
			iDec += iRem;
			if( iDec >= 100 )
			{
				iDec -= 100;
				iCur++;
			}
		}
		dwCnt = (iCnt+1) * iCnt * (iCnt-1);
		dwSz =  dwCnt * sizeof( COLRNG2 );
		if( ( hg = DVGlobalAlloc( GHND, dwSz ) ) &&
			( lpCR = (LPCOLRNG2) DVGlobalLock( hg ) ) )
		{
			iCur = 0;
			iDec = 0;
			lpc = lpCR;
			dwCnt2 = 0;
			for( i = 0; i < iCnt; i++ )
			{
				iNxt = iCur + iInc;
				iDec += iRem;
				if( iDec >= 100 )
				{
					iDec -= 100;
					iNxt++;
				}
				if( iNxt < 255 )
				{
					if( iCur )
					{
						iIns = iCur + 1;
						iAve = (iCur + iNxt) / 2;
					}
					else
					{
						iIns = iCur;
						iAve = 0;
					}
					lpc->bl.cMin = (BYTE)iIns;
					lpc->bl.cMax = (BYTE)iNxt;
					lpc->bl.cVal = (BYTE)iAve;

					lpc->gr.cMin = (BYTE)iIns;
					lpc->gr.cMax = (BYTE)iNxt;
					lpc->gr.cVal = (BYTE)iAve;

					lpc->re.cMin = (BYTE)iIns;
					lpc->re.cMax = (BYTE)iNxt;
					lpc->re.cVal = (BYTE)iAve;
					lpc++;
					dwCnt2++;
					iDec2 = iDec;
					iIns2 = iNxt + 1;
					for( j = i+1; j < iCnt; j++ )
					{
						iNxt2 = iIns2 + iInc;
						iDec2 += iRem;
						if( iDec2 >= 100 )
						{
							iDec2 -= 100;
							iNxt2++;
						}
						if( iNxt2 < 255 )
						{
							iAve2 = (iIns2 + iNxt2) / 2;
							lpc->bl.cMin = (BYTE)iIns;
							lpc->bl.cMax = (BYTE)iNxt;
							lpc->bl.cVal = (BYTE)iAve;
	
							lpc->gr.cMin = (BYTE)iIns2;
							lpc->gr.cMax = (BYTE)iNxt2;
							lpc->gr.cVal = (BYTE)iAve2;

							lpc->re.cMin = (BYTE)iIns2;
							lpc->re.cMax = (BYTE)iNxt2;
							lpc->re.cVal = (BYTE)iAve2;
							lpc++;
							dwCnt2++;
						}
						else
						{
							iNxt2 = 255;
							iAve2 = 255;
							lpc->bl.cMin = (BYTE)iIns;
							lpc->bl.cMax = (BYTE)iNxt;
							lpc->bl.cVal = (BYTE)iAve;
	
							lpc->gr.cMin = (BYTE)iIns2;
							lpc->gr.cMax = (BYTE)iNxt2;
							lpc->gr.cVal = (BYTE)iAve2;

							lpc->re.cMin = (BYTE)iIns2;
							lpc->re.cMax = (BYTE)iNxt2;
							lpc->re.cVal = (BYTE)iAve2;
							lpc++;
							dwCnt2++;
							break;
						}
						iDec3 = iDec2;
						iIns3 = iNxt2 + 1;
						for( k = j+1; k < iCnt; k++ )
						{
							iNxt3 = iIns3 + iInc;
							iDec3 += iRem;
							if( iDec3 >= 100 )
							{
								iDec3 -= 100;
								iNxt3++;
							}
							if( iNxt3 < 255 )
							{
								iAve3 = (iIns3 + iNxt3) / 2;
								lpc->bl.cMin = (BYTE)iIns;
								lpc->bl.cMax = (BYTE)iNxt;
								lpc->bl.cVal = (BYTE)iAve;
	
								lpc->gr.cMin = (BYTE)iIns2;
								lpc->gr.cMax = (BYTE)iNxt2;
								lpc->gr.cVal = (BYTE)iAve2;

								lpc->re.cMin = (BYTE)iIns3;
								lpc->re.cMax = (BYTE)iNxt3;
								lpc->re.cVal = (BYTE)iAve3;
								lpc++;
								dwCnt2++;
							}
							else
							{
								iNxt3 = 255;
								iAve3 = 255;
								lpc->bl.cMin = (BYTE)iIns;
								lpc->bl.cMax = (BYTE)iNxt;
								lpc->bl.cVal = (BYTE)iAve;
	
								lpc->gr.cMin = (BYTE)iIns2;
								lpc->gr.cMax = (BYTE)iNxt2;
								lpc->gr.cVal = (BYTE)iAve2;

								lpc->re.cMin = (BYTE)iIns3;
								lpc->re.cMax = (BYTE)iNxt3;
								lpc->re.cVal = (BYTE)iAve3;
								lpc++;
								dwCnt2++;
								break;
							}
							iIns3 = iNxt3 + 1;
						}
						iIns2 = iNxt2 + 1;
					}
				}
				else
				{
					iNxt = 255;
					iAve = 255;
					lpc->bl.cMin = (BYTE)iCur+1;
					lpc->bl.cMax = (BYTE)iNxt;
					lpc->bl.cVal = (BYTE)iAve;

					lpc->gr.cMin = (BYTE)iCur+1;
					lpc->gr.cMax = (BYTE)iNxt;
					lpc->gr.cVal = (BYTE)iAve;

					lpc->re.cMin = (BYTE)iCur+1;
					lpc->re.cMax = (BYTE)iNxt;
					lpc->re.cVal = (BYTE)iAve;
					lpc++;
					dwCnt2++;
					break;
				}
				iCur = iNxt;
			}
#ifdef	DIAGDT
			DumpTable( lpCR, dwCnt, dwCnt2 );
#endif	// DIAGDT
			DVGlobalUnlock( hg );
			lpHTbl->iCnt = dwCnt2;
			lpHTbl->hTbl = hg;
			flg = TRUE;
		}
		else
		{
			if( hg )
				DVGlobalFree( hg );
		}
	}
	return flg;
}

// eof - DvTbl32.c
