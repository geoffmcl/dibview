

// DvHelp.c

#include "dv.h"

extern	char	gszHelpFileName[];
extern	BOOL	fHelpUp;

void	Do_HELP( UINT uHelp, UINT dwData )
{
	fHelpUp = TRUE;
	WinHelp( ghMainWnd, gszHelpFileName, uHelp, dwData ) ;
}
// HELP_CONTENTS and HELP_QUIT
void	Do_IDM_HELP( HWND hWnd, UINT uHelp )
{
	Do_HELP( uHelp, 0L );
}


// eof
