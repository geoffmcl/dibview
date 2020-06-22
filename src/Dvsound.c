
// ===============================================
//
//	File:  DvSound.C
//
//	Purpose:  Contains various sound handlers
//
//	Comments:
//
//	History:
//	Date			Reason
//	29 Nov 1997		Created
// ==============================================

#include	"Dv.h"	// All inclusive include
#include	<mmsystem.h>

char	szSQ[] = "SystemQuestion";

void	StartQuestionSound( void )
{
	UINT	ui;
//	char	szTmp[MAX_PATH+8];
	LPSTR	lps = GetStgBuf();
	DWORD	fdwSound;

//	lps = &szTmp[0];
	fdwSound = SND_ASYNC;
	ui = GetWindowsDirectory( lps, MAX_PATH ); 
	if(ui)
	{
		if( lps[ui-1] != '\\' )
			strcat( lps, "\\" );

		strcat( lps, "Media\\Chimes.wav" );
		if( CheckIfFileExists( lps ) )
		{
			fdwSound |= SND_FILENAME;
		}
		else
		{
			lps = &szSQ[0];	// "SystemQuestion";
		}
	}
	else
	{
		lps = &szSQ[0];	// "SystemQuestion";
	}

	PlaySound( lps, NULL, fdwSound  );

}

// eof - DvSound.c
